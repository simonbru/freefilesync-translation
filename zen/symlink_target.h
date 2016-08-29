// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef SYMLINK_TARGET_H_801783470198357483
#define SYMLINK_TARGET_H_801783470198357483

#include "scope_guard.h"
#include "file_error.h"

#ifdef ZEN_WIN
    #include "win.h" //includes "windows.h"
    #include "privilege.h"
    #include "long_path_prefix.h"
    #include "dll.h"

#elif defined ZEN_LINUX || defined ZEN_MAC
    #include <unistd.h>
    #include <stdlib.h> //realpath
#endif


namespace zen
{
#ifdef ZEN_WIN
    bool isSymlink(const WIN32_FIND_DATA& data); //checking FILE_ATTRIBUTE_REPARSE_POINT is insufficient!
    bool isSymlink(DWORD fileAttributes, DWORD reparseTag);
#endif

Zstring getResolvedSymlinkPath(const Zstring& linkPath); //throw FileError; Win: requires Vista or later!
Zstring getSymlinkTargetRaw   (const Zstring& linkPath); //throw FileError
}









//################################ implementation ################################
#ifdef ZEN_WIN
//I don't have Windows Driver Kit at hands right now, so unfortunately we need to redefine this structure and cross fingers...
typedef struct _REPARSE_DATA_BUFFER //from ntifs.h
{
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union
    {
        struct
        {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG  Flags;
            WCHAR  PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct
        {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR  PathBuffer[1];
        } MountPointReparseBuffer;
        struct
        {
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;
#define REPARSE_DATA_BUFFER_HEADER_SIZE   FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)
#endif

namespace
{
//retrieve raw target data of symlink or junction
Zstring getSymlinkRawTargetString_impl(const Zstring& linkPath) //throw FileError
{
    using namespace zen;
#ifdef ZEN_WIN
    //FSCTL_GET_REPARSE_POINT: https://msdn.microsoft.com/en-us/library/aa364571

    //reading certain symlinks/junctions requires admin rights!
    try
    { activatePrivilege(PrivilegeName::BACKUP); } //throw FileError
    catch (FileError&) {} //This shall not cause an error in user mode!

    const HANDLE hLink = ::CreateFile(applyLongPathPrefix(linkPath).c_str(), //_In_      LPCTSTR lpFileName,
                                      //it seems we do not even need GENERIC_READ!
                                      0,                 //_In_      DWORD dwDesiredAccess,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,    //_In_      DWORD dwShareMode,
                                      nullptr,           //_In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                      OPEN_EXISTING,     //_In_      DWORD dwCreationDisposition,
                                      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, //_In_      DWORD dwFlagsAndAttributes,
                                      nullptr);          //_In_opt_  HANDLE hTemplateFile
    if (hLink == INVALID_HANDLE_VALUE)
        THROW_LAST_FILE_ERROR(replaceCpy(_("Cannot resolve symbolic link %x."), L"%x", fmtPath(linkPath)), L"CreateFile");
    ZEN_ON_SCOPE_EXIT(::CloseHandle(hLink));

    //respect alignment issues...
    const DWORD bufferSize = REPARSE_DATA_BUFFER_HEADER_SIZE + MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
    std::vector<char> buffer(bufferSize);

    DWORD bytesReturned = 0; //dummy value required by FSCTL_GET_REPARSE_POINT!
    if (!::DeviceIoControl(hLink,                   //__in         HANDLE hDevice,
                           FSCTL_GET_REPARSE_POINT, //__in         DWORD dwIoControlCode,
                           nullptr,                 //__in_opt     LPVOID lpInBuffer,
                           0,                       //__in         DWORD nInBufferSize,
                           &buffer[0],              //__out_opt    LPVOID lpOutBuffer,
                           bufferSize,              //__in         DWORD nOutBufferSize,
                           &bytesReturned,          //__out_opt    LPDWORD lpBytesReturned,
                           nullptr))                //__inout_opt  LPOVERLAPPED lpOverlapped
        THROW_LAST_FILE_ERROR(replaceCpy(_("Cannot resolve symbolic link %x."), L"%x", fmtPath(linkPath)), L"DeviceIoControl, FSCTL_GET_REPARSE_POINT");

    REPARSE_DATA_BUFFER& reparseData = *reinterpret_cast<REPARSE_DATA_BUFFER*>(&buffer[0]); //REPARSE_DATA_BUFFER needs to be artificially enlarged!

    Zstring output;
    if (reparseData.ReparseTag == IO_REPARSE_TAG_SYMLINK)
    {
        output = Zstring(reparseData.SymbolicLinkReparseBuffer.PathBuffer + reparseData.SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
                         reparseData.SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR));
    }
    else if (reparseData.ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
    {
        output = Zstring(reparseData.MountPointReparseBuffer.PathBuffer + reparseData.MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
                         reparseData.MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR));
    }
    else
        throw FileError(replaceCpy(_("Cannot resolve symbolic link %x."), L"%x", fmtPath(linkPath)), L"Not a symbolic link or junction.");

    //absolute symlinks and junctions use NT namespace naming convention while relative ones do not:
    //https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247#NT_Namespaces
    return ntPathToWin32Path(output);

#elif defined ZEN_LINUX || defined ZEN_MAC
    const size_t BUFFER_SIZE = 10000;
    std::vector<char> buffer(BUFFER_SIZE);

    const ssize_t bytesWritten = ::readlink(linkPath.c_str(), &buffer[0], BUFFER_SIZE);
    if (bytesWritten < 0)
        THROW_LAST_FILE_ERROR(replaceCpy(_("Cannot resolve symbolic link %x."), L"%x", fmtPath(linkPath)), L"readlink");
    if (bytesWritten >= static_cast<ssize_t>(BUFFER_SIZE)) //detect truncation, not an error for readlink!
        throw FileError(replaceCpy(_("Cannot resolve symbolic link %x."), L"%x", fmtPath(linkPath)), L"readlink: buffer truncated.");

    return Zstring(&buffer[0], bytesWritten); //readlink does not append 0-termination!
#endif
}


Zstring getResolvedSymlinkPath_impl(const Zstring& linkPath) //throw FileError
{
    using namespace zen;
#ifdef ZEN_WIN
    //GetFinalPathNameByHandle() is not available before Vista!
    using GetFinalPathNameByHandleWFunc = DWORD (WINAPI*)(HANDLE hFile, LPTSTR lpszFilePath, DWORD cchFilePath, DWORD dwFlags);
    const SysDllFun<GetFinalPathNameByHandleWFunc> getFinalPathNameByHandle(L"kernel32.dll", "GetFinalPathNameByHandleW");
    if (!getFinalPathNameByHandle)
        throw FileError(replaceCpy(_("Cannot determine final path for %x."), L"%x", fmtPath(linkPath)), replaceCpy(_("Cannot find system function %x."), L"%x", L"\"GetFinalPathNameByHandleW\""));

    const HANDLE hFile = ::CreateFile(applyLongPathPrefix(linkPath).c_str(),                  //_In_      LPCTSTR lpFileName,
                                      0,                                                      //_In_      DWORD dwDesiredAccess,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, //_In_      DWORD dwShareMode,
                                      nullptr,                    //_In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                      OPEN_EXISTING,              //_In_      DWORD dwCreationDisposition,
                                      //needed to open a directory:
                                      FILE_FLAG_BACKUP_SEMANTICS, //_In_      DWORD dwFlagsAndAttributes,
                                      nullptr);                   //_In_opt_  HANDLE hTemplateFile
    if (hFile == INVALID_HANDLE_VALUE)
        THROW_LAST_FILE_ERROR(replaceCpy(_("Cannot determine final path for %x."), L"%x", fmtPath(linkPath)), L"CreateFile");
    ZEN_ON_SCOPE_EXIT(::CloseHandle(hFile));

    const DWORD bufferSize = getFinalPathNameByHandle(hFile, nullptr, 0, 0);
    if (bufferSize == 0)
        THROW_LAST_FILE_ERROR(replaceCpy(_("Cannot determine final path for %x."), L"%x", fmtPath(linkPath)), L"GetFinalPathNameByHandle");

    std::vector<wchar_t> targetPath(bufferSize);
    const DWORD charsWritten = getFinalPathNameByHandle(hFile,          //__in   HANDLE hFile,
                                                        &targetPath[0], //__out  LPTSTR lpszFilePath,
                                                        bufferSize,     //__in   DWORD cchFilePath,
                                                        0);             //__in   DWORD dwFlags
    if (charsWritten == 0)
        THROW_LAST_FILE_ERROR(replaceCpy(_("Cannot determine final path for %x."), L"%x", fmtPath(linkPath)), L"GetFinalPathNameByHandle");
    if (charsWritten >= bufferSize)
        throw FileError(replaceCpy(_("Cannot determine final path for %x."), L"%x", fmtPath(linkPath)), L"GetFinalPathNameByHandle: insufficient buffer size.");

    return removeLongPathPrefix(Zstring(&targetPath[0], charsWritten)); //MSDN: GetFinalPathNameByHandle() always prepends "\\?\"

#elif defined ZEN_LINUX || defined ZEN_MAC
    char* targetPath = ::realpath(linkPath.c_str(), nullptr);
    if (!targetPath)
        THROW_LAST_FILE_ERROR(replaceCpy(_("Cannot determine final path for %x."), L"%x", fmtPath(linkPath)), L"realpath");
    ZEN_ON_SCOPE_EXIT(::free(targetPath));
    return targetPath;
#endif
}
}


namespace zen
{
inline
Zstring getSymlinkTargetRaw(const Zstring& linkPath) { return getSymlinkRawTargetString_impl(linkPath); }

inline
Zstring getResolvedSymlinkPath(const Zstring& linkPath) { return getResolvedSymlinkPath_impl(linkPath); }

#ifdef ZEN_WIN
/*
 Reparse Point Tags
    https://msdn.microsoft.com/en-us/library/windows/desktop/aa365511
 WIN32_FIND_DATA structure
    https://msdn.microsoft.com/en-us/library/windows/desktop/aa365740

 The only surrogate reparse points are;
    IO_REPARSE_TAG_MOUNT_POINT
    IO_REPARSE_TAG_SYMLINK
*/

inline
bool isSymlink(DWORD fileAttributes, DWORD reparseTag)
{
    return (fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0 &&
           IsReparseTagNameSurrogate(reparseTag);
}

inline
bool isSymlink(const WIN32_FIND_DATA& data)
{
    return isSymlink(data.dwFileAttributes, data.dwReserved0);
}
#endif
}

#endif //SYMLINK_TARGET_H_801783470198357483
