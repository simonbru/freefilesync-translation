// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef VERSIONING_H_8760247652438056
#define VERSIONING_H_8760247652438056

#include <functional>
#include <zen/time.h>
#include <zen/file_error.h>
#include "../structures.h"
#include "../fs/abstract.h"


namespace zen
{
//e.g. move C:\Source\subdir\Sample.txt -> D:\Revisions\subdir\Sample.txt 2012-05-15 131513.txt
//scheme: <revisions directory>\<relpath>\<filename>.<ext> YYYY-MM-DD HHMMSS.<ext>
/*
    - ignores missing source files/dirs
    - creates missing intermediate directories
    - does not create empty directories
    - handles symlinks
    - replaces already existing target files/dirs (supports retry)
        => (unlikely) risk of data loss for naming convention "versioning":
        race-condition if two FFS instances start at the very same second OR multiple folder pairs process the same filepath!!
*/

class FileVersioner
{
public:
    FileVersioner(const AbstractPath& versioningFolderPath, //throw FileError
                  VersioningStyle versioningStyle,
                  const TimeComp& timeStamp) :
        versioningFolderPath_(versioningFolderPath),
        versioningStyle_(versioningStyle),
        timeStamp_(formatTime<Zstring>(Zstr("%Y-%m-%d %H%M%S"), timeStamp)) //e.g. "2012-05-15 131513"
    {
        if (AbstractFileSystem::isNullPath(versioningFolderPath_))
            throw std::logic_error("Contract violation! " + std::string(__FILE__) + ":" + numberTo<std::string>(__LINE__));

        if (timeStamp_.size() != 17) //formatTime() returns empty string on error; unexpected length: e.g. problem in year 10,000!
            throw FileError(_("Unable to create time stamp for versioning:") + L" \"" + utfCvrtTo<std::wstring>(timeStamp_) + L"\"");
    }

    bool revisionFile(const AbstractPath& filePath, //throw FileError; return "false" if file is not existing
                      const Zstring& relativePath,

                      //called frequently if move has to revert to copy + delete => see zen::copyFile for limitations when throwing exceptions!
                      const std::function<void(std::int64_t bytesDelta)>& onNotifyCopyStatus); //may be nullptr

    void revisionFolder(const AbstractPath& folderPath, const Zstring& relativePath, //throw FileError

                        //optional callbacks: may be nullptr
                        const std::function<void(const std::wstring& displayPathFrom, const std::wstring& displayPathTo)>& onBeforeFileMove,   //one call for each *existing* object!
                        const std::function<void(const std::wstring& displayPathFrom, const std::wstring& displayPathTo)>& onBeforeFolderMove, //
                        //called frequently if move has to revert to copy + delete => see zen::copyFile for limitations when throwing exceptions!
                        const std::function<void(std::int64_t bytesDelta)>& onNotifyCopyStatus);

    //void limitVersions(std::function<void()> updateUI); //throw FileError; call when done revisioning!

private:
    void revisionFolderImpl(const AbstractPath& folderPath, const Zstring& relativePath,
                            const std::function<void(const std::wstring& displayPathFrom, const std::wstring& displayPathTo)>& onBeforeFileMove,
                            const std::function<void(const std::wstring& displayPathFrom, const std::wstring& displayPathTo)>& onBeforeFolderMove,
                            const std::function<void(std::int64_t bytesDelta)>& onNotifyCopyStatus); //throw FileError

    void moveItemToVersioning(const AbstractPath& itemPath, //throw FileError
                              const Zstring& relativePath,
                              const std::function<void(const AbstractPath& sourcePath, const AbstractPath& targetPath)>& moveItem); //may throw FileError

    const AbstractPath versioningFolderPath_;
    const VersioningStyle versioningStyle_;
    const Zstring timeStamp_;

    //std::vector<Zstring> fileRelNames; //store list of revisioned file and symlink relative names for limitVersions()
};

namespace impl //declare for unit tests:
{
bool isMatchingVersion(const Zstring& shortname, const Zstring& shortnameVersion);
}
}

#endif //VERSIONING_H_8760247652438056
