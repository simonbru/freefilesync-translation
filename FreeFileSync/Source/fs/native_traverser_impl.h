// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include <zen/sys_error.h>
#include <zen/symlink_target.h>

#include <cstddef> //offsetof
#include <sys/stat.h>
#include <dirent.h>

//implementation header for native.cpp, not for reuse!!!

namespace
{
using namespace zen;
using AFS = AbstractFileSystem;


inline
AFS::FileId convertToAbstractFileId(const zen::FileId& fid)
{
    if (fid == zen::FileId())
        return AFS::FileId();

    AFS::FileId out(reinterpret_cast<const char*>(&fid.volumeId), sizeof(fid.volumeId));
    out.append(reinterpret_cast<const char*>(&fid.fileIndex), sizeof(fid.fileIndex));
    return out;
}


class DirTraverser
{
public:
    static void execute(const Zstring& baseDirectory, AFS::TraverserCallback& sink)
    {
        DirTraverser(baseDirectory, sink);
    }

private:
    DirTraverser(const Zstring& baseDirectory, AFS::TraverserCallback& sink)
    {
        /* quote: "Since POSIX.1 does not specify the size of the d_name field, and other nonstandard fields may precede
                   that field within the dirent structure, portable applications that use readdir_r() should allocate
                   the buffer whose address is passed in entry as follows:
                       len = offsetof(struct dirent, d_name) + pathconf(dirPath, _PC_NAME_MAX) + 1
                       entryp = malloc(len); */
        const size_t nameMax = std::max<long>(::pathconf(baseDirectory.c_str(), _PC_NAME_MAX), 10000); //::pathconf may return long(-1)
        buffer.resize(offsetof(struct ::dirent, d_name) + nameMax + 1);

        traverse(baseDirectory, sink);
    }

    DirTraverser           (const DirTraverser&) = delete;
    DirTraverser& operator=(const DirTraverser&) = delete;

    void traverse(const Zstring& dirPath, AFS::TraverserCallback& sink)
    {
        tryReportingDirError([&]
        {
            traverseWithException(dirPath, sink); //throw FileError
        }, sink);
    }

    void traverseWithException(const Zstring& dirPath, AFS::TraverserCallback& sink) //throw FileError
    {
        //no need to check for endless recursion: Linux has a fixed limit on the number of symbolic links in a path

        DIR* dirObj = ::opendir(dirPath.c_str()); //directory must NOT end with path separator, except "/"
        if (!dirObj)
            THROW_LAST_FILE_ERROR(replaceCpy(_("Cannot open directory %x."), L"%x", fmtPath(dirPath)), L"opendir");
        ZEN_ON_SCOPE_EXIT(::closedir(dirObj)); //never close nullptr handles! -> crash

        for (;;)
        {
            struct ::dirent* dirEntry = nullptr;
            if (::readdir_r(dirObj, reinterpret_cast< ::dirent*>(&buffer[0]), &dirEntry) != 0)
                THROW_LAST_FILE_ERROR(replaceCpy(_("Cannot enumerate directory %x."), L"%x", fmtPath(dirPath)), L"readdir_r");
            //don't retry but restart dir traversal on error! http://blogs.msdn.com/b/oldnewthing/archive/2014/06/12/10533529.aspx

            if (!dirEntry) //no more items
                return;

            //don't return "." and ".."
            const char* itemName = dirEntry->d_name; //evaluate dirEntry *before* going into recursion => we use a single "buffer"!

            if (itemName[0] == 0) throw FileError(replaceCpy(_("Cannot enumerate directory %x."), L"%x", fmtPath(dirPath)), L"readdir_r: Data corruption; item is missing a name.");
            if (itemName[0] == '.' &&
                (itemName[1] == 0 || (itemName[1] == '.' && itemName[2] == 0)))
                continue;

			const Zstring& itemPath = appendSeparator(dirPath) + itemName;

            struct ::stat statData = {};
            if (!tryReportingItemError([&]
        {
            if (::lstat(itemPath.c_str(), &statData) != 0) //lstat() does not resolve symlinks
                    THROW_LAST_FILE_ERROR(replaceCpy(_("Cannot read file attributes of %x."), L"%x", fmtPath(itemPath)), L"lstat");
            }, sink, itemName))
            continue; //ignore error: skip file

            if (S_ISLNK(statData.st_mode)) //on Linux there is no distinction between file and directory symlinks!
            {
                const AFS::TraverserCallback::SymlinkInfo linkInfo = { itemName, statData.st_mtime };

                switch (sink.onSymlink(linkInfo))
                {
                    case AFS::TraverserCallback::LINK_FOLLOW:
                    {
                        //try to resolve symlink (and report error on failure!!!)
                        struct ::stat statDataTrg = {};

                        bool validLink = tryReportingItemError([&]
                        {
                            if (::stat(itemPath.c_str(), &statDataTrg) != 0)
                                THROW_LAST_FILE_ERROR(replaceCpy(_("Cannot resolve symbolic link %x."), L"%x", fmtPath(itemPath)), L"stat");
                        }, sink, itemName);

                        if (validLink)
                        {
                            if (S_ISDIR(statDataTrg.st_mode)) //a directory
                            {
                                if (std::unique_ptr<AFS::TraverserCallback> trav = sink.onDir({ itemName }))
                                    traverse(itemPath, *trav);
                            }
                            else //a file or named pipe, ect.
                            {
                                AFS::TraverserCallback::FileInfo fi = { itemName, makeUnsigned(statDataTrg.st_size), statDataTrg.st_mtime, convertToAbstractFileId(extractFileId(statDataTrg)), &linkInfo };
                                sink.onFile(fi);
                            }
                        }
                        // else //broken symlink -> ignore: it's client's responsibility to handle error!
                    }
                    break;

                    case AFS::TraverserCallback::LINK_SKIP:
                        break;
                }
            }
            else if (S_ISDIR(statData.st_mode)) //a directory
            {
                if (std::unique_ptr<AFS::TraverserCallback> trav = sink.onDir({ itemName }))
                    traverse(itemPath, *trav);
            }
            else //a file or named pipe, ect.
            {
                AFS::TraverserCallback::FileInfo fi = { itemName, makeUnsigned(statData.st_size), statData.st_mtime, convertToAbstractFileId(extractFileId(statData)), nullptr /*symlinkInfo*/ };
                sink.onFile(fi);
            }
            /*
            It may be a good idea to not check "S_ISREG(statData.st_mode)" explicitly and to not issue an error message on other types to support these scenarios:
            - RTS setup watch (essentially wants to read directories only)
            - removeDirectory (wants to delete everything; pipes can be deleted just like files via "unlink")

            However an "open" on a pipe will block (https://sourceforge.net/p/freefilesync/bugs/221/), so the copy routines need to be smarter!!
            */
        }
    }

    std::vector<char> buffer;
};
}
