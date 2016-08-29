// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef PROCESS_XML_H_28345825704254262435
#define PROCESS_XML_H_28345825704254262435

#include <zen/xml_io.h>
#include <wx/gdicmn.h>
#include "localization.h"
#include "../structures.h"
#include "../ui/column_attr.h"


namespace xmlAccess
{
enum XmlType
{
    XML_TYPE_GUI,
    XML_TYPE_BATCH,
    XML_TYPE_GLOBAL,
    XML_TYPE_OTHER
};

XmlType getXmlType(const Zstring& filepath); //throw FileError


enum OnError
{
    ON_ERROR_IGNORE,
    ON_ERROR_POPUP,
    ON_ERROR_STOP
};

enum OnGuiError
{
    ON_GUIERROR_POPUP,
    ON_GUIERROR_IGNORE
};

using Description = std::wstring;
using Commandline = Zstring;
using ExternalApps = std::vector<std::pair<Description, Commandline>>;

//---------------------------------------------------------------------
struct XmlGuiConfig
{
    zen::MainConfiguration mainCfg;

    OnGuiError handleError = ON_GUIERROR_POPUP; //reaction on error situation during synchronization
    bool highlightSyncAction = true;
};


inline
bool operator==(const XmlGuiConfig& lhs, const XmlGuiConfig& rhs)
{
    return lhs.mainCfg             == rhs.mainCfg           &&
           lhs.handleError         == rhs.handleError       &&
           lhs.highlightSyncAction == rhs.highlightSyncAction;
}


struct XmlBatchConfig
{
    zen::MainConfiguration mainCfg;

    bool runMinimized = false;
    Zstring logFolderPathPhrase;
    int logfilesCountLimit = -1; //max logfiles; 0 := don't save logfiles; < 0 := no limit
    OnError handleError = ON_ERROR_POPUP; //reaction on error situation during synchronization
};


struct OptionalDialogs
{
    bool warningDependentFolders          = true;
    bool warningFolderPairRaceCondition   = true;
    bool warningSignificantDifference     = true;
    bool warningNotEnoughDiskSpace        = true;
    bool warningUnresolvedConflicts       = true;
    bool warningDatabaseError             = true;
    bool warningRecyclerMissing           = true;
    bool warningInputFieldEmpty           = true;
    bool warningDirectoryLockFailed       = true;
    bool popupOnConfigChange              = true;
    bool confirmSyncStart                 = true;
    bool confirmExternalCommandMassInvoke = true;
};


enum FileIconSize
{
    ICON_SIZE_SMALL,
    ICON_SIZE_MEDIUM,
    ICON_SIZE_LARGE
};


struct ViewFilterDefault
{
    //shared
    bool equal    = false;
    bool conflict = true;
    bool excluded = false;
    //category view
    bool leftOnly   = true;
    bool rightOnly  = true;
    bool leftNewer  = true;
    bool rightNewer = true;
    bool different  = true;
    //action view
    bool createLeft  = true;
    bool createRight = true;
    bool updateLeft  = true;
    bool updateRight = true;
    bool deleteLeft  = true;
    bool deleteRight = true;
    bool doNothing   = true;
};


struct ConfigFileItem
{
    ConfigFileItem() {}
    explicit ConfigFileItem(const Zstring& filePath) : filePath_(filePath) {}
    Zstring filePath_;
    //add support? -> time_t lastSyncTime
};


Zstring getGlobalConfigFile();

struct XmlGlobalSettings
{
    XmlGlobalSettings(); //clang needs this anyway

    //---------------------------------------------------------------------
    //Shared (GUI/BATCH) settings
    wxLanguage programLanguage = zen::getSystemLanguage();
    bool failSafeFileCopy = true;
    bool copyLockedFiles  = false; //safer default: avoid copies of partially written files
    bool copyFilePermissions = false;
    size_t automaticRetryCount = 0;
    size_t automaticRetryDelay = 5; //unit: [sec]

    int fileTimeTolerance = 2; //max. allowed file time deviation; < 0 means unlimited tolerance; default 2s: FAT vs NTFS
    int folderAccessTimeout = 20;  //unit: [s]; consider CD-ROM insert or hard disk spin up time from sleep
    bool runWithBackgroundPriority = false;
    bool createLockFile = true;
    bool verifyFileCopy = false;
    size_t lastSyncsLogFileSizeMax = 100000; //maximum size for LastSyncs.log: use a human-readable number
    Zstring soundFileCompareFinished;
    Zstring soundFileSyncFinished= Zstr("gong.wav");

    OptionalDialogs optDialogs;

    //---------------------------------------------------------------------
    struct Gui
    {
        Gui() {} //clang needs this anyway
        struct
        {
            wxPoint dlgPos;
            wxSize dlgSize;
            bool isMaximized = false;

            struct
            {
                bool keepRelPaths      = false;
                bool overwriteIfExists = false;
                Zstring lastUsedPath;
                std::vector<Zstring> folderHistory;
                size_t  historySizeMax = 15;
            } copyToCfg;

            bool manualDeletionUseRecycler = true;
            bool textSearchRespectCase = false; //good default for Linux, too!
            int maxFolderPairsVisible = 6;

            bool naviGridShowPercentBar = zen::naviGridShowPercentageDefault; //in navigation panel
            zen::ColumnTypeNavi naviGridLastSortColumn    = zen::naviGridLastSortColumnDefault;    //remember sort on navigation panel
            bool                naviGridLastSortAscending = zen::naviGridLastSortAscendingDefault; //

            std::vector<zen::ColumnAttributeNavi> columnAttribNavi = zen::getDefaultColumnAttributesNavi(); //compressed view/navigation

            bool showIcons = true;
            FileIconSize iconSize = ICON_SIZE_SMALL;
            int sashOffset = 0;

            zen::ItemPathFormat itemPathFormatLeftGrid  = zen::defaultItemPathFormatLeftGrid;
            zen::ItemPathFormat itemPathFormatRightGrid = zen::defaultItemPathFormatRightGrid;

            std::vector<zen::ColumnAttributeRim>  columnAttribLeft  = zen::getDefaultColumnAttributesLeft();
            std::vector<zen::ColumnAttributeRim>  columnAttribRight = zen::getDefaultColumnAttributesRight();

            ViewFilterDefault viewFilterDefault;
            wxString guiPerspectiveLast; //used by wxAuiManager
        } mainDlg;

#ifdef ZEN_WIN
        Zstring defaultExclusionFilter = Zstr("\\System Volume Information\\") Zstr("\n")
                                         Zstr("\\$Recycle.Bin\\")              Zstr("\n")
                                         Zstr("\\RECYCLER\\")                  Zstr("\n")
                                         Zstr("\\RECYCLED\\")                  Zstr("\n")
                                         Zstr("*\\desktop.ini")                Zstr("\n")
                                         Zstr("*\\thumbs.db");
#elif defined ZEN_LINUX
        Zstring defaultExclusionFilter = Zstr("/.Trash-*/") Zstr("\n")
                                         Zstr("/.recycle/");
#elif defined ZEN_MAC
        Zstring defaultExclusionFilter = Zstr("/.fseventsd/")      Zstr("\n")
                                         Zstr("/.Spotlight-V100/") Zstr("\n")
                                         Zstr("/.Trashes/")        Zstr("\n")
                                         Zstr("*/.DS_Store")       Zstr("\n")
                                         Zstr("*/._.*");
#endif

        std::vector<ConfigFileItem> lastUsedConfigFiles;

        std::vector<ConfigFileItem> cfgFileHistory;
        size_t cfgFileHistMax = 100;

        std::vector<Zstring> folderHistoryLeft;
        std::vector<Zstring> folderHistoryRight;
        size_t folderHistMax = 15;

        std::vector<Zstring> onCompletionHistory;
        size_t onCompletionHistoryMax = 8;

        ExternalApps externelApplications
        {
            //default external app descriptions will be translated "on the fly"!!!
            //CONTRACT: first entry will be used for [Enter] or mouse double-click!
#ifdef ZEN_WIN
            { L"Show in Explorer",              Zstr("explorer /select, \"%local_path%\"") },
            { L"Open with default application", Zstr("\"%local_path%\"")                   },
            //mark for extraction: _("Show in Explorer")
            //mark for extraction: _("Open with default application")
#elif defined ZEN_LINUX
            { L"Browse directory",              Zstr("xdg-open \"%folder_path%\"") },
            { L"Open with default application", Zstr("xdg-open \"%local_path%\"")   },
            //mark for extraction: _("Browse directory") Linux doesn't use the term "folder"
#elif defined ZEN_MAC
            { L"Browse directory",              Zstr("open -R \"%local_path%\"") },
            { L"Open with default application", Zstr("open \"%local_path%\"")    },
#endif
        };

        time_t lastUpdateCheck = 0; //number of seconds since 00:00 hours, Jan 1, 1970 UTC
        std::wstring lastOnlineVersion;
        std::wstring lastOnlineChangeLog;
    } gui;
};

//read/write specific config types
void readConfig(const Zstring& filepath, XmlGuiConfig&      config, std::wstring& warningMsg); //
void readConfig(const Zstring& filepath, XmlBatchConfig&    config, std::wstring& warningMsg); //throw FileError
void readConfig(const Zstring& filepath, XmlGlobalSettings& config, std::wstring& warningMsg); //

void writeConfig(const XmlGuiConfig&      config, const Zstring& filepath); //
void writeConfig(const XmlBatchConfig&    config, const Zstring& filepath); //throw FileError
void writeConfig(const XmlGlobalSettings& config, const Zstring& filepath); //

//convert (multiple) *.ffs_gui, *.ffs_batch files or combinations of both into target config structure:
void readAnyConfig(const std::vector<Zstring>& filepaths, XmlGuiConfig& config, std::wstring& warningMsg); //throw FileError

//config conversion utilities
XmlGuiConfig   convertBatchToGui(const XmlBatchConfig& batchCfg); //noexcept
XmlBatchConfig convertGuiToBatch(const XmlGuiConfig&   guiCfg, const XmlBatchConfig* referenceBatchCfg); //

std::wstring extractJobName(const Zstring& configFilename);
}

#endif //PROCESS_XML_H_28345825704254262435
