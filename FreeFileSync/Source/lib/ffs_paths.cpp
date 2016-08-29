// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "ffs_paths.h"
#include <zen/file_access.h>
#include <wx/stdpaths.h>
#include <wx/app.h>
#include <wx+/string_conv.h>

#ifdef ZEN_MAC
    #include <vector>
    #include <zen/scope_guard.h>
    #include <zen/osx_string.h>
    //keep in .cpp file to not pollute global namespace!
    #include <ApplicationServices/ApplicationServices.h> //LSFindApplicationForInfo
#endif

using namespace zen;


namespace
{
#if defined ZEN_WIN || defined ZEN_LINUX
inline
Zstring getExecutablePathPf() //directory containing executable WITH path separator at end
{
    return appendSeparator(beforeLast(utfCvrtTo<Zstring>(wxStandardPaths::Get().GetExecutablePath()), FILE_NAME_SEPARATOR, IF_MISSING_RETURN_NONE));
}
#endif

#ifdef ZEN_WIN
inline
Zstring getInstallFolderPathPf() //root install directory WITH path separator at end
{
    return appendSeparator(beforeLast(beforeLast(getExecutablePathPf(), FILE_NAME_SEPARATOR, IF_MISSING_RETURN_NONE), FILE_NAME_SEPARATOR, IF_MISSING_RETURN_NONE));
}
#endif
}


bool zen::isPortableVersion()
{
#ifdef ZEN_WIN
    return !fileExists(getInstallFolderPathPf() + L"uninstall.exe") && //created by NSIS
           !dirExists (getInstallFolderPathPf() + L"Uninstall");       //created by Inno Setup

#elif defined ZEN_LINUX
    return !endsWith(getExecutablePathPf(), "/bin/");  //this check is a bit lame...

#elif defined ZEN_MAC
    return false;
#endif
}


bool zen::manualProgramUpdateRequired()
{
#if defined ZEN_WIN || defined ZEN_MAC
    return true;
#elif defined ZEN_LINUX
    return true;
    //return isPortableVersion(); //locally installed version is updated by Launchpad
#endif
}


Zstring zen::getResourceDir()
{
    //make independent from wxWidgets global variable "appname"; support being called by RealTimeSync
    auto appName = wxTheApp->GetAppName();
    wxTheApp->SetAppName(L"FreeFileSync");
    ZEN_ON_SCOPE_EXIT(wxTheApp->SetAppName(appName));

#ifdef ZEN_WIN
    return getInstallFolderPathPf();
#elif defined ZEN_LINUX
    if (isPortableVersion())
        return getExecutablePathPf();
    else //use OS' standard paths
        return appendSeparator(toZ(wxStandardPathsBase::Get().GetResourcesDir()));
#elif defined ZEN_MAC
    return appendSeparator(toZ(wxStandardPathsBase::Get().GetResourcesDir())); //if packaged, used "Contents/Resources", else the executable directory
#endif
}


Zstring zen::getConfigDir()
{
    //make independent from wxWidgets global variable "appname"; support being called by RealTimeSync
    auto appName = wxTheApp->GetAppName();
    wxTheApp->SetAppName(L"FreeFileSync");
    ZEN_ON_SCOPE_EXIT(wxTheApp->SetAppName(appName));

#ifdef ZEN_WIN
    if (isPortableVersion())
        return getInstallFolderPathPf();
#elif defined ZEN_LINUX
    if (isPortableVersion())
        return getExecutablePathPf();
#elif defined ZEN_MAC
    //portable apps do not seem common on OS - fine with me: http://theocacao.com/document.page/319
#endif
    //use OS' standard paths
    Zstring configDirPath = toZ(wxStandardPathsBase::Get().GetUserDataDir());
    try
    {
        makeDirectoryRecursively(configDirPath); //throw FileError
    }
    catch (const FileError&) { assert(false); }

    return appendSeparator(configDirPath);
}


//this function is called by RealTimeSync!!!
Zstring zen::getFreeFileSyncLauncherPath()
{
#ifdef ZEN_WIN
    return getInstallFolderPathPf() + Zstr("FreeFileSync.exe");

#elif defined ZEN_LINUX
    return getExecutablePathPf() + Zstr("FreeFileSync");

#elif defined ZEN_MAC
    CFURLRef appURL = nullptr;
    ZEN_ON_SCOPE_EXIT(if (appURL) ::CFRelease(appURL));

    if (::LSFindApplicationForInfo(kLSUnknownCreator, // OSType inCreator,
                                   CFSTR("Zenju.FreeFileSync"),//CFStringRef inBundleID,
                                   nullptr,           //CFStringRef inName,
                                   nullptr,           //FSRef *outAppRef,
                                   &appURL) == noErr) //CFURLRef *outAppURL
        if (appURL)
            if (CFURLRef absUrl = ::CFURLCopyAbsoluteURL(appURL))
            {
                ZEN_ON_SCOPE_EXIT(::CFRelease(absUrl));

                if (CFStringRef path = ::CFURLCopyFileSystemPath(absUrl, kCFURLPOSIXPathStyle))
                {
                    ZEN_ON_SCOPE_EXIT(::CFRelease(path));
                    try
                    {
                        return appendSeparator(osx::cfStringToZstring(path)) + "Contents/MacOS/FreeFileSync"; //throw SysError
                    }
                    catch (SysError&) {}
                }
            }
    return Zstr("./FreeFileSync"); //fallback: at least give some hint...
#endif
}
