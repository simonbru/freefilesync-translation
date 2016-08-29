// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef ALGORITHM_H_34218518475321452548
#define ALGORITHM_H_34218518475321452548

#include <functional>
#include "file_hierarchy.h"
#include "lib/soft_filter.h"
#include "process_callback.h"


namespace zen
{
void swapGrids(const MainConfiguration& config, FolderComparison& folderCmp);

std::vector<DirectionConfig> extractDirectionCfg(const MainConfiguration& mainCfg);

void redetermineSyncDirection(const DirectionConfig& directConfig,
                              BaseFolderPair& baseFolder,
                              const std::function<void(const std::wstring& msg)>& reportWarning,
                              const std::function<void(std::int64_t bytesDelta)>& notifyProgress);

void redetermineSyncDirection(const MainConfiguration& mainCfg,
                              FolderComparison& folderCmp,
                              const std::function<void(const std::wstring& msg)>& reportWarning,
                              const std::function<void(std::int64_t bytesDelta)>& notifyProgress);

void setSyncDirectionRec(SyncDirection newDirection, FileSystemObject& fsObj); //set new direction (recursively)

bool allElementsEqual(const FolderComparison& folderCmp);

//filtering
void applyFiltering  (FolderComparison& folderCmp, const MainConfiguration& mainCfg); //full filter apply
void addHardFiltering(BaseFolderPair& baseFolder, const Zstring& excludeFilter);     //exclude additional entries only
void addSoftFiltering(BaseFolderPair& baseFolder, const SoftFilter& timeSizeFilter); //exclude additional entries only

void applyTimeSpanFilter(FolderComparison& folderCmp, std::int64_t timeFrom, std::int64_t timeTo); //overwrite current active/inactive settings

void setActiveStatus(bool newStatus, FolderComparison& folderCmp); //activate or deactivate all rows
void setActiveStatus(bool newStatus, FileSystemObject& fsObj);     //activate or deactivate row: (not recursively anymore)

std::pair<std::wstring, int> getSelectedItemsAsString( //returns string with item names and total count of selected(!) items, NOT total files/dirs!
    const std::vector<const FileSystemObject*>& selectionLeft,   //all pointers need to be bound!
    const std::vector<const FileSystemObject*>& selectionRight); //

//manual copy to alternate folder:
void copyToAlternateFolder(const std::vector<const FileSystemObject*>& rowsToCopyOnLeft,  //all pointers need to be bound!
                           const std::vector<const FileSystemObject*>& rowsToCopyOnRight, //
                           const Zstring& targetFolderPathPhrase,
                           bool keepRelPaths,
                           bool overwriteIfExists,
                           ProcessCallback& callback);

//manual deletion of files on main grid
void deleteFromGridAndHD(const std::vector<FileSystemObject*>& rowsToDeleteOnLeft,  //refresh GUI grid after deletion to remove invalid rows
                         const std::vector<FileSystemObject*>& rowsToDeleteOnRight, //all pointers need to be bound!
                         FolderComparison& folderCmp,                         //attention: rows will be physically deleted!
                         const std::vector<DirectionConfig>& directCfgs,
                         bool useRecycleBin,
                         //global warnings:
                         bool& warningRecyclerMissing,
                         ProcessCallback& callback);

//get native Win32 paths or create temporary copy for SFTP/MTP, ect.
class TempFileBuffer
{
public:
    TempFileBuffer() {}
    ~TempFileBuffer();

    struct FileDetails
    {
        AbstractPath path;
        FileDescriptor descr;
    };
    Zstring getTempPath(const FileDetails& details) const; //returns empty if not in buffer (item not existing, error during copy)

    //contract: only add files not yet in the buffer!
    void createTempFiles(const std::set<FileDetails>& workLoad, ProcessCallback& callback);

private:
    TempFileBuffer           (const TempFileBuffer&) = delete;
    TempFileBuffer& operator=(const TempFileBuffer&) = delete;

    std::map<FileDetails, Zstring> tempFilePaths;
    Zstring tempFolderPath;
};
bool operator<(const TempFileBuffer::FileDetails& lhs, const TempFileBuffer::FileDetails& rhs);
}
#endif //ALGORITHM_H_34218518475321452548
