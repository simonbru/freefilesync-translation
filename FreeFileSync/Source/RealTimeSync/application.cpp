// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "application.h"
#include "main_dlg.h"
#include <zen/file_access.h>
#include <zen/thread.h>
#include <wx/event.h>
#include <wx/log.h>
#include <wx/tooltip.h>
#include <wx+/string_conv.h>
#include <wx+/popup_dlg.h>
#include <wx+/image_resources.h>
#include "xml_proc.h"
#include "../lib/localization.h"
#include "../lib/ffs_paths.h"
#include "../lib/return_codes.h"
#include "../lib/error_log.h"
#include "../lib/help_provider.h"
#include "../lib/resolve_path.h"

#ifdef ZEN_WIN
    #include <zen/win_ver.h>
    #include <zen/dll.h>
    #include "../lib/app_user_mode_id.h"

#elif defined ZEN_LINUX
    #include <gtk/gtk.h>
#endif

using namespace zen;


IMPLEMENT_APP(Application);


namespace
{
#ifdef _MSC_VER
void crtInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved) { assert(false); }
#endif

const wxEventType EVENT_ENTER_EVENT_LOOP = wxNewEventType();
}


bool Application::OnInit()
{
#ifdef ZEN_WIN
#ifdef _MSC_VER
    _set_invalid_parameter_handler(crtInvalidParameterHandler); //see comment in <zen/time.h>
#endif
    //Quote: "Best practice is that all applications call the process-wide SetErrorMode function with a parameter of
    //SEM_FAILCRITICALERRORS at startup. This is to prevent error mode dialogs from hanging the application."
    ::SetErrorMode(SEM_FAILCRITICALERRORS);

#ifdef ZEN_WIN_VISTA_AND_LATER
    setAppUserModeId(L"RealTimeSync", L"Zenju.RealTimeSync"); //noexcept
    //consider: RealTimeSync.exe, RealTimeSync_Win32.exe, RealTimeSync_x64.exe
#endif

    wxToolTip::SetMaxWidth(-1); //disable tooltip wrapping -> Windows only

#elif defined ZEN_LINUX
    ::gtk_rc_parse((zen::getResourceDir() + "styles.gtk_rc").c_str()); //remove inner border from bitmap buttons
#endif

    //Windows User Experience Interaction Guidelines: tool tips should have 5s timeout, info tips no timeout => compromise:
    wxToolTip::Enable(true); //yawn, a wxWidgets screw-up: wxToolTip::SetAutoPop is no-op if global tooltip window is not yet constructed: wxToolTip::Enable creates it
    wxToolTip::SetAutoPop(10000); //https://msdn.microsoft.com/en-us/library/windows/desktop/aa511495

    SetAppName(L"RealTimeSync");

    initResourceImages(getResourceDir() + Zstr("Resources.zip"));

    Connect(wxEVT_QUERY_END_SESSION, wxEventHandler(Application::onQueryEndSession), nullptr, this);
    Connect(wxEVT_END_SESSION,       wxEventHandler(Application::onQueryEndSession), nullptr, this);

    //do not call wxApp::OnInit() to avoid using default commandline parser

    //Note: app start is deferred:  -> see FreeFileSync
    Connect(EVENT_ENTER_EVENT_LOOP, wxEventHandler(Application::onEnterEventLoop), nullptr, this);
    wxCommandEvent scrollEvent(EVENT_ENTER_EVENT_LOOP);
    AddPendingEvent(scrollEvent);

    return true; //true: continue processing; false: exit immediately.
}


int Application::OnExit()
{
    uninitializeHelp();
    releaseWxLocale();
    cleanupResourceImages();
    return wxApp::OnExit();
}


void Application::onEnterEventLoop(wxEvent& event)
{
    Disconnect(EVENT_ENTER_EVENT_LOOP, wxEventHandler(Application::onEnterEventLoop), nullptr, this);

    try
    {
        wxLanguage lngId = xmlAccess::getProgramLanguage();
        setLanguage(lngId); //throw FileError
    }
    catch (const FileError& e)
    {
        showNotificationDialog(nullptr, DialogInfoType::ERROR2, PopupDialogCfg().setDetailInstructions(e.toString()));
        //continue!
    }

    //try to set config/batch- filepath set by %1 parameter
    std::vector<Zstring> commandArgs;
    for (int i = 1; i < argc; ++i)
    {
        Zstring filePath = getResolvedFilePath(toZ(argv[i]));

        if (!fileExists(filePath)) //be a little tolerant
        {
            if (fileExists(filePath + Zstr(".ffs_real")))
                filePath += Zstr(".ffs_real");
            else if (fileExists(filePath + Zstr(".ffs_batch")))
                filePath += Zstr(".ffs_batch");
            else
            {
                showNotificationDialog(nullptr, DialogInfoType::ERROR2, PopupDialogCfg().setMainInstructions(replaceCpy(_("Cannot find file %x."), L"%x", fmtPath(filePath))));
                return;
            }
        }
        commandArgs.push_back(filePath);
    }

    Zstring cfgFilename;
    if (!commandArgs.empty())
        cfgFilename = commandArgs[0];

    MainDialog::create(cfgFilename);
}


int Application::OnRun()
{
    try
    {
        wxApp::OnRun();
    }
    catch (const std::bad_alloc& e) //the only kind of exception we don't want crash dumps for
    {
        logFatalError(e.what()); //it's not always possible to display a message box, e.g. corrupted stack, however low-level file output works!
        wxSafeShowMessage(L"RealTimeSync - " + _("An exception occurred"), e.what());
        return FFS_RC_EXCEPTION;
    }
    //catch (...) -> let it crash and create mini dump!!!

    return FFS_RC_SUCCESS; //program's return code
}



void Application::onQueryEndSession(wxEvent& event)
{
    if (auto mainWin = dynamic_cast<MainDialog*>(GetTopWindow()))
        mainWin->onQueryEndSession();
    //it's futile to try and clean up while the process is in full swing (CRASH!) => just terminate!
#ifdef ZEN_WIN
    /*BOOL rv = */ ::TerminateProcess(::GetCurrentProcess(), //_In_  HANDLE hProcess,
                                      FFS_RC_EXCEPTION);     //_In_  UINT uExitCode
#else
    std::abort(); //on Windows calls ::ExitProcess() which can still internally process Window messages and crash!
#endif
}
