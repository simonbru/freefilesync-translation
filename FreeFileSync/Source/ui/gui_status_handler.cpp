// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "gui_status_handler.h"
#include <zen/shell_execute.h>
#include <wx/app.h>
#include <wx/wupdlock.h>
#include <wx+/bitmap_button.h>
#include <wx+/popup_dlg.h>
#include "main_dlg.h"
#include "on_completion_box.h"
#include "../lib/generate_logfile.h"
#include "../lib/resolve_path.h"
#include "../lib/status_handler_impl.h"

using namespace zen;
using namespace xmlAccess;


StatusHandlerTemporaryPanel::StatusHandlerTemporaryPanel(MainDialog& dlg) : mainDlg(dlg)
{
    {
#ifdef ZEN_WIN
        wxWindowUpdateLocker dummy(&mainDlg); //leads to GUI corruption problems on Linux/OS X!
#endif
        mainDlg.compareStatus->init(*this); //clear old values before showing panel

        //------------------------------------------------------------------
        const wxAuiPaneInfo& topPanel = mainDlg.auiMgr.GetPane(mainDlg.m_panelTopButtons);
        wxAuiPaneInfo& statusPanel    = mainDlg.auiMgr.GetPane(mainDlg.compareStatus->getAsWindow());

        //determine best status panel row near top panel
        switch (topPanel.dock_direction)
        {
            case wxAUI_DOCK_TOP:
            case wxAUI_DOCK_BOTTOM:
                statusPanel.Layer    (topPanel.dock_layer);
                statusPanel.Direction(topPanel.dock_direction);
                statusPanel.Row      (topPanel.dock_row + 1);
                break;

            case wxAUI_DOCK_LEFT:
            case wxAUI_DOCK_RIGHT:
                statusPanel.Layer    (std::max(0, topPanel.dock_layer - 1));
                statusPanel.Direction(wxAUI_DOCK_TOP);
                statusPanel.Row      (0);
                break;
                //case wxAUI_DOCK_CENTRE:
        }

        wxAuiPaneInfoArray& paneArray = mainDlg.auiMgr.GetAllPanes();

        const bool statusRowTaken = [&]
        {
            for (size_t i = 0; i < paneArray.size(); ++i)
            {
                wxAuiPaneInfo& paneInfo = paneArray[i];

                if (&paneInfo != &statusPanel &&
                paneInfo.dock_layer     == statusPanel.dock_layer &&
                paneInfo.dock_direction == statusPanel.dock_direction &&
                paneInfo.dock_row       == statusPanel.dock_row)
                    return true;
            }
            return false;
        }();

        //move all rows that are in the way one step further
        if (statusRowTaken)
            for (size_t i = 0; i < paneArray.size(); ++i)
            {
                wxAuiPaneInfo& paneInfo = paneArray[i];

                if (&paneInfo != &statusPanel &&
                    paneInfo.dock_layer     == statusPanel.dock_layer &&
                    paneInfo.dock_direction == statusPanel.dock_direction &&
                    paneInfo.dock_row       >= statusPanel.dock_row)
                    ++paneInfo.dock_row;
            }
        //------------------------------------------------------------------

        statusPanel.Show();
        mainDlg.auiMgr.Update();
    }

    mainDlg.Update(); //don't wait until idle event!

    //register keys
    mainDlg.Connect(wxEVT_CHAR_HOOK, wxKeyEventHandler(StatusHandlerTemporaryPanel::OnKeyPressed), nullptr, this);
    mainDlg.m_buttonCancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(StatusHandlerTemporaryPanel::OnAbortCompare), nullptr, this);
}


StatusHandlerTemporaryPanel::~StatusHandlerTemporaryPanel()
{
    //unregister keys
    mainDlg.Disconnect(wxEVT_CHAR_HOOK, wxKeyEventHandler(StatusHandlerTemporaryPanel::OnKeyPressed), nullptr, this);
    mainDlg.m_buttonCancel->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(StatusHandlerTemporaryPanel::OnAbortCompare), nullptr, this);

    mainDlg.auiMgr.GetPane(mainDlg.compareStatus->getAsWindow()).Hide();
    mainDlg.auiMgr.Update();
    mainDlg.compareStatus->teardown();
}


void StatusHandlerTemporaryPanel::OnKeyPressed(wxKeyEvent& event)
{
    const int keyCode = event.GetKeyCode();
    if (keyCode == WXK_ESCAPE)
    {
        wxCommandEvent dummy;
        OnAbortCompare(dummy);
    }

    event.Skip();
}


void StatusHandlerTemporaryPanel::initNewPhase(int objectsTotal, std::int64_t dataTotal, Phase phaseID)
{
    StatusHandler::initNewPhase(objectsTotal, dataTotal, phaseID);

    mainDlg.compareStatus->initNewPhase(); //call after "StatusHandler::initNewPhase"

    forceUiRefresh(); //throw ?; OS X needs a full yield to update GUI and get rid of "dummy" texts
}


void StatusHandlerTemporaryPanel::reportInfo(const std::wstring& text)
{
    StatusHandler::reportInfo(text);
    errorLog.logMsg(text, TYPE_INFO);
}


ProcessCallback::Response StatusHandlerTemporaryPanel::reportError(const std::wstring& errorMessage, size_t retryNumber)
{
    //no need to implement auto-retry here: 1. user is watching 2. comparison is fast
    //=> similar behavior like "ignoreErrors" which is also not used for the comparison phase in GUI mode

    //always, except for "retry":
    auto guardWriteLog = zen::makeGuard<ScopeGuardRunMode::ON_EXIT>([&] { errorLog.logMsg(errorMessage, TYPE_ERROR); });

    switch (handleError_)
    {
        case ON_GUIERROR_POPUP:
        {
            forceUiRefresh();

            bool ignoreNextErrors = false;
            switch (showConfirmationDialog3(&mainDlg, DialogInfoType::ERROR2, PopupDialogCfg3().
                                            setDetailInstructions(errorMessage).
                                            setCheckBox(ignoreNextErrors, _("&Ignore subsequent errors"), ConfirmationButton3::DONT_DO_IT),
                                            _("&Ignore"), _("&Retry")))
            {
                case ConfirmationButton3::DO_IT: //ignore
                    if (ignoreNextErrors) //falsify only
                        handleError_ = ON_GUIERROR_IGNORE;
                    return ProcessCallback::IGNORE_ERROR;

                case ConfirmationButton3::DONT_DO_IT: //retry
                    guardWriteLog.dismiss();
                    errorLog.logMsg(errorMessage + L"\n-> " + _("Retrying operation..."), TYPE_INFO); //explain why there are duplicate "doing operation X" info messages in the log!
                    return ProcessCallback::RETRY;

                case ConfirmationButton3::CANCEL:
                    abortProcessNow();
                    break;
            }
        }
        break;

        case ON_GUIERROR_IGNORE:
            return ProcessCallback::IGNORE_ERROR;
    }

    assert(false);
    return ProcessCallback::IGNORE_ERROR; //dummy return value
}


void StatusHandlerTemporaryPanel::reportFatalError(const std::wstring& errorMessage)
{
    errorLog.logMsg(errorMessage, TYPE_FATAL_ERROR);

    forceUiRefresh();
    showNotificationDialog(&mainDlg, DialogInfoType::ERROR2, PopupDialogCfg().setTitle(_("Serious Error")).setDetailInstructions(errorMessage));
}


void StatusHandlerTemporaryPanel::reportWarning(const std::wstring& warningMessage, bool& warningActive)
{
    errorLog.logMsg(warningMessage, TYPE_WARNING);

    if (!warningActive) //if errors are ignored, then warnings should also
        return;

    switch (handleError_)
    {
        case ON_GUIERROR_POPUP:
        {
            forceUiRefresh();

            bool dontWarnAgain = false;
            switch (showConfirmationDialog(&mainDlg, DialogInfoType::WARNING,
                                           PopupDialogCfg().setDetailInstructions(warningMessage).
                                           setCheckBox(dontWarnAgain, _("&Don't show this warning again")),
                                           _("&Ignore")))
            {
                case ConfirmationButton::DO_IT:
                    warningActive = !dontWarnAgain;
                    break;
                case ConfirmationButton::CANCEL:
                    abortProcessNow();
                    break;
            }
        }
        break;

        case ON_GUIERROR_IGNORE:
            break; //if errors are ignored, then warnings should also
    }
}


void StatusHandlerTemporaryPanel::forceUiRefresh()
{
    mainDlg.compareStatus->updateStatusPanelNow();
}


void StatusHandlerTemporaryPanel::abortProcessNow()
{
    requestAbortion(); //just make sure...
    throw GuiAbortProcess();
}


void StatusHandlerTemporaryPanel::OnAbortCompare(wxCommandEvent& event)
{
    requestAbortion();
}

//########################################################################################################

StatusHandlerFloatingDialog::StatusHandlerFloatingDialog(wxFrame* parentDlg,
                                                         size_t lastSyncsLogFileSizeMax,
                                                         OnGuiError handleError,
                                                         size_t automaticRetryCount,
                                                         size_t automaticRetryDelay,
                                                         const std::wstring& jobName,
                                                         const Zstring& soundFileSyncComplete,
                                                         const Zstring& onCompletion,
                                                         std::vector<Zstring>& onCompletionHistory) :
    progressDlg(createProgressDialog(*this, [this] { this->onProgressDialogTerminate(); }, *this, parentDlg, true, jobName, soundFileSyncComplete, onCompletion, onCompletionHistory)),
            lastSyncsLogFileSizeMax_(lastSyncsLogFileSizeMax),
            handleError_(handleError),
            automaticRetryCount_(automaticRetryCount),
            automaticRetryDelay_(automaticRetryDelay),
            jobName_(jobName),
startTime_(std::time(nullptr)) {}


StatusHandlerFloatingDialog::~StatusHandlerFloatingDialog()
{
    //------------ "on completion" command conceptually is part of the sync, not cleanup --------------------------------------

    //decide whether to stay on status screen or exit immediately...
    bool showFinalResults = true;

    if (progressDlg)
    {
        //execute "on completion" command (even in case of ignored errors)
        if (!abortIsRequested()) //if aborted (manually), we don't execute the command
        {
            const Zstring finalCommand = progressDlg->getExecWhenFinishedCommand(); //final value (after possible user modification)
            if (!finalCommand.empty())
            {
                if (isCloseProgressDlgCommand(finalCommand))
                    showFinalResults = false; //take precedence over current visibility status
                else
                    try
                    {
                        //use EXEC_TYPE_ASYNC until there is reason not to: https://sourceforge.net/p/freefilesync/discussion/help/thread/828dca52
                        tryReportingError([&] { shellExecute(expandMacros(finalCommand), EXEC_TYPE_ASYNC); }, //throw FileError
                                          *this); //throw X?
                    }
                    catch (...) {}
            }
        }
    }
    //------------ end of sync: begin of cleanup --------------------------------------

    const int totalErrors   = errorLog_.getItemCount(TYPE_ERROR | TYPE_FATAL_ERROR); //evaluate before finalizing log
    const int totalWarnings = errorLog_.getItemCount(TYPE_WARNING);

    //finalize error log
    std::wstring finalStatus;
    if (abortIsRequested())
    {
        finalStatus = _("Synchronization stopped");
        errorLog_.logMsg(finalStatus, TYPE_ERROR);
    }
    else if (totalErrors > 0)
    {
        finalStatus = _("Synchronization completed with errors");
        errorLog_.logMsg(finalStatus, TYPE_ERROR);
    }
    else if (totalWarnings > 0)
    {
        finalStatus = _("Synchronization completed with warnings");
        errorLog_.logMsg(finalStatus, TYPE_WARNING); //give status code same warning priority as display category!
    }
    else
    {
        if (getItemsTotal(PHASE_SYNCHRONIZING) == 0 && //we're past "initNewPhase(PHASE_SYNCHRONIZING)" at this point!
            getBytesTotal(PHASE_SYNCHRONIZING) == 0)
            finalStatus = _("Nothing to synchronize"); //even if "ignored conflicts" occurred!
        else
            finalStatus = _("Synchronization completed successfully");
        errorLog_.logMsg(finalStatus, TYPE_INFO);
    }

    const SummaryInfo summary =
    {
        jobName_, finalStatus,
        getItemsCurrent(PHASE_SYNCHRONIZING), getBytesCurrent(PHASE_SYNCHRONIZING),
        getItemsTotal  (PHASE_SYNCHRONIZING), getBytesTotal  (PHASE_SYNCHRONIZING),
        std::time(nullptr) - startTime_
    };

    //----------------- write results into LastSyncs.log------------------------
    try
    {
        saveToLastSyncsLog(summary, errorLog_, lastSyncsLogFileSizeMax_, OnUpdateLogfileStatusNoThrow(*this, utfCvrtTo<std::wstring>(getLastSyncsLogfilePath()))); //throw FileError
    }
    catch (FileError&) { assert(false); }

    if (progressDlg)
    {
        //notify to progressDlg that current process has ended
        if (showFinalResults)
        {
            if (abortIsRequested())
                progressDlg->processHasFinished(SyncProgressDialog::RESULT_ABORTED, errorLog_); //enable okay and close events
            else if (totalErrors > 0)
                progressDlg->processHasFinished(SyncProgressDialog::RESULT_FINISHED_WITH_ERROR, errorLog_);
            else if (totalWarnings > 0)
                progressDlg->processHasFinished(SyncProgressDialog::RESULT_FINISHED_WITH_WARNINGS, errorLog_);
            else
                progressDlg->processHasFinished(SyncProgressDialog::RESULT_FINISHED_WITH_SUCCESS, errorLog_);
        }
        else
            progressDlg->closeWindowDirectly();

        //wait until progress dialog notified shutdown via onProgressDialogTerminate()
        //-> required since it has our "this" pointer captured in lambda "notifyWindowTerminate"!
        //-> nicely manages dialog lifetime
        while (progressDlg)
        {
            wxTheApp->Yield(); //*first* refresh GUI (removing flicker) before sleeping!
            std::this_thread::sleep_for(std::chrono::milliseconds(UI_UPDATE_INTERVAL));
        }
    }
}


void StatusHandlerFloatingDialog::initNewPhase(int objectsTotal, std::int64_t dataTotal, Phase phaseID)
{
    assert(phaseID == PHASE_SYNCHRONIZING);
    StatusHandler::initNewPhase(objectsTotal, dataTotal, phaseID);
    if (progressDlg)
        progressDlg->initNewPhase(); //call after "StatusHandler::initNewPhase"

    forceUiRefresh(); //throw ?; OS X needs a full yield to update GUI and get rid of "dummy" texts
}


void StatusHandlerFloatingDialog::updateProcessedData(int objectsDelta, std::int64_t dataDelta)
{
    StatusHandler::updateProcessedData(objectsDelta, dataDelta);
    if (progressDlg)
        progressDlg->notifyProgressChange(); //noexcept
    //note: this method should NOT throw in order to properly allow undoing setting of statistics!
}


void StatusHandlerFloatingDialog::reportInfo(const std::wstring& text)
{
    StatusHandler::reportInfo(text);
    errorLog_.logMsg(text, TYPE_INFO);
}


ProcessCallback::Response StatusHandlerFloatingDialog::reportError(const std::wstring& errorMessage, size_t retryNumber)
{
    //auto-retry
    if (retryNumber < automaticRetryCount_)
    {
        errorLog_.logMsg(errorMessage + L"\n-> " +
                         _P("Automatic retry in 1 second...", "Automatic retry in %x seconds...", automaticRetryDelay_), TYPE_INFO);
        //delay
        const int iterations = static_cast<int>(1000 * automaticRetryDelay_ / UI_UPDATE_INTERVAL); //always round down: don't allow for negative remaining time below
        for (int i = 0; i < iterations; ++i)
        {
            reportStatus(_("Error") + L": " + _P("Automatic retry in 1 second...", "Automatic retry in %x seconds...",
                                                 (1000 * automaticRetryDelay_ - i * UI_UPDATE_INTERVAL + 999) / 1000)); //integer round up
            std::this_thread::sleep_for(std::chrono::milliseconds(UI_UPDATE_INTERVAL));
        }
        return ProcessCallback::RETRY;
    }


    //always, except for "retry":
    auto guardWriteLog = zen::makeGuard<ScopeGuardRunMode::ON_EXIT>([&] { errorLog_.logMsg(errorMessage, TYPE_ERROR); });

    switch (handleError_)
    {
        case ON_GUIERROR_POPUP:
        {
            if (!progressDlg) abortProcessNow();
            PauseTimers dummy(*progressDlg);
            forceUiRefresh();

            bool ignoreNextErrors = false;
            switch (showConfirmationDialog3(progressDlg->getWindowIfVisible(), DialogInfoType::ERROR2, PopupDialogCfg3().
                                            setDetailInstructions(errorMessage).
                                            setCheckBox(ignoreNextErrors, _("&Ignore subsequent errors"), ConfirmationButton3::DONT_DO_IT),
                                            _("&Ignore"), _("&Retry")))
            {
                case ConfirmationButton3::DO_IT: //ignore
                    if (ignoreNextErrors) //falsify only
                        handleError_ = ON_GUIERROR_IGNORE;
                    return ProcessCallback::IGNORE_ERROR;

                case ConfirmationButton3::DONT_DO_IT: //retry
                    guardWriteLog.dismiss();
                    errorLog_.logMsg(errorMessage + L"\n-> " + _("Retrying operation..."), TYPE_INFO); //explain why there are duplicate "doing operation X" info messages in the log!
                    return ProcessCallback::RETRY;

                case ConfirmationButton3::CANCEL:
                    abortProcessNow();
                    break;
            }
        }
        break;

        case ON_GUIERROR_IGNORE:
            return ProcessCallback::IGNORE_ERROR;
    }

    assert(false);
    return ProcessCallback::IGNORE_ERROR; //dummy value
}


void StatusHandlerFloatingDialog::reportFatalError(const std::wstring& errorMessage)
{
    errorLog_.logMsg(errorMessage, TYPE_FATAL_ERROR);

    switch (handleError_)
    {
        case ON_GUIERROR_POPUP:
        {
            if (!progressDlg) abortProcessNow();
            PauseTimers dummy(*progressDlg);
            forceUiRefresh();

            bool ignoreNextErrors = false;
            switch (showConfirmationDialog(progressDlg->getWindowIfVisible(), DialogInfoType::ERROR2,
                                           PopupDialogCfg().setTitle(_("Serious Error")).
                                           setDetailInstructions(errorMessage).
                                           setCheckBox(ignoreNextErrors, _("&Ignore subsequent errors")),
                                           _("&Ignore")))
            {
                case ConfirmationButton::DO_IT:
                    if (ignoreNextErrors) //falsify only
                        handleError_ = ON_GUIERROR_IGNORE;
                    break;
                case ConfirmationButton::CANCEL:
                    abortProcessNow();
                    break;
            }
        }
        break;

        case ON_GUIERROR_IGNORE:
            break;
    }
}


void StatusHandlerFloatingDialog::reportWarning(const std::wstring& warningMessage, bool& warningActive)
{
    errorLog_.logMsg(warningMessage, TYPE_WARNING);

    if (!warningActive)
        return;

    switch (handleError_)
    {
        case ON_GUIERROR_POPUP:
        {
            if (!progressDlg) abortProcessNow();
            PauseTimers dummy(*progressDlg);
            forceUiRefresh();

            bool dontWarnAgain = false;
            switch (showConfirmationDialog(progressDlg->getWindowIfVisible(), DialogInfoType::WARNING,
                                           PopupDialogCfg().setDetailInstructions(warningMessage).
                                           setCheckBox(dontWarnAgain, _("&Don't show this warning again")),
                                           _("&Ignore")))
            {
                case ConfirmationButton::DO_IT:
                    warningActive = !dontWarnAgain;
                    break;
                case ConfirmationButton::CANCEL:
                    abortProcessNow();
                    break;
            }
        }
        break;

        case ON_GUIERROR_IGNORE:
            break; //if errors are ignored, then warnings should be, too
    }
}


void StatusHandlerFloatingDialog::forceUiRefresh()
{
    if (progressDlg)
        progressDlg->updateGui();
}


void StatusHandlerFloatingDialog::abortProcessNow()
{
    requestAbortion(); //just make sure...
    throw GuiAbortProcess(); //abort can be triggered by progressDlg
}


void StatusHandlerFloatingDialog::onProgressDialogTerminate()
{
    //it's responsibility of "progressDlg" to call requestAbortion() when closing dialog
    progressDlg = nullptr;
}
