// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "version_check.h"
#include <zen/string_tools.h>
#include <zen/i18n.h>
#include <zen/utf.h>
#include <zen/scope_guard.h>
#include <zen/build_info.h>
#include <zen/basic_math.h>
#include <zen/file_error.h>
#include <zen/thread.h> //std::thread::id
#include <wx+/popup_dlg.h>
#include <wx+/http.h>
#include <wx+/image_resources.h>
#include "../lib/ffs_paths.h"
#include "version_check_impl.h"

#ifdef ZEN_WIN
    #include <zen/win_ver.h>

#elif defined ZEN_MAC
    #include <CoreServices/CoreServices.h> //Gestalt()
#endif

using namespace zen;


namespace
{
#ifndef NDEBUG
    const std::thread::id mainThreadId = std::this_thread::get_id();
#endif


std::wstring getIso639Language()
{
    assert(std::this_thread::get_id() == mainThreadId); //this function is not thread-safe, consider wxWidgets usage

#ifdef ZEN_WIN //use a more reliable function than wxWidgets:
    const int bufSize = 10;
    wchar_t buf[bufSize] = {};
    int rv = ::GetLocaleInfo(LOCALE_USER_DEFAULT,    //_In_       LCID Locale,
                             LOCALE_SISO639LANGNAME, //_In_       LCTYPE LCType,
                             buf,                    //_Out_opt_  LPTSTR lpLCData,
                             bufSize);               //_In_       int cchData
    if (0 < rv && rv < bufSize)
        return buf; //MSDN: "This can be a 3-letter code for languages that don't have a 2-letter code"!
    assert(false);
    return std::wstring();

#else
    const std::wstring localeName(wxLocale::GetLanguageCanonicalName(wxLocale::GetSystemLanguage()));
    if (localeName.empty())
        return std::wstring();

    assert(beforeLast(localeName, L"_", IF_MISSING_RETURN_ALL).size() == 2);
    return beforeLast(localeName, L"_", IF_MISSING_RETURN_ALL);
#endif
}


std::wstring getIso3166Country()
{
    assert(std::this_thread::get_id() == mainThreadId); //this function is not thread-safe, consider wxWidgets usage

#ifdef ZEN_WIN //use a more reliable function than wxWidgets:
    const int bufSize = 10;
    wchar_t buf[bufSize] = {};
    int rv = ::GetLocaleInfo(LOCALE_USER_DEFAULT,     //_In_       LCID Locale,
                             LOCALE_SISO3166CTRYNAME, //_In_       LCTYPE LCType,
                             buf,                     //_Out_opt_  LPTSTR lpLCData,
                             bufSize);                //_In_       int cchData
    if (0 < rv && rv < bufSize)
        return buf; //MSDN: "This can also return a number, such as "029" for Caribbean."!
    assert(false);
    return std::wstring();

#else
    const std::wstring localeName(wxLocale::GetLanguageCanonicalName(wxLocale::GetSystemLanguage()));
    if (localeName.empty())
        return std::wstring();

    return afterLast(localeName, L"_", IF_MISSING_RETURN_NONE);
#endif
}


//coordinate with get_latest_version_number.php
std::vector<std::pair<std::string, std::string>> geHttpPostParameters()
{
    assert(std::this_thread::get_id() == mainThreadId); //this function is not thread-safe, e.g. consider wxWidgets usage in isPortableVersion()
    std::vector<std::pair<std::string, std::string>> params;

    params.emplace_back("ffs_version", utfCvrtTo<std::string>(zen::ffsVersion));
    params.emplace_back("ffs_type", isPortableVersion() ? "Portable" : "Local");

#ifdef ZEN_WIN
    params.emplace_back("os_name", "Windows");
    const auto osvMajor = getOsVersion().major;
    const auto osvMinor = getOsVersion().minor;

#elif defined ZEN_LINUX
    params.emplace_back("os_name", "Linux");

    const wxLinuxDistributionInfo distribInfo = wxGetLinuxDistributionInfo();
    assert(contains(distribInfo.Release, L'.'));
    std::vector<wxString> digits = split<wxString>(distribInfo.Release, L'.'); //e.g. "15.04"
    digits.resize(2);
    //distribInfo.Id //e.g. "Ubuntu"

    const int osvMajor = stringTo<int>(digits[0]);
    const int osvMinor = stringTo<int>(digits[1]);

#elif defined ZEN_MAC
    params.emplace_back("os_name", "Mac");

    SInt32 osvMajor = 0;
    SInt32 osvMinor = 0;
    ::Gestalt(gestaltSystemVersionMajor, &osvMajor);
    ::Gestalt(gestaltSystemVersionMinor, &osvMinor);
#endif
    params.emplace_back("os_version", numberTo<std::string>(osvMajor) + "." + numberTo<std::string>(osvMinor));

#ifdef ZEN_WIN
    params.emplace_back("os_arch", running64BitWindows() ? "64" : "32");

#elif defined ZEN_LINUX || defined ZEN_MAC
#ifdef ZEN_BUILD_32BIT
    params.emplace_back("os_arch", "32");
#elif defined ZEN_BUILD_64BIT
    params.emplace_back("os_arch", "64");
#endif
#endif

    const std::string isoLang    = utfCvrtTo<std::string>(getIso639Language());
    const std::string isoCountry = utfCvrtTo<std::string>(getIso3166Country());

    params.emplace_back("language", !isoLang   .empty() ? isoLang    : "zz");
    params.emplace_back("country" , !isoCountry.empty() ? isoCountry : "ZZ");

    return params;
}


enum GetVerResult
{
    GET_VER_SUCCESS,
    GET_VER_NO_CONNECTION, //no internet connection?
    GET_VER_PAGE_NOT_FOUND //version file seems to have moved! => trigger an update!
};

//access is thread-safe on Windows (WinInet), but not on Linux/OS X (wxWidgets)
GetVerResult getOnlineVersion(const std::vector<std::pair<std::string, std::string>>& postParams, std::wstring& version)
{
    try //harmonize with wxHTTP: get_latest_version_number.php must be accessible without https!!!
    {
        const std::string buffer = sendHttpPost(L"http://www.freefilesync.org/get_latest_version_number.php", L"FFS-Update-Check", postParams); //throw FileError
        version = utfCvrtTo<std::wstring>(buffer);
        trim(version);
        return version.empty() ? GET_VER_PAGE_NOT_FOUND : GET_VER_SUCCESS; //empty version possible??
    }
    catch (const FileError&)
    {
        return internetIsAlive() ? GET_VER_PAGE_NOT_FOUND : GET_VER_NO_CONNECTION;
    }
}


std::vector<size_t> parseVersion(const std::wstring& version)
{
    std::vector<size_t> output;
    for (const std::wstring& digit : split(version, FFS_VERSION_SEPARATOR))
        output.push_back(stringTo<size_t>(digit));
    return output;
}


std::wstring getOnlineChangelogDelta()
{
    try //harmonize with wxHTTP: get_latest_changes.php must be accessible without https!!!
    {
        const std::string buffer = sendHttpPost(L"http://www.freefilesync.org/get_latest_changes.php", L"FFS-Update-Check", { { "since", utfCvrtTo<std::string>(zen::ffsVersion) } }); //throw FileError
        return utfCvrtTo<std::wstring>(buffer);
    }
    catch (FileError&) { assert(false); return std::wstring(); }
}


void showUpdateAvailableDialog(wxWindow* parent, const std::wstring& onlineVersion, const std::wstring& onlineChangeLog)
{
    switch (showConfirmationDialog(parent, DialogInfoType::INFO, PopupDialogCfg().
                                   setIcon(getResourceImage(L"download_update")).
                                   setTitle(_("Check for Program Updates")).
                                   setMainInstructions(_("A new version of FreeFileSync is available:")  + L" " + onlineVersion + L"\n" + _("Download now?")).
                                   setDetailInstructions(onlineChangeLog),
                                   _("&Download")))
    {
        case ConfirmationButton::DO_IT:
            wxLaunchDefaultBrowser(L"http://www.freefilesync.org/get_latest.php");
            break;
        case ConfirmationButton::CANCEL:
            break;
    }
}
}


bool zen::haveNewerVersionOnline(const std::wstring& onlineVersion)
{
    std::vector<size_t> current = parseVersion(zen::ffsVersion);
    std::vector<size_t> online  = parseVersion(onlineVersion);

    if (online.empty() || online[0] == 0) //online version string may be "This website has been moved..." In this case better check for an update
        return true;

    return std::lexicographical_compare(current.begin(), current.end(),
                                        online .begin(), online .end());
}


bool zen::updateCheckActive(time_t lastUpdateCheck)
{
    return lastUpdateCheck != getVersionCheckInactiveId();
}


void zen::disableUpdateCheck(time_t& lastUpdateCheck)
{
    lastUpdateCheck = getVersionCheckInactiveId();
}


void zen::checkForUpdateNow(wxWindow* parent, std::wstring& lastOnlineVersion, std::wstring& lastOnlineChangeLog)
{
    std::wstring onlineVersion;
    switch (getOnlineVersion(geHttpPostParameters(), onlineVersion))
    {
        case GET_VER_SUCCESS:
            lastOnlineVersion   = onlineVersion;
            lastOnlineChangeLog = haveNewerVersionOnline(onlineVersion) ? getOnlineChangelogDelta() : L"";

            if (haveNewerVersionOnline(onlineVersion))
                showUpdateAvailableDialog(parent, lastOnlineVersion, lastOnlineChangeLog);
            else
                showNotificationDialog(parent, DialogInfoType::INFO, PopupDialogCfg().
                                       setTitle(_("Check for Program Updates")).
                                       setMainInstructions(_("FreeFileSync is up to date.")));
            break;

        case GET_VER_NO_CONNECTION:
            showNotificationDialog(parent, DialogInfoType::ERROR2, PopupDialogCfg().
                                   setTitle(_("Check for Program Updates")).
                                   setMainInstructions(replaceCpy(_("Unable to connect to %x."), L"%x", L"www.freefilesync.org.")));
            break;

        case GET_VER_PAGE_NOT_FOUND:
            lastOnlineVersion = L"Unknown";
            lastOnlineChangeLog.clear();

            switch (showConfirmationDialog(parent, DialogInfoType::ERROR2, PopupDialogCfg().
                                           setTitle(_("Check for Program Updates")).
                                           setMainInstructions(_("Cannot find current FreeFileSync version number online. A newer version is likely available. Check manually now?")),
                                           _("&Check")))
            {
                case ConfirmationButton::DO_IT:
                    wxLaunchDefaultBrowser(L"http://www.freefilesync.org/get_latest.php");
                    break;
                case ConfirmationButton::CANCEL:
                    break;
            }
            break;
    }
}


struct zen::UpdateCheckResultPrep
{
    const std::vector<std::pair<std::string, std::string>> postParameters { geHttpPostParameters() };
};

//run on main thread:
std::shared_ptr<UpdateCheckResultPrep> zen::periodicUpdateCheckPrepare()
{
#ifdef ZEN_WIN
    return std::make_shared<UpdateCheckResultPrep>();
#else
    return nullptr;
#endif
}


struct zen::UpdateCheckResultAsync
{
#ifdef ZEN_WIN
    GetVerResult versionStatus = GET_VER_PAGE_NOT_FOUND;
    std::wstring onlineVersion;
#endif
};

//run on worker thread:
std::shared_ptr<UpdateCheckResultAsync> zen::periodicUpdateCheckRunAsync(const UpdateCheckResultPrep* resultPrep)
{
#ifdef ZEN_WIN
    auto result = std::make_shared<UpdateCheckResultAsync>();
    result->versionStatus = getOnlineVersion(resultPrep->postParameters, result->onlineVersion); //access is thread-safe on Windows only!
    return result;
#else
    return nullptr;
#endif
}


//run on main thread:
void zen::periodicUpdateCheckEval(wxWindow* parent, time_t& lastUpdateCheck, std::wstring& lastOnlineVersion, std::wstring& lastOnlineChangeLog, const UpdateCheckResultAsync* resultAsync)
{
#ifdef ZEN_WIN
    const GetVerResult versionStatus = resultAsync->versionStatus;
    const std::wstring onlineVersion = resultAsync->onlineVersion;
#else
    std::wstring onlineVersion;
    const GetVerResult versionStatus = getOnlineVersion(geHttpPostParameters(), onlineVersion);
#endif

    switch (versionStatus)
    {
        case GET_VER_SUCCESS:
            lastUpdateCheck     = getVersionCheckCurrentTime();
            lastOnlineVersion   = onlineVersion;
            lastOnlineChangeLog = haveNewerVersionOnline(onlineVersion) ? getOnlineChangelogDelta() : L"";

            if (haveNewerVersionOnline(onlineVersion))
                showUpdateAvailableDialog(parent, lastOnlineVersion, lastOnlineChangeLog);
            break;

        case GET_VER_NO_CONNECTION:
            break; //ignore this error

        case GET_VER_PAGE_NOT_FOUND:
            lastOnlineVersion = L"Unknown";
            lastOnlineChangeLog.clear();

            switch (showConfirmationDialog(parent, DialogInfoType::ERROR2, PopupDialogCfg().
                                           setTitle(_("Check for Program Updates")).
                                           setMainInstructions(_("Cannot find current FreeFileSync version number online. A newer version is likely available. Check manually now?")),
                                           _("&Check")))
            {
                case ConfirmationButton::DO_IT:
                    wxLaunchDefaultBrowser(L"http://www.freefilesync.org/get_latest.php");
                    break;
                case ConfirmationButton::CANCEL:
                    break;
            }
            break;
    }
}
