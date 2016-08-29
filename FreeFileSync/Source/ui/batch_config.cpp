// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "batch_config.h"
#include <wx/wupdlock.h>
#include <wx+/std_button_layout.h>
#include <wx+/font_size.h>
#include <wx+/image_resources.h>
#include "gui_generated.h"
#include "folder_selector.h"
#include "../ui/on_completion_box.h"
#include "../lib/help_provider.h"

#ifdef ZEN_WIN
    #include <wx+/mouse_move_dlg.h>
#endif

using namespace zen;
using namespace xmlAccess;


namespace
{
class BatchDialog : public BatchDlgGenerated
{
public:
    BatchDialog(wxWindow* parent,
                XmlBatchConfig& batchCfg, //in/out
                std::vector<Zstring>& onCompletionHistory,
                size_t onCompletionHistoryMax);

private:
    void OnClose       (wxCloseEvent&   event) override { EndModal(ReturnBatchConfig::BUTTON_CANCEL); }
    void OnCancel      (wxCommandEvent& event) override { EndModal(ReturnBatchConfig::BUTTON_CANCEL); }
    void OnSaveBatchJob(wxCommandEvent& event) override;
    void OnErrorPopup  (wxCommandEvent& event) override { localBatchCfg.handleError = ON_ERROR_POPUP;  updateGui(); }
    void OnErrorIgnore (wxCommandEvent& event) override { localBatchCfg.handleError = ON_ERROR_IGNORE; updateGui(); }
    void OnErrorStop   (wxCommandEvent& event) override { localBatchCfg.handleError = ON_ERROR_STOP;   updateGui(); }
    void OnHelpScheduleBatch(wxHyperlinkEvent& event) override { displayHelpEntry(L"schedule-a-batch-job", this); }

    void OnToggleGenerateLogfile(wxCommandEvent& event) override { updateGui(); }
    void OnToggleLogfilesLimit  (wxCommandEvent& event) override { updateGui(); }

    void updateGui(); //re-evaluate gui after config changes

    void setConfig(const XmlBatchConfig& batchCfg);
    XmlBatchConfig getConfig() const;

    XmlBatchConfig&       batchCfgOutRef; //output only!
    std::vector<Zstring>& onCompletionHistoryOut; //

    XmlBatchConfig localBatchCfg; //a mixture of settings some of which have OWNERSHIP WITHIN GUI CONTROLS! use getConfig() to resolve

    std::unique_ptr<FolderSelector> logfileDir; //always bound, solve circular compile-time dependency
};

//###################################################################################################################################

BatchDialog::BatchDialog(wxWindow* parent,
                         XmlBatchConfig& batchCfg,
                         std::vector<Zstring>& onCompletionHistory,
                         size_t onCompletionHistoryMax) :
    BatchDlgGenerated(parent),
    batchCfgOutRef(batchCfg),
    onCompletionHistoryOut(onCompletionHistory)
{
#ifdef ZEN_WIN
    new zen::MouseMoveWindow(*this); //allow moving main dialog by clicking (nearly) anywhere...; ownership passed to "this"
    wxWindowUpdateLocker dummy(this); //leads to GUI corruption problems on Linux/OS X!
#endif
    setStandardButtonLayout(*bSizerStdButtons, StdButtons().setAffirmative(m_buttonSaveAs).setCancel(m_buttonCancel));

    m_staticTextDescr->SetLabel(replaceCpy(m_staticTextDescr->GetLabel(), L"%x", L"FreeFileSync.exe <" + _("job name") + L">.ffs_batch"));

    m_comboBoxOnCompletion->setHistory(onCompletionHistory, onCompletionHistoryMax);

    m_bitmapBatchJob->SetBitmap(getResourceImage(L"batch"));

    logfileDir = std::make_unique<FolderSelector>(*m_panelLogfile, *m_buttonSelectLogFolder, *m_bpButtonSelectAltLogFolder, *m_logFolderPath, nullptr /*staticText*/, nullptr /*wxWindow*/);

    setConfig(batchCfg);

    GetSizer()->SetSizeHints(this); //~=Fit() + SetMinSize()
    //=> works like a charm for GTK2 with window resizing problems and title bar corruption; e.g. Debian!!!
    Center(); //needs to be re-applied after a dialog size change!

    m_buttonSaveAs->SetFocus();
}


void BatchDialog::updateGui() //re-evaluate gui after config changes
{
    XmlBatchConfig cfg = getConfig(); //resolve parameter ownership: some on GUI controls, others member variables

    m_panelLogfile        ->Enable(m_checkBoxGenerateLogfile->GetValue()); //enabled status is *not* directly dependent from resolved config! (but transitively)
    m_spinCtrlLogfileLimit->Enable(m_checkBoxGenerateLogfile->GetValue() && m_checkBoxLogfilesLimit->GetValue());

    m_radioBtnIgnoreErrors  ->SetValue(false);
    m_radioBtnPopupOnErrors ->SetValue(false);
    m_radioBtnStopOnError   ->SetValue(false);
    switch (cfg.handleError) //*not* owned by GUI controls
    {
        case ON_ERROR_IGNORE:
            m_radioBtnIgnoreErrors->SetValue(true);
            break;
        case ON_ERROR_POPUP:
            m_radioBtnPopupOnErrors->SetValue(true);
            break;
        case ON_ERROR_STOP:
            m_radioBtnStopOnError->SetValue(true);
            break;
    }
}


void BatchDialog::setConfig(const XmlBatchConfig& batchCfg)
{
#ifdef ZEN_WIN
    wxWindowUpdateLocker dummy(this); //leads to GUI corruption problems on Linux/OS X!
#endif

    localBatchCfg = batchCfg; //contains some parameters not owned by GUI controls

    //transfer parameter ownership to GUI
    m_checkBoxRunMinimized->SetValue(batchCfg.runMinimized);
    logfileDir->setPath(batchCfg.logFolderPathPhrase);
    m_comboBoxOnCompletion->setValue(batchCfg.mainCfg.onCompletion);

    //map single parameter "logfiles limit" to all three checkboxs and spin ctrl:
    m_checkBoxGenerateLogfile->SetValue(batchCfg.logfilesCountLimit != 0);
    m_checkBoxLogfilesLimit  ->SetValue(batchCfg.logfilesCountLimit >= 0);
    m_spinCtrlLogfileLimit   ->SetValue(batchCfg.logfilesCountLimit >= 0 ? batchCfg.logfilesCountLimit : 100 /*XmlBatchConfig().logfilesCountLimit*/);
    //attention: emits a "change value" event!! => updateGui() called implicitly!

    updateGui(); //re-evaluate gui after config changes
}


XmlBatchConfig BatchDialog::getConfig() const
{
    XmlBatchConfig batchCfg = localBatchCfg;

    //load parameters with ownership within GIU controls...

    //load structure with batch settings "batchCfg"
    batchCfg.runMinimized     = m_checkBoxRunMinimized->GetValue();
    batchCfg.logFolderPathPhrase = utfCvrtTo<Zstring>(logfileDir->getPath());
    batchCfg.mainCfg.onCompletion = m_comboBoxOnCompletion->getValue();
    //get single parameter "logfiles limit" from all three checkboxes and spin ctrl:
    batchCfg.logfilesCountLimit = m_checkBoxGenerateLogfile->GetValue() ? (m_checkBoxLogfilesLimit->GetValue() ? m_spinCtrlLogfileLimit->GetValue() : -1) : 0;

    return batchCfg;
}


void BatchDialog::OnSaveBatchJob(wxCommandEvent& event)
{
    batchCfgOutRef = getConfig();
    m_comboBoxOnCompletion->addItemHistory(); //a good place to commit current "on completion" history item
    onCompletionHistoryOut = m_comboBoxOnCompletion->getHistory();
    EndModal(ReturnBatchConfig::BUTTON_SAVE_AS);
}
}


ReturnBatchConfig::ButtonPressed zen::customizeBatchConfig(wxWindow* parent,
                                                           xmlAccess::XmlBatchConfig& batchCfg, //in/out
                                                           std::vector<Zstring>& onCompletionHistory,
                                                           size_t onCompletionHistoryMax)
{
    BatchDialog batchDlg(parent, batchCfg, onCompletionHistory, onCompletionHistoryMax);
    return static_cast<ReturnBatchConfig::ButtonPressed>(batchDlg.ShowModal());
}
