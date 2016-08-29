// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "sync_cfg.h"
#include <memory>
#include <wx/wupdlock.h>
#include <wx+/rtl.h>
#include <wx+/no_flicker.h>
#include <wx+/choice_enum.h>
#include <wx+/image_tools.h>
#include <wx+/font_size.h>
#include <wx+/std_button_layout.h>
#include <wx+/popup_dlg.h>
#include <wx+/image_resources.h>
#include "gui_generated.h"
#include "on_completion_box.h"
#include "folder_selector.h"
#include "../file_hierarchy.h"
#include "../lib/help_provider.h"
#include "../lib/norm_filter.h"

#ifdef ZEN_WIN
    #include <wx+/mouse_move_dlg.h>

#elif defined ZEN_MAC
    #include <zen/osx_string.h>
#endif

using namespace zen;
using namespace xmlAccess;


namespace
{
void toggleDeletionPolicy(DeletionPolicy& deletionPolicy);


class ConfigDialog : public ConfigDlgGenerated
{
public:
    ConfigDialog(wxWindow* parent,
                 SyncConfigPanel panelToShow,
                 int localPairIndexToShow,
                 std::vector<LocalPairConfig>& folderPairConfig,
                 GlobalSyncConfig& globalCfg,
                 size_t onCompletionHistoryMax);

private:
    void OnOkay  (wxCommandEvent& event) override;
    void OnCancel(wxCommandEvent& event) override { EndModal(ReturnSyncConfig::BUTTON_CANCEL); }
    void OnClose (wxCloseEvent&   event) override { EndModal(ReturnSyncConfig::BUTTON_CANCEL); }

    void onLocalKeyEvent(wxKeyEvent& event);
    void onListBoxKeyEvent(wxKeyEvent& event) override;
    void OnSelectFolderPair(wxCommandEvent& event) override;

    enum class ConfigTypeImage
    {
        COMPARISON = 0, //used as zero-based wxImageList index!
        COMPARISON_GREY,
        FILTER,
        FILTER_GREY,
        SYNC,
        SYNC_GREY,
    };

    //------------- comparison panel ----------------------
    void OnHelpComparisonSettings(wxHyperlinkEvent& event) override { displayHelpEntry(L"comparison-settings" , this); }
    void OnHelpTimeShift         (wxHyperlinkEvent& event) override { displayHelpEntry(L"daylight-saving-time", this); }

    void OnToggleLocalCompSettings(wxCommandEvent& event) override { updateCompGui(); updateSyncGui(); /*affects sync settings, too!*/ }
    void OnCompByTimeSize         (wxCommandEvent& event) override { localCmpVar = CompareVariant::TIME_SIZE; updateCompGui(); updateSyncGui(); } //
    void OnCompByContent          (wxCommandEvent& event) override { localCmpVar = CompareVariant::CONTENT;   updateCompGui(); updateSyncGui(); } //affects sync settings, too!
    void OnCompBySize             (wxCommandEvent& event) override { localCmpVar = CompareVariant::SIZE;      updateCompGui(); updateSyncGui(); } //
    void OnCompByTimeSizeDouble   (wxMouseEvent&   event) override;
    void OnCompBySizeDouble       (wxMouseEvent&   event) override;
    void OnCompByContentDouble    (wxMouseEvent&   event) override;
    void OnChangeCompOption       (wxCommandEvent& event) override { updateCompGui(); }
    void onlTimeShiftKeyDown      (wxKeyEvent& event) override;

    std::shared_ptr<const CompConfig> getCompConfig() const;
    void setCompConfig(std::shared_ptr<const CompConfig> compCfg);

    void updateCompGui();

    CompareVariant localCmpVar = CompareVariant::TIME_SIZE;

    //------------- filter panel --------------------------
    void OnHelpShowExamples(wxHyperlinkEvent& event) override { displayHelpEntry(L"exclude-items", this); }
    void OnChangeFilterOption(wxCommandEvent& event) override { updateFilterGui(); }
    void OnFilterReset       (wxCommandEvent& event) override { setFilterConfig(FilterConfig()); }

    void onFilterKeyEvent(wxKeyEvent& event);

    FilterConfig getFilterConfig() const;
    void setFilterConfig(const FilterConfig& filter);

    void updateFilterGui();

    EnumDescrList<UnitTime> enumTimeDescr;
    EnumDescrList<UnitSize> enumSizeDescr;

    //------------- synchronization panel -----------------
    void OnSyncTwoWay(wxCommandEvent& event) override { directionCfg.var = DirectionConfig::TWO_WAY; updateSyncGui(); }
    void OnSyncMirror(wxCommandEvent& event) override { directionCfg.var = DirectionConfig::MIRROR;  updateSyncGui(); }
    void OnSyncUpdate(wxCommandEvent& event) override { directionCfg.var = DirectionConfig::UPDATE;  updateSyncGui(); }
    void OnSyncCustom(wxCommandEvent& event) override { directionCfg.var = DirectionConfig::CUSTOM;  updateSyncGui(); }

    void OnToggleLocalSyncSettings(wxCommandEvent& event) override { updateSyncGui(); }
    void OnToggleDetectMovedFiles (wxCommandEvent& event) override { directionCfg.detectMovedFiles = !directionCfg.detectMovedFiles; updateSyncGui(); } //parameter NOT owned by checkbox!
    void OnChangeSyncOption       (wxCommandEvent& event) override { updateSyncGui(); }

    void OnSyncTwoWayDouble(wxMouseEvent& event) override;
    void OnSyncMirrorDouble(wxMouseEvent& event) override;
    void OnSyncUpdateDouble(wxMouseEvent& event) override;
    void OnSyncCustomDouble(wxMouseEvent& event) override;

    void OnExLeftSideOnly (wxCommandEvent& event) override;
    void OnExRightSideOnly(wxCommandEvent& event) override;
    void OnLeftNewer      (wxCommandEvent& event) override;
    void OnRightNewer     (wxCommandEvent& event) override;
    void OnDifferent      (wxCommandEvent& event) override;
    void OnConflict       (wxCommandEvent& event) override;

    void OnDeletionPermanent  (wxCommandEvent& event) override { handleDeletion = DeletionPolicy::PERMANENT;  updateSyncGui(); }
    void OnDeletionRecycler   (wxCommandEvent& event) override { handleDeletion = DeletionPolicy::RECYCLER;   updateSyncGui(); }
    void OnDeletionVersioning (wxCommandEvent& event) override { handleDeletion = DeletionPolicy::VERSIONING; updateSyncGui(); }

    void OnToggleDeletionType(wxCommandEvent& event) override { toggleDeletionPolicy(handleDeletion); updateSyncGui(); }

    void OnHelpDetectMovedFiles(wxHyperlinkEvent& event) override { displayHelpEntry(L"synchronization-settings" , this); }
    void OnHelpVersioning      (wxHyperlinkEvent& event) override { displayHelpEntry(L"versioning", this); }

    std::shared_ptr<const SyncConfig> getSyncConfig() const;
    void setSyncConfig(std::shared_ptr<const SyncConfig> syncCfg);

    void updateSyncGui();

    //-----------------------------------------------------

    void OnErrorPopup (wxCommandEvent& event) override { onGuiError = ON_GUIERROR_POPUP;  updateMiscGui(); } //parameter NOT owned by radio button
    void OnErrorIgnore(wxCommandEvent& event) override { onGuiError = ON_GUIERROR_IGNORE; updateMiscGui(); } //

    MiscSyncConfig getMiscSyncOptions() const;
    void setMiscSyncOptions(const MiscSyncConfig& miscCfg);

    void updateMiscGui();

    //parameters with ownership NOT within GUI controls!
    DirectionConfig directionCfg;
    DeletionPolicy handleDeletion = DeletionPolicy::RECYCLER; //use Recycler, delete permanently or move to user-defined location
    OnGuiError onGuiError = ON_GUIERROR_POPUP;

    EnumDescrList<VersioningStyle> enumVersioningStyle;
    FolderSelector versioningFolder;

    //-----------------------------------------------------

    void selectFolderPairConfig(int newPairIndexToShow);
    bool unselectFolderPairConfig(); //returns false on error: shows message box!

    //output-only parameters
    GlobalSyncConfig& globalCfgOut;
    std::vector<LocalPairConfig>& folderPairConfigOut;

    //working copy of ALL config parameters: only one folder pair is selected at a time!
    GlobalSyncConfig globalCfg_;
    std::vector<LocalPairConfig> folderPairConfig_;

    int selectedPairIndexToShow = EMPTY_PAIR_INDEX_SELECTED;
    static const int EMPTY_PAIR_INDEX_SELECTED = -2;

    const size_t onCompletionHistoryMax_;
};

//#################################################################################################################

std::wstring getCompVariantDescription(CompareVariant var)
{
    switch (var)
    {
        case CompareVariant::TIME_SIZE:
            return _("Identify equal files by comparing modification time and size.");
        case CompareVariant::CONTENT:
            return _("Identify equal files by comparing the file content.");
        case CompareVariant::SIZE:
            return _("Identify equal files by comparing their file size.");
    }
    assert(false);
    return _("Error");
}


std::wstring getSyncVariantDescription(DirectionConfig::Variant var)
{
    switch (var)
    {
        case DirectionConfig::TWO_WAY:
            return _("Identify and propagate changes on both sides. Deletions, moves and conflicts are detected automatically using a database.");
        case DirectionConfig::MIRROR:
            return _("Create a mirror backup of the left folder by adapting the right folder to match.");
        case DirectionConfig::UPDATE:
            return _("Copy new and updated files to the right folder.");
        case DirectionConfig::CUSTOM:
            return _("Configure your own synchronization rules.");
    }
    assert(false);
    return _("Error");
}


ConfigDialog::ConfigDialog(wxWindow* parent,
                           SyncConfigPanel panelToShow,
                           int localPairIndexToShow,
                           std::vector<LocalPairConfig>& folderPairConfig,
                           GlobalSyncConfig& globalCfg,
                           size_t onCompletionHistoryMax) :
    ConfigDlgGenerated(parent),
    versioningFolder(*m_panelVersioning, *m_buttonSelectVersioningFolder, *m_bpButtonSelectAltFolder, *m_versioningFolderPath, nullptr /*staticText*/, nullptr /*wxWindow*/),
    globalCfgOut(globalCfg),
    folderPairConfigOut(folderPairConfig),
    globalCfg_(globalCfg),
    folderPairConfig_(folderPairConfig),
    onCompletionHistoryMax_(onCompletionHistoryMax)
{
#ifdef ZEN_WIN
    new zen::MouseMoveWindow(*this); //allow moving main dialog by clicking (nearly) anywhere...; ownership passed to "this"
#endif
    setStandardButtonLayout(*bSizerStdButtons, StdButtons().setAffirmative(m_buttonOkay).setCancel(m_buttonCancel));

    SetTitle(_("Synchronization Settings"));

    //fill image list to cope with wxNotebook image setting design desaster...
    const int imageListSize = getResourceImage(L"cfg_compare_small").GetHeight();
    assert(imageListSize == 16); //Windows default size for panel caption
    auto imgList = std::make_unique<wxImageList>(imageListSize, imageListSize);

    auto addToImageList = [&](const wxBitmap& bmp)
    {
        assert(bmp.GetWidth () <= imageListSize);
        assert(bmp.GetHeight() <= imageListSize);
        imgList->Add(bmp);
        imgList->Add(greyScale(bmp));
    };
    //add images in same sequence like ConfigTypeImage enum!!!
    addToImageList(getResourceImage(L"cfg_compare_small"));
    addToImageList(getResourceImage(L"filter_small"     ));
    addToImageList(getResourceImage(L"cfg_sync_small"   ));
    assert(imgList->GetImageCount() == static_cast<int>(ConfigTypeImage::SYNC_GREY) + 1);

    m_notebook->AssignImageList(imgList.release()); //pass ownership

    m_notebook->SetPageText(static_cast<size_t>(SyncConfigPanel::COMPARISON), _("Comparison")      + L" (F6)");
    m_notebook->SetPageText(static_cast<size_t>(SyncConfigPanel::FILTER    ), _("Filter")          + L" (F7)");
    m_notebook->SetPageText(static_cast<size_t>(SyncConfigPanel::SYNC      ), _("Synchronization") + L" (F8)");

    m_notebook->ChangeSelection(static_cast<size_t>(panelToShow));

    //------------- comparison panel ----------------------
    setRelativeFontSize(*m_toggleBtnByTimeSize, 1.25);
    setRelativeFontSize(*m_toggleBtnBySize,     1.25);
    setRelativeFontSize(*m_toggleBtnByContent,  1.25);

    m_toggleBtnByTimeSize->SetToolTip(getCompVariantDescription(CompareVariant::TIME_SIZE));
    m_toggleBtnByContent ->SetToolTip(getCompVariantDescription(CompareVariant::CONTENT));
    m_toggleBtnBySize    ->SetToolTip(getCompVariantDescription(CompareVariant::SIZE));

    m_bitmapByTimeSize->SetToolTip(getCompVariantDescription(CompareVariant::TIME_SIZE));
    m_bitmapByContent ->SetToolTip(getCompVariantDescription(CompareVariant::CONTENT));
    m_bitmapBySize    ->SetToolTip(getCompVariantDescription(CompareVariant::SIZE));

    //------------- filter panel --------------------------
#ifndef __WXGTK__  //wxWidgets breaks portability promise once again
    m_textCtrlInclude->SetMaxLength(0); //allow large filter entries!
    m_textCtrlExclude->SetMaxLength(0); //
#endif
    assert(!contains(m_buttonClear->GetLabel(), L"&C") && !contains(m_buttonClear->GetLabel(), L"&c")); //gazillionth wxWidgets bug on OS X: Command + C mistakenly hits "&C" access key!

    m_textCtrlInclude->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(ConfigDialog::onFilterKeyEvent), nullptr, this);
    m_textCtrlExclude->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(ConfigDialog::onFilterKeyEvent), nullptr, this);

    enumTimeDescr.
    add(UnitTime::NONE, L"(" + _("None") + L")"). //meta options should be enclosed in parentheses
    add(UnitTime::TODAY,       _("Today")).
    //add(UnitTime::THIS_WEEK,   _("This week")).
    add(UnitTime::THIS_MONTH,  _("This month")).
    add(UnitTime::THIS_YEAR,   _("This year")).
    add(UnitTime::LAST_X_DAYS, _("Last x days"));

    enumSizeDescr.
    add(UnitSize::NONE, L"(" + _("None") + L")"). //meta options should be enclosed in parentheses
    add(UnitSize::BYTE, _("Byte")).
    add(UnitSize::KB,   _("KB")).
    add(UnitSize::MB,   _("MB"));

    //------------- synchronization panel -----------------
    m_toggleBtnTwoWay->SetLabel(getVariantName(DirectionConfig::TWO_WAY));
    m_toggleBtnMirror->SetLabel(getVariantName(DirectionConfig::MIRROR));
    m_toggleBtnUpdate->SetLabel(getVariantName(DirectionConfig::UPDATE));
    m_toggleBtnCustom->SetLabel(getVariantName(DirectionConfig::CUSTOM));

    m_toggleBtnTwoWay->SetToolTip(getSyncVariantDescription(DirectionConfig::TWO_WAY));
    m_toggleBtnMirror->SetToolTip(getSyncVariantDescription(DirectionConfig::MIRROR));
    m_toggleBtnUpdate->SetToolTip(getSyncVariantDescription(DirectionConfig::UPDATE));
    m_toggleBtnCustom->SetToolTip(getSyncVariantDescription(DirectionConfig::CUSTOM));

    m_bitmapLeftOnly  ->SetBitmap(mirrorIfRtl(greyScale(getResourceImage(L"cat_left_only"  ))));
    m_bitmapRightOnly ->SetBitmap(mirrorIfRtl(greyScale(getResourceImage(L"cat_right_only" ))));
    m_bitmapLeftNewer ->SetBitmap(mirrorIfRtl(greyScale(getResourceImage(L"cat_left_newer" ))));
    m_bitmapRightNewer->SetBitmap(mirrorIfRtl(greyScale(getResourceImage(L"cat_right_newer"))));
    m_bitmapDifferent ->SetBitmap(mirrorIfRtl(greyScale(getResourceImage(L"cat_different"  ))));
    m_bitmapConflict  ->SetBitmap(mirrorIfRtl(greyScale(getResourceImage(L"cat_conflict"   ))));

    setRelativeFontSize(*m_toggleBtnTwoWay, 1.25);
    setRelativeFontSize(*m_toggleBtnMirror, 1.25);
    setRelativeFontSize(*m_toggleBtnUpdate, 1.25);
    setRelativeFontSize(*m_toggleBtnCustom, 1.25);

    enumVersioningStyle.
    add(VersioningStyle::REPLACE,       _("Replace"),    _("Move files and replace if existing")).
    add(VersioningStyle::ADD_TIMESTAMP, _("Time stamp"), _("Append a time stamp to each file name"));

    //use spacer to keep dialog height stable, no matter if versioning options are visible
    bSizerDelHandling->Add(0, m_panelVersioning->GetSize().GetHeight());

    //-----------------------------------------------------

    //enable dialog-specific key local events
    Connect(wxEVT_CHAR_HOOK, wxKeyEventHandler(ConfigDialog::onLocalKeyEvent), nullptr, this);

    assert(!m_listBoxFolderPair->IsSorted());

    m_listBoxFolderPair->Append(_("Main config"));
    for (const LocalPairConfig& cfg : folderPairConfig)
    {
        const bool pairNameEmpty = trimCpy(cfg.folderPairName).empty();
        m_listBoxFolderPair->Append(L"     " + (pairNameEmpty ? L"<" + _("empty") + L">" : cfg.folderPairName));
    }

    if (folderPairConfig.empty())
    {
        m_listBoxFolderPair->Hide();
        m_staticTextFolderPairLabel->Hide();
    }

    selectFolderPairConfig(-1); //temporarily set main config as reference for window height calculations:

    GetSizer()->SetSizeHints(this); //~=Fit() + SetMinSize()
    //=> works like a charm for GTK2 with window resizing problems and title bar corruption; e.g. Debian!!!
    Center(); //needs to be re-applied after a dialog size change!

    unselectFolderPairConfig();
    selectFolderPairConfig(localPairIndexToShow);

    //more useful and Enter is redirected to m_buttonOkay anyway:
    (m_listBoxFolderPair->IsShown() ? static_cast<wxWindow*>(m_listBoxFolderPair) : m_notebook)->SetFocus();
}


void ConfigDialog::onLocalKeyEvent(wxKeyEvent& event) //process key events without explicit menu entry :)
{
    const int keyCode = event.GetKeyCode();

    switch (keyCode)
    {
        case WXK_F6:
            m_notebook->ChangeSelection(static_cast<size_t>(SyncConfigPanel::COMPARISON));
            break; //handled!
        case WXK_F7:
            m_notebook->ChangeSelection(static_cast<size_t>(SyncConfigPanel::FILTER));
            break;
        case WXK_F8:
            m_notebook->ChangeSelection(static_cast<size_t>(SyncConfigPanel::SYNC));
            break;
        default:
            event.Skip();
            return;
    }
    (m_listBoxFolderPair->IsShown() ? static_cast<wxWindow*>(m_listBoxFolderPair) : m_notebook)->SetFocus(); //GTK ignores F-keys if focus is on hidden item!
}


void ConfigDialog::onListBoxKeyEvent(wxKeyEvent& event)
{
    int keyCode = event.GetKeyCode();
    if (wxTheApp->GetLayoutDirection() == wxLayout_RightToLeft)
    {
        if (keyCode == WXK_LEFT || keyCode == WXK_NUMPAD_LEFT)
            keyCode = WXK_RIGHT;
        else if (keyCode == WXK_RIGHT || keyCode == WXK_NUMPAD_RIGHT)
            keyCode = WXK_LEFT;
    }

    switch (keyCode)
    {
        case WXK_LEFT:
        case WXK_NUMPAD_LEFT:
            switch (static_cast<SyncConfigPanel>(m_notebook->GetSelection()))
            {
                case SyncConfigPanel::COMPARISON:
                    break;
                case SyncConfigPanel::FILTER:
                    m_notebook->ChangeSelection(static_cast<size_t>(SyncConfigPanel::COMPARISON));
                    break;
                case SyncConfigPanel::SYNC:
                    m_notebook->ChangeSelection(static_cast<size_t>(SyncConfigPanel::FILTER));
                    break;
            }
            m_listBoxFolderPair->SetFocus(); //needed! wxNotebook::ChangeSelection() leads to focus change!
            return; //handled!

        case WXK_RIGHT:
        case WXK_NUMPAD_RIGHT:
            switch (static_cast<SyncConfigPanel>(m_notebook->GetSelection()))
            {
                case SyncConfigPanel::COMPARISON:
                    m_notebook->ChangeSelection(static_cast<size_t>(SyncConfigPanel::FILTER));
                    break;
                case SyncConfigPanel::FILTER:
                    m_notebook->ChangeSelection(static_cast<size_t>(SyncConfigPanel::SYNC));
                    break;
                case SyncConfigPanel::SYNC:
                    break;
            }
            m_listBoxFolderPair->SetFocus();
            return; //handled!
    }

    event.Skip();
}


void ConfigDialog::OnSelectFolderPair(wxCommandEvent& event)
{
    assert(!m_listBoxFolderPair->HasMultipleSelection()); //single-choice!
    const int selPos = event.GetSelection();
    assert(0 <= selPos && selPos < makeSigned(m_listBoxFolderPair->GetCount()));

    //m_listBoxFolderPair has no parameter ownership! => selectedPairIndexToShow has!

    if (!unselectFolderPairConfig())
    {
        //restore old selection:
        m_listBoxFolderPair->SetSelection(selectedPairIndexToShow + 1);
        return;
    }
    selectFolderPairConfig(selPos - 1);
}


void ConfigDialog::OnCompByTimeSizeDouble(wxMouseEvent& event)
{
    wxCommandEvent dummy;
    OnCompByTimeSize(dummy);
    OnOkay(dummy);
}


void ConfigDialog::OnCompBySizeDouble(wxMouseEvent& event)
{
    wxCommandEvent dummy;
    OnCompBySize(dummy);
    OnOkay(dummy);
}


void ConfigDialog::OnCompByContentDouble(wxMouseEvent& event)
{
    wxCommandEvent dummy;
    OnCompByContent(dummy);
    OnOkay(dummy);
}


void ConfigDialog::onlTimeShiftKeyDown(wxKeyEvent& event)
{
    const int keyCode = event.GetKeyCode();

    //ignore invalid input: basically only numeric keys + navigation + text edit keys should be allowed, but let's not hard-code too much...
    if ('A' <= keyCode && keyCode <= 'Z')
        return;

    event.Skip();
}


std::shared_ptr<const CompConfig> ConfigDialog::getCompConfig() const
{
    if (!m_checkBoxUseLocalCmpOptions->GetValue())
        return nullptr;

    CompConfig compCfg;
    compCfg.compareVar = localCmpVar;
    compCfg.handleSymlinks = !m_checkBoxSymlinksInclude->GetValue() ? SymLinkHandling::EXCLUDE : m_radioBtnSymlinksDirect->GetValue() ? SymLinkHandling::DIRECT : SymLinkHandling::FOLLOW;
    compCfg.ignoreTimeShiftMinutes = fromTimeShiftPhrase(copyStringTo<std::wstring>(m_textCtrlTimeShift->GetValue()));

    return std::make_shared<const CompConfig>(compCfg);
}


void ConfigDialog::setCompConfig(std::shared_ptr<const CompConfig> compCfg)
{
    m_checkBoxUseLocalCmpOptions->SetValue(compCfg != nullptr);

    if (!compCfg) //when local settings are inactive, display (current) global settings instead:
        compCfg = std::make_shared<const CompConfig>(globalCfg_.cmpConfig);

    localCmpVar = compCfg->compareVar;

    switch (compCfg->handleSymlinks)
    {
        case SymLinkHandling::EXCLUDE:
            m_checkBoxSymlinksInclude->SetValue(false);
            m_radioBtnSymlinksFollow ->SetValue(true);
            break;
        case SymLinkHandling::FOLLOW:
            m_checkBoxSymlinksInclude->SetValue(true);
            m_radioBtnSymlinksFollow->SetValue(true);
            break;
        case SymLinkHandling::DIRECT:
            m_checkBoxSymlinksInclude->SetValue(true);
            m_radioBtnSymlinksDirect->SetValue(true);
            break;
    }

    m_textCtrlTimeShift->ChangeValue(toTimeShiftPhrase(compCfg->ignoreTimeShiftMinutes));

    updateCompGui();
}


void ConfigDialog::updateCompGui()
{
    m_panelComparisonSettings->Enable(m_checkBoxUseLocalCmpOptions->GetValue());

    m_notebook->SetPageImage(static_cast<size_t>(SyncConfigPanel::COMPARISON),
                             static_cast<int>(m_checkBoxUseLocalCmpOptions->GetValue() ? ConfigTypeImage::COMPARISON : ConfigTypeImage::COMPARISON_GREY));

    //update toggle buttons -> they have no parameter-ownership at all!
    m_toggleBtnByTimeSize->SetValue(false);
    m_toggleBtnBySize    ->SetValue(false);
    m_toggleBtnByContent ->SetValue(false);

    if (m_checkBoxUseLocalCmpOptions->GetValue()) //help wxWidgets a little to render inactive config state (need on Windows, NOT on Linux!)
        switch (localCmpVar)
        {
            case CompareVariant::TIME_SIZE:
                m_toggleBtnByTimeSize->SetValue(true);
                break;
            case CompareVariant::CONTENT:
                m_toggleBtnByContent->SetValue(true);
                break;
            case CompareVariant::SIZE:
                m_toggleBtnBySize->SetValue(true);
                break;
        }

    auto setBitmap = [&](wxStaticBitmap& bmpCtrl, bool active, const wxBitmap& bmp)
    {
        if (active &&
            m_checkBoxUseLocalCmpOptions->GetValue()) //help wxWidgets a little to render inactive config state (need on Windows, NOT on Linux!)
            bmpCtrl.SetBitmap(bmp);
        else
            bmpCtrl.SetBitmap(greyScale(bmp));
    };
    setBitmap(*m_bitmapByTimeSize, localCmpVar == CompareVariant::TIME_SIZE, getResourceImage(L"file-time"));
    setBitmap(*m_bitmapByContent,  localCmpVar == CompareVariant::CONTENT,   getResourceImage(L"file-content"));
    setBitmap(*m_bitmapBySize,     localCmpVar == CompareVariant::SIZE,      getResourceImage(L"file-size"));

    //active variant description:
    setText(*m_textCtrlCompVarDescription, L"\n" + getCompVariantDescription(localCmpVar));

    m_radioBtnSymlinksDirect->Enable(m_checkBoxSymlinksInclude->GetValue());
    m_radioBtnSymlinksFollow->Enable(m_checkBoxSymlinksInclude->GetValue());
}


void ConfigDialog::onFilterKeyEvent(wxKeyEvent& event)
{
    const int keyCode = event.GetKeyCode();

    if (event.ControlDown())
        switch (keyCode)
        {
            case 'A': //CTRL + A
                if (auto textCtrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject()))
                    textCtrl->SetSelection(-1, -1); //select all
                return;
        }

    event.Skip();
}


FilterConfig ConfigDialog::getFilterConfig() const
{
    Zstring includeFilter = utfCvrtTo<Zstring>(m_textCtrlInclude->GetValue());
    Zstring exludeFilter  = utfCvrtTo<Zstring>(m_textCtrlExclude->GetValue());

#ifdef ZEN_MAC //normalize all text input (see see native_traverser_impl.h)
    try
    {
        includeFilter = osx::normalizeUtfForPosix(includeFilter.c_str()); //throw SysError
        exludeFilter  = osx::normalizeUtfForPosix( exludeFilter.c_str()); //
    }
    catch (SysError&) { assert(false); }
#endif

    return FilterConfig(includeFilter, exludeFilter,
                        m_spinCtrlTimespan->GetValue(),
                        getEnumVal(enumTimeDescr, *m_choiceUnitTimespan),
                        m_spinCtrlMinSize->GetValue(),
                        getEnumVal(enumSizeDescr, *m_choiceUnitMinSize),
                        m_spinCtrlMaxSize->GetValue(),
                        getEnumVal(enumSizeDescr, *m_choiceUnitMaxSize));
}


void ConfigDialog::setFilterConfig(const FilterConfig& filter)
{
    m_textCtrlInclude->ChangeValue(utfCvrtTo<wxString>(filter.includeFilter));
    m_textCtrlExclude->ChangeValue(utfCvrtTo<wxString>(filter.excludeFilter));

    setEnumVal(enumTimeDescr, *m_choiceUnitTimespan, filter.unitTimeSpan);
    setEnumVal(enumSizeDescr, *m_choiceUnitMinSize,  filter.unitSizeMin);
    setEnumVal(enumSizeDescr, *m_choiceUnitMaxSize,  filter.unitSizeMax);

    m_spinCtrlTimespan->SetValue(static_cast<int>(filter.timeSpan));
    m_spinCtrlMinSize ->SetValue(static_cast<int>(filter.sizeMin));
    m_spinCtrlMaxSize ->SetValue(static_cast<int>(filter.sizeMax));

    updateFilterGui();
}


void ConfigDialog::updateFilterGui()
{
    const FilterConfig activeCfg = getFilterConfig();

    m_notebook->SetPageImage(static_cast<size_t>(SyncConfigPanel::FILTER),
                             static_cast<int>(!isNullFilter(activeCfg) ? ConfigTypeImage::FILTER: ConfigTypeImage::FILTER_GREY));

    auto setStatusBitmap = [&](wxStaticBitmap& staticBmp, const wxString& bmpName, bool active)
    {
        if (active)
            staticBmp.SetBitmap(getResourceImage(bmpName));
        else
            staticBmp.SetBitmap(greyScale(getResourceImage(bmpName)));
    };
    setStatusBitmap(*m_bitmapInclude,    L"filter_include", !NameFilter::isNull(activeCfg.includeFilter, FilterConfig().excludeFilter));
    setStatusBitmap(*m_bitmapExclude,    L"filter_exclude", !NameFilter::isNull(FilterConfig().includeFilter, activeCfg.excludeFilter));
    setStatusBitmap(*m_bitmapFilterDate, L"file-time", activeCfg.unitTimeSpan != UnitTime::NONE);
    setStatusBitmap(*m_bitmapFilterSize, L"file-size",  activeCfg.unitSizeMin  != UnitSize::NONE || activeCfg.unitSizeMax != UnitSize::NONE);

    m_spinCtrlTimespan->Enable(activeCfg.unitTimeSpan == UnitTime::LAST_X_DAYS);
    m_spinCtrlMinSize ->Enable(activeCfg.unitSizeMin != UnitSize::NONE);
    m_spinCtrlMaxSize ->Enable(activeCfg.unitSizeMax != UnitSize::NONE);

    m_buttonClear->Enable(!(activeCfg == FilterConfig()));
}


void ConfigDialog::OnSyncTwoWayDouble(wxMouseEvent& event)
{
    wxCommandEvent dummy;
    OnSyncTwoWay(dummy);
    OnOkay(dummy);
}


void ConfigDialog::OnSyncMirrorDouble(wxMouseEvent& event)
{
    wxCommandEvent dummy;
    OnSyncMirror(dummy);
    OnOkay(dummy);
}


void ConfigDialog::OnSyncUpdateDouble(wxMouseEvent& event)
{
    wxCommandEvent dummy;
    OnSyncUpdate(dummy);
    OnOkay(dummy);
}


void ConfigDialog::OnSyncCustomDouble(wxMouseEvent& event)
{
    wxCommandEvent dummy;
    OnSyncCustom(dummy);
    OnOkay(dummy);
}


void toggleSyncDirection(SyncDirection& current)
{
    switch (current)
    {
        case SyncDirection::RIGHT:
            current = SyncDirection::LEFT;
            break;
        case SyncDirection::LEFT:
            current = SyncDirection::NONE;
            break;
        case SyncDirection::NONE:
            current = SyncDirection::RIGHT;
            break;
    }
}


void toggleCustomSyncConfig(DirectionConfig& directionCfg, SyncDirection& custSyncDir)
{
    switch (directionCfg.var)
    {
        case DirectionConfig::TWO_WAY:
            assert(false);
            break;
        case DirectionConfig::MIRROR:
        case DirectionConfig::UPDATE:
            directionCfg.custom = extractDirections(directionCfg);
            break;
        case DirectionConfig::CUSTOM:
            break;
    }
    toggleSyncDirection(custSyncDir);

    //some config optimization: if custom settings happen to match "mirror" or "update", just switch variant
    const DirectionSet mirrorSet = []
    {
        DirectionConfig mirrorCfg;
        mirrorCfg.var = DirectionConfig::MIRROR;
        return extractDirections(mirrorCfg);
    }();

    const DirectionSet updateSet = []
    {
        DirectionConfig updateCfg;
        updateCfg.var = DirectionConfig::UPDATE;
        return extractDirections(updateCfg);
    }();

    if (directionCfg.custom == mirrorSet)
        directionCfg.var = DirectionConfig::MIRROR;
    else if (directionCfg.custom == updateSet)
        directionCfg.var = DirectionConfig::UPDATE;
    else
        directionCfg.var = DirectionConfig::CUSTOM;
}


void ConfigDialog::OnExLeftSideOnly(wxCommandEvent& event)
{
    toggleCustomSyncConfig(directionCfg, directionCfg.custom.exLeftSideOnly);
    updateSyncGui();
}


void ConfigDialog::OnExRightSideOnly(wxCommandEvent& event)
{
    toggleCustomSyncConfig(directionCfg, directionCfg.custom.exRightSideOnly);
    updateSyncGui();
}


void ConfigDialog::OnLeftNewer(wxCommandEvent& event)
{
    toggleCustomSyncConfig(directionCfg, directionCfg.custom.leftNewer);
    updateSyncGui();
}


void ConfigDialog::OnRightNewer(wxCommandEvent& event)
{
    toggleCustomSyncConfig(directionCfg, directionCfg.custom.rightNewer);
    updateSyncGui();
}


void ConfigDialog::OnDifferent(wxCommandEvent& event)
{
    toggleCustomSyncConfig(directionCfg, directionCfg.custom.different);
    updateSyncGui();
}


void ConfigDialog::OnConflict(wxCommandEvent& event)
{
    toggleCustomSyncConfig(directionCfg, directionCfg.custom.conflict);
    updateSyncGui();
}


void updateSyncDirectionIcons(const DirectionConfig& directionCfg,
                              wxBitmapButton& buttonLeftOnly,
                              wxBitmapButton& buttonRightOnly,
                              wxBitmapButton& buttonLeftNewer,
                              wxBitmapButton& buttonRightNewer,
                              wxBitmapButton& buttonDifferent,
                              wxBitmapButton& buttonConflict)
{
    if (directionCfg.var != DirectionConfig::TWO_WAY) //automatic mode needs no sync-directions
    {
        auto updateButton = [](wxBitmapButton& button, SyncDirection dir,
                               const wchar_t* imgNameLeft, const wchar_t* imgNameNone, const wchar_t* imgNameRight,
                               SyncOperation opLeft, SyncOperation opNone, SyncOperation opRight)
        {
            switch (dir)
            {
                case SyncDirection::LEFT:
                    button.SetBitmapLabel(mirrorIfRtl(getResourceImage(imgNameLeft)));
                    button.SetToolTip(getSyncOpDescription(opLeft));
                    break;
                case SyncDirection::NONE:
                    button.SetBitmapLabel(mirrorIfRtl(getResourceImage(imgNameNone)));
                    button.SetToolTip(getSyncOpDescription(opNone));
                    break;
                case SyncDirection::RIGHT:
                    button.SetBitmapLabel(mirrorIfRtl(getResourceImage(imgNameRight)));
                    button.SetToolTip(getSyncOpDescription(opRight));
                    break;
            }
            button.SetBitmapDisabled(greyScale(button.GetBitmap())); //fix wxWidgets' all-too-clever multi-state!
            //=> the disabled bitmap is generated during first SetBitmapLabel() call but never updated again by wxWidgets!
        };

        const DirectionSet dirCfg = extractDirections(directionCfg);

        updateButton(buttonLeftOnly  , dirCfg.exLeftSideOnly , L"so_delete_left", L"so_none", L"so_create_right", SO_DELETE_LEFT    , SO_DO_NOTHING, SO_CREATE_NEW_RIGHT);
        updateButton(buttonRightOnly , dirCfg.exRightSideOnly, L"so_create_left", L"so_none", L"so_delete_right", SO_CREATE_NEW_LEFT, SO_DO_NOTHING, SO_DELETE_RIGHT    );
        updateButton(buttonLeftNewer , dirCfg.leftNewer      , L"so_update_left", L"so_none", L"so_update_right", SO_OVERWRITE_LEFT , SO_DO_NOTHING, SO_OVERWRITE_RIGHT );
        updateButton(buttonRightNewer, dirCfg.rightNewer     , L"so_update_left", L"so_none", L"so_update_right", SO_OVERWRITE_LEFT , SO_DO_NOTHING, SO_OVERWRITE_RIGHT );
        updateButton(buttonDifferent , dirCfg.different      , L"so_update_left", L"so_none", L"so_update_right", SO_OVERWRITE_LEFT , SO_DO_NOTHING, SO_OVERWRITE_RIGHT );

        switch (dirCfg.conflict)
        {
            case SyncDirection::LEFT:
                buttonConflict.SetBitmapLabel(mirrorIfRtl(getResourceImage(L"so_update_left")));
                buttonConflict.SetToolTip(getSyncOpDescription(SO_OVERWRITE_LEFT));
                break;
            case SyncDirection::NONE:
                buttonConflict.SetBitmapLabel(mirrorIfRtl(getResourceImage(L"cat_conflict"))); //silent dependency from algorithm.cpp::Redetermine!!!
                buttonConflict.SetToolTip(_("Leave as unresolved conflict"));
                break;
            case SyncDirection::RIGHT:
                buttonConflict.SetBitmapLabel(mirrorIfRtl(getResourceImage(L"so_update_right")));
                buttonConflict.SetToolTip(getSyncOpDescription(SO_OVERWRITE_RIGHT));
                break;
        }
        buttonConflict.SetBitmapDisabled(greyScale(buttonConflict.GetBitmap())); //fix wxWidgets' all-too-clever multi-state!
    }
}


void toggleDeletionPolicy(DeletionPolicy& deletionPolicy)
{
    switch (deletionPolicy)
    {
        case DeletionPolicy::PERMANENT:
            deletionPolicy = DeletionPolicy::VERSIONING;
            break;
        case DeletionPolicy::RECYCLER:
            deletionPolicy = DeletionPolicy::PERMANENT;
            break;
        case DeletionPolicy::VERSIONING:
            deletionPolicy = DeletionPolicy::RECYCLER;
            break;
    }
}


std::shared_ptr<const SyncConfig> ConfigDialog::getSyncConfig() const
{
    if (!m_checkBoxUseLocalSyncOptions->GetValue())
        return nullptr;

    SyncConfig syncCfg;
    syncCfg.directionCfg           = directionCfg;
    syncCfg.handleDeletion         = handleDeletion;
    syncCfg.versioningFolderPhrase = versioningFolder.getPath();
    syncCfg.versioningStyle        = getEnumVal(enumVersioningStyle, *m_choiceVersioningStyle);

    return std::make_shared<const SyncConfig>(syncCfg);
}


void ConfigDialog::setSyncConfig(std::shared_ptr<const SyncConfig> syncCfg)
{
    m_checkBoxUseLocalSyncOptions->SetValue(syncCfg != nullptr);

    if (!syncCfg) //when local settings are inactive, display (current) global settings instead:
        syncCfg = std::make_shared<const SyncConfig>(globalCfg_.syncCfg);

    directionCfg   = syncCfg->directionCfg; //make working copy; ownership *not* on GUI
    handleDeletion = syncCfg->handleDeletion;
    versioningFolder.setPath(syncCfg->versioningFolderPhrase);
    setEnumVal(enumVersioningStyle, *m_choiceVersioningStyle, syncCfg->versioningStyle);

    updateSyncGui();
}


void ConfigDialog::updateSyncGui()
{
#ifdef ZEN_WIN
    wxWindowUpdateLocker dummy(this); //leads to GUI corruption problems on Linux/OS X!
    wxWindowUpdateLocker dummy2(m_panelVersioning);
    wxWindowUpdateLocker dummy3(m_bpButtonLeftOnly);
    wxWindowUpdateLocker dummy4(m_bpButtonRightOnly);
    wxWindowUpdateLocker dummy5(m_bpButtonLeftNewer);
    wxWindowUpdateLocker dummy6(m_bpButtonRightNewer);
    wxWindowUpdateLocker dummy7(m_bpButtonDifferent);
    wxWindowUpdateLocker dummy8(m_bpButtonConflict);
#endif

    m_panelSyncSettings->Enable(m_checkBoxUseLocalSyncOptions->GetValue());

    m_notebook->SetPageImage(static_cast<size_t>(SyncConfigPanel::SYNC),
                             static_cast<int>(m_checkBoxUseLocalSyncOptions->GetValue() ? ConfigTypeImage::SYNC: ConfigTypeImage::SYNC_GREY));

    updateSyncDirectionIcons(directionCfg,
                             *m_bpButtonLeftOnly,
                             *m_bpButtonRightOnly,
                             *m_bpButtonLeftNewer,
                             *m_bpButtonRightNewer,
                             *m_bpButtonDifferent,
                             *m_bpButtonConflict);

    //selecting "detect move files" does not always make sense:
    m_checkBoxDetectMove->Enable(detectMovedFilesSelectable(directionCfg));
    m_checkBoxDetectMove->SetValue(detectMovedFilesEnabled(directionCfg)); //parameter NOT owned by checkbox!

    auto setBitmap = [&](wxStaticBitmap& bmpCtrl, bool active, const wxBitmap& bmp)
    {
        if (active &&
            m_checkBoxUseLocalSyncOptions->GetValue()) //help wxWidgets a little to render inactive config state (need on Windows, NOT on Linux!)
            bmpCtrl.SetBitmap(bmp);
        else
            bmpCtrl.SetBitmap(greyScale(bmp));
    };

    //display only relevant sync options
    m_bitmapDatabase     ->Show(directionCfg.var == DirectionConfig::TWO_WAY);
    fgSizerSyncDirections->Show(directionCfg.var != DirectionConfig::TWO_WAY);

    if (directionCfg.var == DirectionConfig::TWO_WAY)
        setBitmap(*m_bitmapDatabase, true, getResourceImage(L"database"));
    else
    {
        const CompareVariant activeCmpVar = m_checkBoxUseLocalCmpOptions->GetValue() ? localCmpVar : globalCfg_.cmpConfig.compareVar;

        m_bitmapLeftNewer   ->Show(activeCmpVar == CompareVariant::TIME_SIZE);
        m_bpButtonLeftNewer ->Show(activeCmpVar == CompareVariant::TIME_SIZE);
        m_bitmapRightNewer  ->Show(activeCmpVar == CompareVariant::TIME_SIZE);
        m_bpButtonRightNewer->Show(activeCmpVar == CompareVariant::TIME_SIZE);

        m_bitmapDifferent  ->Show(activeCmpVar == CompareVariant::CONTENT || activeCmpVar == CompareVariant::SIZE);
        m_bpButtonDifferent->Show(activeCmpVar == CompareVariant::CONTENT || activeCmpVar == CompareVariant::SIZE);
    }

    //active variant description:
    setText(*m_textCtrlSyncVarDescription, L"\n" + getSyncVariantDescription(directionCfg.var));

    //update toggle buttons -> they have no parameter-ownership at all!
    m_toggleBtnTwoWay->SetValue(false);
    m_toggleBtnMirror->SetValue(false);
    m_toggleBtnUpdate->SetValue(false);
    m_toggleBtnCustom->SetValue(false);

    if (m_checkBoxUseLocalSyncOptions->GetValue()) //help wxWidgets a little to render inactive config state (need on Windows, NOT on Linux!)
        switch (directionCfg.var)
        {
            case DirectionConfig::TWO_WAY:
                m_toggleBtnTwoWay->SetValue(true);
                break;
            case DirectionConfig::MIRROR:
                m_toggleBtnMirror->SetValue(true);
                break;
            case DirectionConfig::UPDATE:
                m_toggleBtnUpdate->SetValue(true);
                break;
            case DirectionConfig::CUSTOM:
                m_toggleBtnCustom->SetValue(true);
                break;
        }

    switch (handleDeletion)
    {
        case DeletionPolicy::PERMANENT:
            m_radioBtnPermanent->SetValue(true);

            m_bpButtonDeletionType->SetBitmapLabel(getResourceImage(L"delete_permanently"));
            m_bpButtonDeletionType->SetToolTip(_("Delete or overwrite files permanently"));
            break;
        case DeletionPolicy::RECYCLER:
            m_radioBtnRecycler->SetValue(true);

            m_bpButtonDeletionType->SetBitmapLabel(getResourceImage(L"delete_recycler"));
            m_bpButtonDeletionType->SetToolTip(_("Back up deleted and overwritten files in the recycle bin"));
            break;
        case DeletionPolicy::VERSIONING:
            m_radioBtnVersioning->SetValue(true);

            m_bpButtonDeletionType->SetBitmapLabel(getResourceImage(L"delete_versioning"));
            m_bpButtonDeletionType->SetToolTip(_("Move files to a user-defined folder"));
            break;
    }
    m_bpButtonDeletionType->SetBitmapDisabled(greyScale(m_bpButtonDeletionType->GetBitmap())); //fix wxWidgets' all-too-clever multi-state!


    const bool versioningSelected = handleDeletion == DeletionPolicy::VERSIONING;
    m_panelVersioning->Show(versioningSelected);

    if (versioningSelected)
    {
        updateTooltipEnumVal(enumVersioningStyle, *m_choiceVersioningStyle);

        const std::wstring pathSep = utfCvrtTo<std::wstring>(FILE_NAME_SEPARATOR);
        switch (getEnumVal(enumVersioningStyle, *m_choiceVersioningStyle))
        {
            case VersioningStyle::REPLACE:
                setText(*m_staticTextNamingCvtPart1, pathSep + _("Folder") + pathSep + _("File") + L".doc");
                setText(*m_staticTextNamingCvtPart2Bold, L"");
                setText(*m_staticTextNamingCvtPart3, L"");
                break;

            case VersioningStyle::ADD_TIMESTAMP:
                setText(*m_staticTextNamingCvtPart1, pathSep + _("Folder") + pathSep + _("File") + L".doc ");
                setText(*m_staticTextNamingCvtPart2Bold, _("YYYY-MM-DD hhmmss"));
                setText(*m_staticTextNamingCvtPart3, L".doc");
                break;
        }
    }

    m_panelSyncSettings->Layout();
    //Layout();
    //Refresh(); //removes a few artifacts when toggling display of versioning folder
}


MiscSyncConfig ConfigDialog::getMiscSyncOptions() const
{
    assert(selectedPairIndexToShow == -1);

    MiscSyncConfig miscCfg;
    miscCfg.handleError         = onGuiError;
    miscCfg.onCompletionCommand = m_comboBoxOnCompletion->getValue();
    miscCfg.onCompletionHistory = m_comboBoxOnCompletion->getHistory();
    return miscCfg;
}


void ConfigDialog::setMiscSyncOptions(const MiscSyncConfig& miscCfg)
{
    onGuiError = miscCfg.handleError;
    m_comboBoxOnCompletion->setValue(miscCfg.onCompletionCommand);
    m_comboBoxOnCompletion->setHistory(miscCfg.onCompletionHistory, onCompletionHistoryMax_);

    updateMiscGui();
}


void ConfigDialog::updateMiscGui()
{
    switch (onGuiError)
    {
        case ON_GUIERROR_IGNORE:
            m_radioBtnIgnoreErrors->SetValue(true);
            break;
        case ON_GUIERROR_POPUP:
            m_radioBtnPopupOnErrors->SetValue(true);
            break;
    }
}


void ConfigDialog::selectFolderPairConfig(int newPairIndexToShow)
{
    assert(selectedPairIndexToShow == EMPTY_PAIR_INDEX_SELECTED);
    assert(newPairIndexToShow == -1 || makeUnsigned(newPairIndexToShow) < folderPairConfig_.size());
    numeric::clamp(newPairIndexToShow, -1, static_cast<int>(folderPairConfig_.size()) - 1);

    selectedPairIndexToShow = newPairIndexToShow;
    m_listBoxFolderPair->SetSelection(newPairIndexToShow + 1);

    //show/hide controls that are only relevant for main/local config
    const bool mainConfigSelected = newPairIndexToShow < 0;
    //comparison panel:
    m_staticTextMainCompSettings->Show( mainConfigSelected && !folderPairConfig_.empty());
    m_checkBoxUseLocalCmpOptions->Show(!mainConfigSelected && !folderPairConfig_.empty());
    m_staticlineCompHeader->Show(!folderPairConfig_.empty());
    m_panelCompSettingsHolder->Layout(); //fix comp panel glitch on Win 7 125% font size
    //filter panel
    m_staticTextMainFilterSettings ->Show( mainConfigSelected && !folderPairConfig_.empty());
    m_staticTextLocalFilterSettings->Show(!mainConfigSelected && !folderPairConfig_.empty());
    m_staticlineFilterHeader->Show(!folderPairConfig_.empty());
    m_panelFilterSettingsHolder->Layout();
    //sync panel:
    m_staticTextMainSyncSettings ->Show( mainConfigSelected && !folderPairConfig_.empty());
    m_checkBoxUseLocalSyncOptions->Show(!mainConfigSelected && !folderPairConfig_.empty());
    m_staticlineSyncHeader->Show(!folderPairConfig_.empty());
    m_panelSyncSettingsHolder->Layout();
    //misc
    bSizerMiscConfig->Show(mainConfigSelected);
    Layout();

    if (mainConfigSelected)
    {
        setCompConfig     (std::make_shared<const CompConfig>(globalCfg_.cmpConfig));
        setSyncConfig     (std::make_shared<const SyncConfig>(globalCfg_.syncCfg));
        setFilterConfig   (globalCfg_.filter);
        setMiscSyncOptions(globalCfg_.miscCfg);
    }
    else
    {
        setCompConfig  (folderPairConfig_[selectedPairIndexToShow].altCmpConfig);
        setSyncConfig  (folderPairConfig_[selectedPairIndexToShow].altSyncConfig);
        setFilterConfig(folderPairConfig_[selectedPairIndexToShow].localFilter);
    }
}


bool ConfigDialog::unselectFolderPairConfig()
{
    assert(selectedPairIndexToShow == -1 ||  makeUnsigned(selectedPairIndexToShow) < folderPairConfig_.size());

    auto compCfg   = getCompConfig();
    auto syncCfg   = getSyncConfig();
    auto filterCfg = getFilterConfig();

    //------- parameter validation (BEFORE writing output!) -------

    //check if user-defined directory for deletion was specified:
    if (syncCfg && syncCfg->handleDeletion == DeletionPolicy::VERSIONING)
        if (trimCpy(syncCfg->versioningFolderPhrase).empty())
        {
            m_notebook->ChangeSelection(static_cast<size_t>(SyncConfigPanel::SYNC));
            showNotificationDialog(this, DialogInfoType::INFO, PopupDialogCfg().setMainInstructions(_("Please enter a target folder for versioning.")));
            //don't show error icon to follow "Windows' encouraging tone"
            m_versioningFolderPath->SetFocus();
            return false;
        }

    //parameter correction: include filter must not be empty!
    if (trimCpy(filterCfg.includeFilter).empty())
        filterCfg.includeFilter = FilterConfig().includeFilter; //no need to show error message, just correct user input

    //-------------------------------------------------------------

    m_comboBoxOnCompletion->addItemHistory(); //commit current "on completion" history item

    if (selectedPairIndexToShow < 0)
    {
        globalCfg_.cmpConfig = *compCfg;
        globalCfg_.syncCfg   = *syncCfg;
        globalCfg_.filter    = filterCfg;
        globalCfg_.miscCfg   = getMiscSyncOptions();
    }
    else
    {
        folderPairConfig_[selectedPairIndexToShow].altCmpConfig  = compCfg;
        folderPairConfig_[selectedPairIndexToShow].altSyncConfig = syncCfg;
        folderPairConfig_[selectedPairIndexToShow].localFilter   = filterCfg;
    }

    selectedPairIndexToShow = EMPTY_PAIR_INDEX_SELECTED;
    //m_listBoxFolderPair->SetSelection(wxNOT_FOUND); not needed, selectedPairIndexToShow has parameter ownership
    return true;
}


void ConfigDialog::OnOkay(wxCommandEvent& event)
{
    if (!unselectFolderPairConfig())
        return;

    globalCfgOut        = globalCfg_;
    folderPairConfigOut = folderPairConfig_;

    EndModal(ReturnSyncConfig::BUTTON_OKAY);
}
}

//########################################################################################

ReturnSyncConfig::ButtonPressed zen::showSyncConfigDlg(wxWindow* parent,
                                                       SyncConfigPanel panelToShow,
                                                       int localPairIndexToShow,

                                                       std::vector<LocalPairConfig>& folderPairConfig,

                                                       CompConfig&   globalCmpConfig,
                                                       SyncConfig&   globalSyncCfg,
                                                       FilterConfig& globalFilter,

                                                       xmlAccess::OnGuiError& handleError,
                                                       Zstring& onCompletionCommand,
                                                       std::vector<Zstring>& onCompletionHistory,

                                                       size_t onCompletionHistoryMax)
{
    GlobalSyncConfig globalCfg;
    globalCfg.cmpConfig = globalCmpConfig;
    globalCfg.syncCfg   = globalSyncCfg;
    globalCfg.filter    = globalFilter;

    globalCfg.miscCfg.handleError         = handleError;
    globalCfg.miscCfg.onCompletionCommand = onCompletionCommand;
    globalCfg.miscCfg.onCompletionHistory = onCompletionHistory;

    ConfigDialog syncDlg(parent,
                         panelToShow,
                         localPairIndexToShow,
                         folderPairConfig,
                         globalCfg,
                         onCompletionHistoryMax);
    auto rv = static_cast<ReturnSyncConfig::ButtonPressed>(syncDlg.ShowModal());

    if (rv != ReturnSyncConfig::BUTTON_CANCEL)
    {
        globalCmpConfig = globalCfg.cmpConfig;
        globalSyncCfg   = globalCfg.syncCfg;
        globalFilter    = globalCfg.filter;

        handleError         = globalCfg.miscCfg.handleError;
        onCompletionCommand = globalCfg.miscCfg.onCompletionCommand;
        onCompletionHistory = globalCfg.miscCfg.onCompletionHistory;
    }

    return rv;
}
