// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef FILE_ID_DEF_H_013287632486321493
#define FILE_ID_DEF_H_013287632486321493

#include <utility>

#ifdef ZEN_WIN
    #include "win.h" //includes "windows.h"

#elif defined ZEN_LINUX || defined ZEN_MAC
    #include <sys/stat.h>
#endif


namespace zen
{
#ifdef ZEN_WIN
using VolumeId  = DWORD;
using FileIndex = ULONGLONG;

#elif defined ZEN_LINUX || defined ZEN_MAC
namespace impl { typedef struct ::stat StatDummy; } //sigh...

using VolumeId  = decltype(impl::StatDummy::st_dev);
using FileIndex = decltype(impl::StatDummy::st_ino);
#endif


struct FileId  //always available on Linux, and *generally* available on Windows)
{
    FileId() {}
    FileId(VolumeId volId, FileIndex fIdx) : volumeId(volId), fileIndex(fIdx) {}
    VolumeId  volumeId  = 0;
    FileIndex fileIndex = 0;
};
inline bool operator==(const FileId& lhs, const FileId& rhs) { return lhs.volumeId == rhs.volumeId && lhs.fileIndex == rhs.fileIndex; }


#ifdef ZEN_WIN
inline
FileId extractFileId(const BY_HANDLE_FILE_INFORMATION& fileInfo)
{
    ULARGE_INTEGER fileIndex = {};
    fileIndex.HighPart = fileInfo.nFileIndexHigh;
    fileIndex.LowPart  = fileInfo.nFileIndexLow;

    return fileInfo.dwVolumeSerialNumber != 0 && fileIndex.QuadPart != 0 ?
           FileId(fileInfo.dwVolumeSerialNumber, fileIndex.QuadPart) : FileId();
}

inline
FileId extractFileId(DWORD volumeSerialNumber, ULONGLONG fileIndex)
{
    return volumeSerialNumber != 0 && fileIndex != 0 ?
           FileId(volumeSerialNumber, fileIndex) : FileId();
}

static_assert(sizeof(FileId().volumeId ) == sizeof(BY_HANDLE_FILE_INFORMATION().dwVolumeSerialNumber), "");
static_assert(sizeof(FileId().fileIndex) == sizeof(BY_HANDLE_FILE_INFORMATION().nFileIndexHigh) + sizeof(BY_HANDLE_FILE_INFORMATION().nFileIndexLow), "");
static_assert(sizeof(FileId().fileIndex) == sizeof(ULARGE_INTEGER), "");

#elif defined ZEN_LINUX || defined ZEN_MAC
inline
FileId extractFileId(const struct ::stat& fileInfo)
{
    return fileInfo.st_dev != 0 && fileInfo.st_ino != 0 ?
           FileId(fileInfo.st_dev, fileInfo.st_ino) : FileId();
}
#endif
}

#endif //FILE_ID_DEF_H_013287632486321493
