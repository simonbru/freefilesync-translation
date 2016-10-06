///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __GUI_GENERATED_H__
#define __GUI_GENERATED_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
class FolderHistoryBox;
class OnCompletionBox;
class ToggleButton;
namespace zen{ class BitmapTextButton; }
namespace zen{ class Graph2D; }
namespace zen{ class Grid; }
namespace zen{ class TripleSplitter; }

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/bmpbuttn.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/scrolwin.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/listbox.h>
#include <wx/frame.h>
#include <wx/tglbtn.h>
#include <wx/radiobut.h>
#include <wx/hyperlink.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/notebook.h>
#include <wx/dialog.h>
#include <wx/treectrl.h>
#include <wx/animate.h>
#include <wx/grid.h>
#include <wx/calctrl.h>

#include "zen/i18n.h"

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class MainDialogGenerated
///////////////////////////////////////////////////////////////////////////////
class MainDialogGenerated : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* m_menubar1;
		wxMenu* m_menuFile;
		wxMenuItem* m_menuItemNew;
		wxMenuItem* m_menuItemLoad;
		wxMenuItem* m_menuItemSave;
		wxMenuItem* m_menuItemSaveAs;
		wxMenuItem* m_menuItemSaveAsBatch;
		wxMenu* m_menu4;
		wxMenuItem* m_menuItemCompare;
		wxMenuItem* m_menuItemCompSettings;
		wxMenuItem* m_menuItemFilter;
		wxMenuItem* m_menuItemSyncSettings;
		wxMenuItem* m_menuItemSynchronize;
		wxMenu* m_menuTools;
		wxMenuItem* m_menuItemOptions;
		wxMenu* m_menuLanguages;
		wxMenu* m_menuHelp;
		wxMenuItem* m_menuItemHelp;
		wxMenu* m_menuCheckVersion;
		wxMenuItem* m_menuItemCheckVersionNow;
		wxMenuItem* m_menuItemCheckVersionAuto;
		wxMenuItem* m_menuItemAbout;
		wxBoxSizer* bSizerPanelHolder;
		wxPanel* m_panelTopButtons;
		wxBoxSizer* bSizerTopButtons;
		zen::BitmapTextButton* m_buttonCancel;
		zen::BitmapTextButton* m_buttonCompare;
		wxBitmapButton* m_bpButtonCmpConfig;
		wxBitmapButton* m_bpButtonCmpContext;
		wxBitmapButton* m_bpButtonFilter;
		wxBitmapButton* m_bpButtonFilterContext;
		wxBitmapButton* m_bpButtonSyncConfig;
		wxBitmapButton* m_bpButtonSyncContext;
		zen::BitmapTextButton* m_buttonSync;
		wxPanel* m_panelDirectoryPairs;
		wxStaticText* m_staticTextResolvedPathL;
		wxBitmapButton* m_bpButtonAddPair;
		wxButton* m_buttonSelectFolderLeft;
		wxPanel* m_panelTopCenter;
		wxBitmapButton* m_bpButtonSwapSides;
		wxStaticText* m_staticTextResolvedPathR;
		wxButton* m_buttonSelectFolderRight;
		wxScrolledWindow* m_scrolledWindowFolderPairs;
		wxBoxSizer* bSizerAddFolderPairs;
		zen::Grid* m_gridNavi;
		wxPanel* m_panelCenter;
		zen::TripleSplitter* m_splitterMain;
		zen::Grid* m_gridMainL;
		zen::Grid* m_gridMainC;
		zen::Grid* m_gridMainR;
		wxPanel* m_panelStatusBar;
		wxBoxSizer* bSizerFileStatus;
		wxBoxSizer* bSizerStatusLeft;
		wxBoxSizer* bSizerStatusLeftDirectories;
		wxStaticBitmap* m_bitmapSmallDirectoryLeft;
		wxStaticText* m_staticTextStatusLeftDirs;
		wxBoxSizer* bSizerStatusLeftFiles;
		wxStaticBitmap* m_bitmapSmallFileLeft;
		wxStaticText* m_staticTextStatusLeftFiles;
		wxStaticText* m_staticTextStatusLeftBytes;
		wxStaticLine* m_staticline9;
		wxStaticText* m_staticTextStatusCenter;
		wxBoxSizer* bSizerStatusRight;
		wxStaticLine* m_staticline10;
		wxBoxSizer* bSizerStatusRightDirectories;
		wxStaticBitmap* m_bitmapSmallDirectoryRight;
		wxStaticText* m_staticTextStatusRightDirs;
		wxBoxSizer* bSizerStatusRightFiles;
		wxStaticBitmap* m_bitmapSmallFileRight;
		wxStaticText* m_staticTextStatusRightFiles;
		wxStaticText* m_staticTextStatusRightBytes;
		wxStaticText* m_staticTextFullStatus;
		wxPanel* m_panelSearch;
		wxBitmapButton* m_bpButtonHideSearch;
		wxStaticText* m_staticText101;
		wxTextCtrl* m_textCtrlSearchTxt;
		wxCheckBox* m_checkBoxMatchCase;
		wxPanel* m_panelConfig;
		wxBoxSizer* bSizerConfig;
		wxBitmapButton* m_bpButtonNew;
		wxStaticText* m_staticText951;
		wxBitmapButton* m_bpButtonOpen;
		wxStaticText* m_staticText95;
		wxBitmapButton* m_bpButtonSave;
		wxStaticText* m_staticText961;
		wxBitmapButton* m_bpButtonSaveAs;
		wxBitmapButton* m_bpButtonSaveAsBatch;
		wxStaticText* m_staticText97;
		wxListBox* m_listBoxHistory;
		wxPanel* m_panelViewFilter;
		wxBoxSizer* bSizerViewFilter;
		wxStaticText* m_staticTextViewType;
		ToggleButton* m_bpButtonViewTypeSyncAction;
		ToggleButton* m_bpButtonShowExcluded;
		wxStaticText* m_staticTextSelectView;
		ToggleButton* m_bpButtonShowDeleteLeft;
		ToggleButton* m_bpButtonShowUpdateLeft;
		ToggleButton* m_bpButtonShowCreateLeft;
		ToggleButton* m_bpButtonShowLeftOnly;
		ToggleButton* m_bpButtonShowLeftNewer;
		ToggleButton* m_bpButtonShowEqual;
		ToggleButton* m_bpButtonShowDoNothing;
		ToggleButton* m_bpButtonShowDifferent;
		ToggleButton* m_bpButtonShowRightNewer;
		ToggleButton* m_bpButtonShowRightOnly;
		ToggleButton* m_bpButtonShowCreateRight;
		ToggleButton* m_bpButtonShowUpdateRight;
		ToggleButton* m_bpButtonShowDeleteRight;
		ToggleButton* m_bpButtonShowConflict;
		wxStaticText* m_staticText96;
		wxPanel* m_panelStatistics;
		wxBoxSizer* bSizer1801;
		wxStaticBitmap* m_bitmapDeleteLeft;
		wxStaticText* m_staticTextDeleteLeft;
		wxStaticBitmap* m_bitmapUpdateLeft;
		wxStaticText* m_staticTextUpdateLeft;
		wxStaticBitmap* m_bitmapCreateLeft;
		wxStaticText* m_staticTextCreateLeft;
		wxStaticBitmap* m_bitmapData;
		wxStaticText* m_staticTextData;
		wxStaticBitmap* m_bitmapCreateRight;
		wxStaticText* m_staticTextCreateRight;
		wxStaticBitmap* m_bitmapUpdateRight;
		wxStaticText* m_staticTextUpdateRight;
		wxStaticBitmap* m_bitmapDeleteRight;
		wxStaticText* m_staticTextDeleteRight;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnConfigNew( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnConfigLoad( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnConfigSave( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnConfigSaveAs( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveAsBatchJob( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuQuit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCompare( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCmpSettings( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnConfigureFilter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSyncSettings( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnStartSync( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuOptions( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuFindItem( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuResetLayout( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuExportFileList( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowHelp( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuCheckVersion( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuCheckVersionAutomatically( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuAbout( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCompSettingsContext( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnCompSettingsContext( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnGlobalFilterContext( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnGlobalFilterContext( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSyncSettingsContext( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnSyncSettingsContext( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTopFolderPairAdd( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTopFolderPairRemove( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSwapSides( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTopLocalCompCfg( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTopLocalFilterCfg( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTopLocalSyncCfg( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnHideSearchPanel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSearchGridEnter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCfgHistoryKeyEvent( wxKeyEvent& event ) { event.Skip(); }
		virtual void OnLoadFromHistory( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLoadFromHistoryDoubleClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCfgHistoryRightClick( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnToggleViewType( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnToggleViewButton( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnViewButtonRightClick( wxMouseEvent& event ) { event.Skip(); }
		
	
	public:
		wxPanel* m_panelTopLeft;
		wxBitmapButton* m_bpButtonRemovePair;
		FolderHistoryBox* m_folderPathLeft;
		wxBitmapButton* m_bpButtonSelectAltFolderLeft;
		wxBitmapButton* m_bpButtonAltCompCfg;
		wxBitmapButton* m_bpButtonLocalFilter;
		wxBitmapButton* m_bpButtonAltSyncCfg;
		wxPanel* m_panelTopRight;
		FolderHistoryBox* m_folderPathRight;
		wxBitmapButton* m_bpButtonSelectAltFolderRight;
		wxBoxSizer* bSizerStatistics;
		wxBoxSizer* bSizerData;
		
		MainDialogGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("dummy"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 900,600 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~MainDialogGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ConfigDlgGenerated
///////////////////////////////////////////////////////////////////////////////
class ConfigDlgGenerated : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticTextFolderPairLabel;
		wxListBox* m_listBoxFolderPair;
		wxNotebook* m_notebook;
		wxPanel* m_panelCompSettingsHolder;
		wxBoxSizer* bSizerHeaderCompSettings;
		wxStaticText* m_staticTextMainCompSettings;
		wxCheckBox* m_checkBoxUseLocalCmpOptions;
		wxStaticLine* m_staticlineCompHeader;
		wxPanel* m_panelComparisonSettings;
		wxStaticText* m_staticText91;
		wxStaticBitmap* m_bitmapByTimeSize;
		wxToggleButton* m_toggleBtnByTimeSize;
		wxStaticBitmap* m_bitmapByContent;
		wxToggleButton* m_toggleBtnByContent;
		wxStaticBitmap* m_bitmapBySize;
		wxToggleButton* m_toggleBtnBySize;
		wxStaticLine* m_staticline42;
		wxTextCtrl* m_textCtrlCompVarDescription;
		wxStaticLine* m_staticline33;
		wxCheckBox* m_checkBoxSymlinksInclude;
		wxRadioButton* m_radioBtnSymlinksFollow;
		wxRadioButton* m_radioBtnSymlinksDirect;
		wxHyperlinkCtrl* m_hyperlink24;
		wxStaticLine* m_staticline44;
		wxStaticText* m_staticText112;
		wxTextCtrl* m_textCtrlTimeShift;
		wxStaticText* m_staticText1381;
		wxStaticText* m_staticText13811;
		wxHyperlinkCtrl* m_hyperlink241;
		wxStaticLine* m_staticline441;
		wxStaticLine* m_staticline331;
		wxPanel* m_panelFilterSettingsHolder;
		wxBoxSizer* bSizerHeaderFilterSettings;
		wxStaticText* m_staticTextMainFilterSettings;
		wxStaticText* m_staticTextLocalFilterSettings;
		wxStaticLine* m_staticlineFilterHeader;
		wxPanel* m_panelFilterSettings;
		wxStaticBitmap* m_bitmapInclude;
		wxStaticText* m_staticText78;
		wxTextCtrl* m_textCtrlInclude;
		wxStaticLine* m_staticline22;
		wxStaticBitmap* m_bitmapExclude;
		wxStaticText* m_staticText77;
		wxHyperlinkCtrl* m_hyperlink171;
		wxTextCtrl* m_textCtrlExclude;
		wxStaticLine* m_staticline24;
		wxStaticBitmap* m_bitmapFilterDate;
		wxStaticText* m_staticText79;
		wxSpinCtrl* m_spinCtrlTimespan;
		wxChoice* m_choiceUnitTimespan;
		wxStaticLine* m_staticline23;
		wxStaticBitmap* m_bitmapFilterSize;
		wxStaticText* m_staticText80;
		wxStaticText* m_staticText101;
		wxSpinCtrl* m_spinCtrlMinSize;
		wxChoice* m_choiceUnitMinSize;
		wxStaticText* m_staticText102;
		wxSpinCtrl* m_spinCtrlMaxSize;
		wxChoice* m_choiceUnitMaxSize;
		wxStaticLine* m_staticline62;
		wxStaticText* m_staticText44;
		wxStaticLine* m_staticline46;
		wxButton* m_buttonClear;
		wxPanel* m_panelSyncSettingsHolder;
		wxBoxSizer* bSizerHeaderSyncSettings;
		wxStaticText* m_staticTextMainSyncSettings;
		wxCheckBox* m_checkBoxUseLocalSyncOptions;
		wxStaticLine* m_staticlineSyncHeader;
		wxPanel* m_panelSyncSettings;
		wxStaticText* m_staticText86;
		wxToggleButton* m_toggleBtnTwoWay;
		wxToggleButton* m_toggleBtnMirror;
		wxToggleButton* m_toggleBtnUpdate;
		wxToggleButton* m_toggleBtnCustom;
		wxStaticLine* m_staticline53;
		wxBoxSizer* bSizerSyncConfig;
		wxStaticText* m_staticText119;
		wxStaticText* m_staticText120;
		wxFlexGridSizer* fgSizerSyncDirections;
		wxStaticBitmap* m_bitmapLeftOnly;
		wxStaticBitmap* m_bitmapLeftNewer;
		wxStaticBitmap* m_bitmapDifferent;
		wxStaticBitmap* m_bitmapConflict;
		wxStaticBitmap* m_bitmapRightNewer;
		wxStaticBitmap* m_bitmapRightOnly;
		wxBitmapButton* m_bpButtonLeftOnly;
		wxBitmapButton* m_bpButtonLeftNewer;
		wxBitmapButton* m_bpButtonDifferent;
		wxBitmapButton* m_bpButtonConflict;
		wxBitmapButton* m_bpButtonRightNewer;
		wxBitmapButton* m_bpButtonRightOnly;
		wxStaticBitmap* m_bitmapDatabase;
		wxStaticLine* m_staticline431;
		wxCheckBox* m_checkBoxDetectMove;
		wxHyperlinkCtrl* m_hyperlink242;
		wxStaticLine* m_staticline531;
		wxTextCtrl* m_textCtrlSyncVarDescription;
		wxStaticLine* m_staticline54;
		wxBoxSizer* bSizerDelHandling;
		wxStaticText* m_staticText87;
		wxBitmapButton* m_bpButtonDeletionType;
		wxRadioButton* m_radioBtnRecycler;
		wxRadioButton* m_radioBtnPermanent;
		wxRadioButton* m_radioBtnVersioning;
		wxPanel* m_panelVersioning;
		FolderHistoryBox* m_versioningFolderPath;
		wxButton* m_buttonSelectVersioningFolder;
		wxStaticText* m_staticText93;
		wxHyperlinkCtrl* m_hyperlink17;
		wxBoxSizer* bSizer192;
		wxChoice* m_choiceVersioningStyle;
		wxStaticText* m_staticTextNamingCvtPart1;
		wxStaticText* m_staticTextNamingCvtPart2Bold;
		wxStaticText* m_staticTextNamingCvtPart3;
		wxStaticLine* m_staticline582;
		wxBoxSizer* bSizerMiscConfig;
		wxStaticText* m_staticText88;
		wxRadioButton* m_radioBtnPopupOnErrors;
		wxRadioButton* m_radioBtnIgnoreErrors;
		wxStaticLine* m_staticline57;
		wxBoxSizer* bSizerOnCompletion;
		wxStaticText* m_staticText89;
		OnCompletionBox* m_comboBoxOnCompletion;
		wxBoxSizer* bSizerStdButtons;
		wxButton* m_buttonOkay;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void onListBoxKeyEvent( wxKeyEvent& event ) { event.Skip(); }
		virtual void OnSelectFolderPair( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnToggleLocalCompSettings( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCompByTimeSizeDouble( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnCompByTimeSize( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCompByContentDouble( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnCompByContent( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCompBySizeDouble( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnCompBySize( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnChangeCompOption( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnHelpComparisonSettings( wxHyperlinkEvent& event ) { event.Skip(); }
		virtual void onlTimeShiftKeyDown( wxKeyEvent& event ) { event.Skip(); }
		virtual void OnHelpTimeShift( wxHyperlinkEvent& event ) { event.Skip(); }
		virtual void OnChangeFilterOption( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnHelpShowExamples( wxHyperlinkEvent& event ) { event.Skip(); }
		virtual void OnFilterReset( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnToggleLocalSyncSettings( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSyncTwoWayDouble( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnSyncTwoWay( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSyncMirrorDouble( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnSyncMirror( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSyncUpdateDouble( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnSyncUpdate( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSyncCustomDouble( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnSyncCustom( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnExLeftSideOnly( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLeftNewer( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDifferent( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnConflict( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRightNewer( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnExRightSideOnly( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnToggleDetectMovedFiles( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnHelpDetectMovedFiles( wxHyperlinkEvent& event ) { event.Skip(); }
		virtual void OnToggleDeletionType( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDeletionRecycler( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDeletionPermanent( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDeletionVersioning( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnHelpVersioning( wxHyperlinkEvent& event ) { event.Skip(); }
		virtual void OnChangeSyncOption( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnErrorPopup( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnErrorIgnore( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOkay( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxBitmapButton* m_bpButtonSelectAltFolder;
		
		ConfigDlgGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("dummy"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER ); 
		~ConfigDlgGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class FolderPairPanelGenerated
///////////////////////////////////////////////////////////////////////////////
class FolderPairPanelGenerated : public wxPanel 
{
	private:
	
	protected:
		wxButton* m_buttonSelectFolderLeft;
		wxButton* m_buttonSelectFolderRight;
	
	public:
		wxPanel* m_panelLeft;
		wxBitmapButton* m_bpButtonFolderPairOptions;
		wxBitmapButton* m_bpButtonRemovePair;
		FolderHistoryBox* m_folderPathLeft;
		wxBitmapButton* m_bpButtonSelectAltFolderLeft;
		wxPanel* m_panel20;
		wxBitmapButton* m_bpButtonAltCompCfg;
		wxBitmapButton* m_bpButtonLocalFilter;
		wxBitmapButton* m_bpButtonAltSyncCfg;
		wxPanel* m_panelRight;
		FolderHistoryBox* m_folderPathRight;
		wxBitmapButton* m_bpButtonSelectAltFolderRight;
		
		FolderPairPanelGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 ); 
		~FolderPairPanelGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SftpSetupDlgGenerated
///////////////////////////////////////////////////////////////////////////////
class SftpSetupDlgGenerated : public wxDialog 
{
	private:
	
	protected:
		wxStaticBitmap* m_bitmapSftp;
		wxStaticText* m_staticTextHeader;
		wxStaticLine* m_staticline371;
		wxPanel* m_panel41;
		wxStaticText* m_staticText12311;
		wxTextCtrl* m_textCtrlServer;
		wxStaticText* m_staticText1233;
		wxTextCtrl* m_textCtrlPort;
		wxStaticText* m_staticText1381;
		wxStaticText* m_staticText1382;
		wxStaticText* m_staticText138;
		wxStaticText* m_staticText123;
		wxTextCtrl* m_textCtrlUserName;
		wxStaticText* m_staticText1231;
		wxTextCtrl* m_textCtrlPasswordVisible;
		wxTextCtrl* m_textCtrlPasswordHidden;
		wxCheckBox* m_checkBoxShowPassword;
		wxStaticText* m_staticText1232;
		wxTextCtrl* m_textCtrlServerPath;
		wxButton* m_buttonSelectFolder;
		wxStaticLine* m_staticline571;
		wxStaticText* m_staticText1361;
		wxHyperlinkCtrl* m_hyperlink171;
		wxStaticLine* m_staticline57;
		wxPanel* m_panel411;
		wxStaticText* m_staticText12341;
		wxSpinCtrl* m_spinCtrlConnectionCount;
		wxStaticText* m_staticText138111;
		wxStaticText* m_staticText1231111;
		wxSpinCtrl* m_spinCtrlChannelCount;
		wxButton* m_button42;
		wxStaticText* m_staticText138112;
		wxStaticText* m_staticText13811;
		wxStaticLine* m_staticline12;
		wxBoxSizer* bSizerStdButtons;
		wxButton* m_buttonOkay;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnToggleShowPassword( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBrowseSftpFolder( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnHelpSftpPerformance( wxHyperlinkEvent& event ) { event.Skip(); }
		virtual void OnDetectServerChannelLimit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOkay( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		SftpSetupDlgGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("SSH File Transfer Protocol"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER ); 
		~SftpSetupDlgGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SftpFolderPickerGenerated
///////////////////////////////////////////////////////////////////////////////
class SftpFolderPickerGenerated : public wxDialog 
{
	private:
	
	protected:
		wxStaticBitmap* m_bitmapSftp;
		wxStaticText* m_staticTextHeader;
		wxStaticLine* m_staticline371;
		wxPanel* m_panel41;
		wxTreeCtrl* m_treeCtrlFileSystem;
		wxStaticLine* m_staticline12;
		wxBoxSizer* bSizerStdButtons;
		wxButton* m_buttonOkay;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnExpandNode( wxTreeEvent& event ) { event.Skip(); }
		virtual void OnOkay( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		SftpFolderPickerGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("SSH File Transfer Protocol"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER ); 
		~SftpFolderPickerGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SyncConfirmationDlgGenerated
///////////////////////////////////////////////////////////////////////////////
class SyncConfirmationDlgGenerated : public wxDialog 
{
	private:
	
	protected:
		wxStaticBitmap* m_bitmapSync;
		wxStaticText* m_staticTextHeader;
		wxStaticLine* m_staticline371;
		wxPanel* m_panelStatistics;
		wxStaticLine* m_staticline38;
		wxStaticText* m_staticText84;
		wxStaticText* m_staticTextVariant;
		wxStaticLine* m_staticline14;
		wxStaticText* m_staticText83;
		wxStaticBitmap* m_bitmapDeleteLeft;
		wxStaticBitmap* m_bitmapUpdateLeft;
		wxStaticBitmap* m_bitmapCreateLeft;
		wxStaticBitmap* m_bitmapData;
		wxStaticBitmap* m_bitmapCreateRight;
		wxStaticBitmap* m_bitmapUpdateRight;
		wxStaticBitmap* m_bitmapDeleteRight;
		wxStaticText* m_staticTextDeleteLeft;
		wxStaticText* m_staticTextUpdateLeft;
		wxStaticText* m_staticTextCreateLeft;
		wxStaticText* m_staticTextData;
		wxStaticText* m_staticTextCreateRight;
		wxStaticText* m_staticTextUpdateRight;
		wxStaticText* m_staticTextDeleteRight;
		wxStaticLine* m_staticline381;
		wxStaticLine* m_staticline12;
		wxCheckBox* m_checkBoxDontShowAgain;
		wxBoxSizer* bSizerStdButtons;
		wxButton* m_buttonStartSync;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnStartSync( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		SyncConfirmationDlgGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("FreeFileSync"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~SyncConfirmationDlgGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CompareProgressDlgGenerated
///////////////////////////////////////////////////////////////////////////////
class CompareProgressDlgGenerated : public wxPanel 
{
	private:
	
	protected:
		wxPanel* m_panelStatistics;
		wxStaticText* m_staticTextItemsFoundLabel;
		wxStaticText* m_staticTextItemsFound;
		wxStaticText* m_staticTextItemsRemainingLabel;
		wxBoxSizer* bSizerItemsRemaining;
		wxStaticText* m_staticTextItemsRemaining;
		wxStaticText* m_staticTextBytesRemaining;
		wxStaticText* m_staticTextTimeRemainingLabel;
		wxStaticText* m_staticTextTimeRemaining;
		wxStaticText* m_staticTextTimeElapsed;
		wxStaticText* m_staticTextStatus;
		wxPanel* m_panelProgressLabel;
	
	public:
		zen::Graph2D* m_panelGraphProgress;
		
		CompareProgressDlgGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxRAISED_BORDER ); 
		~CompareProgressDlgGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SyncProgressPanelGenerated
///////////////////////////////////////////////////////////////////////////////
class SyncProgressPanelGenerated : public wxPanel 
{
	private:
	
	protected:
		wxBoxSizer* bSizer42;
		wxBoxSizer* bSizer171;
		wxStaticText* m_staticText87;
	
	public:
		wxBoxSizer* bSizerRoot;
		wxStaticBitmap* m_bitmapStatus;
		wxStaticText* m_staticTextPhase;
		wxAnimationCtrl* m_animCtrlSyncing;
		wxBitmapButton* m_bpButtonMinimizeToTray;
		wxBoxSizer* bSizerStatusText;
		wxStaticText* m_staticTextStatus;
		wxPanel* m_panelProgress;
		wxPanel* m_panelItemsProcessed;
		wxStaticText* m_staticTextItemsProcessed;
		wxStaticText* m_staticTextBytesProcessed;
		wxPanel* m_panelItemsRemaining;
		wxStaticText* m_staticTextItemsRemaining;
		wxStaticText* m_staticTextBytesRemaining;
		wxPanel* m_panelTimeRemaining;
		wxStaticText* m_staticTextTimeRemaining;
		wxStaticText* m_staticTextTimeElapsed;
		wxStaticBitmap* m_bitmapGraphKeyBytes;
		zen::Graph2D* m_panelGraphBytes;
		wxStaticBitmap* m_bitmapGraphKeyItems;
		zen::Graph2D* m_panelGraphItems;
		wxNotebook* m_notebookResult;
		wxStaticLine* m_staticlineFooter;
		wxBoxSizer* bSizerStdButtons;
		wxBoxSizer* bSizerOnCompletion;
		OnCompletionBox* m_comboBoxOnCompletion;
		wxButton* m_buttonClose;
		wxButton* m_buttonPause;
		wxButton* m_buttonStop;
		
		SyncProgressPanelGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
		~SyncProgressPanelGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class LogPanelGenerated
///////////////////////////////////////////////////////////////////////////////
class LogPanelGenerated : public wxPanel 
{
	private:
	
	protected:
		ToggleButton* m_bpButtonErrors;
		ToggleButton* m_bpButtonWarnings;
		ToggleButton* m_bpButtonInfo;
		wxStaticLine* m_staticline13;
		zen::Grid* m_gridMessages;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnErrors( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnWarnings( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnInfo( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		LogPanelGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
		~LogPanelGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class BatchDlgGenerated
///////////////////////////////////////////////////////////////////////////////
class BatchDlgGenerated : public wxDialog 
{
	private:
	
	protected:
		wxStaticBitmap* m_bitmapBatchJob;
		wxStaticText* m_staticTextDescr;
		wxStaticLine* m_staticline18;
		wxPanel* m_panel35;
		wxStaticText* m_staticText82;
		wxRadioButton* m_radioBtnPopupOnErrors;
		wxRadioButton* m_radioBtnIgnoreErrors;
		wxRadioButton* m_radioBtnStopOnError;
		wxStaticLine* m_staticline26;
		wxStaticText* m_staticText81;
		OnCompletionBox* m_comboBoxOnCompletion;
		wxCheckBox* m_checkBoxRunMinimized;
		wxStaticLine* m_staticline25;
		wxCheckBox* m_checkBoxGenerateLogfile;
		wxPanel* m_panelLogfile;
		wxButton* m_buttonSelectLogFolder;
		wxCheckBox* m_checkBoxLogfilesLimit;
		wxSpinCtrl* m_spinCtrlLogfileLimit;
		wxHyperlinkCtrl* m_hyperlink17;
		wxStaticLine* m_staticline13;
		wxBoxSizer* bSizerStdButtons;
		wxButton* m_buttonSaveAs;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnErrorPopup( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnErrorIgnore( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnErrorStop( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnToggleGenerateLogfile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnToggleLogfilesLimit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnHelpScheduleBatch( wxHyperlinkEvent& event ) { event.Skip(); }
		virtual void OnSaveBatchJob( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		FolderHistoryBox* m_logFolderPath;
		wxBitmapButton* m_bpButtonSelectAltLogFolder;
		
		BatchDlgGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Save as Batch Job"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~BatchDlgGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DeleteDlgGenerated
///////////////////////////////////////////////////////////////////////////////
class DeleteDlgGenerated : public wxDialog 
{
	private:
	
	protected:
		wxStaticBitmap* m_bitmapDeleteType;
		wxStaticText* m_staticTextHeader;
		wxStaticLine* m_staticline91;
		wxPanel* m_panel31;
		wxStaticLine* m_staticline42;
		wxTextCtrl* m_textCtrlFileList;
		wxStaticLine* m_staticline9;
		wxBoxSizer* bSizerStdButtons;
		wxCheckBox* m_checkBoxUseRecycler;
		wxButton* m_buttonOK;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnUseRecycler( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOK( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DeleteDlgGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Delete Items"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER ); 
		~DeleteDlgGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CopyToDlgGenerated
///////////////////////////////////////////////////////////////////////////////
class CopyToDlgGenerated : public wxDialog 
{
	private:
	
	protected:
		wxStaticBitmap* m_bitmapCopyTo;
		wxStaticText* m_staticTextHeader;
		wxStaticLine* m_staticline91;
		wxPanel* m_panel31;
		wxStaticLine* m_staticline42;
		wxTextCtrl* m_textCtrlFileList;
		wxButton* m_buttonSelectTargetFolder;
		wxStaticLine* m_staticline9;
		wxBoxSizer* bSizerStdButtons;
		wxCheckBox* m_checkBoxKeepRelPath;
		wxCheckBox* m_checkBoxOverwriteIfExists;
		wxButton* m_buttonOK;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnUseRecycler( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOK( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		FolderHistoryBox* m_targetFolderPath;
		wxBitmapButton* m_bpButtonSelectAltTargetFolder;
		
		CopyToDlgGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Copy items"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER ); 
		~CopyToDlgGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class OptionsDlgGenerated
///////////////////////////////////////////////////////////////////////////////
class OptionsDlgGenerated : public wxDialog 
{
	private:
	
	protected:
		wxStaticBitmap* m_bitmapSettings;
		wxStaticText* m_staticText44;
		wxStaticLine* m_staticline20;
		wxPanel* m_panel39;
		wxCheckBox* m_checkBoxFailSafe;
		wxStaticText* m_staticText91;
		wxBoxSizer* bSizerLockedFiles;
		wxCheckBox* m_checkBoxCopyLocked;
		wxStaticText* m_staticText92;
		wxCheckBox* m_checkBoxCopyPermissions;
		wxStaticText* m_staticText93;
		wxStaticLine* m_staticline39;
		wxStaticText* m_staticText95;
		wxStaticText* m_staticText96;
		wxSpinCtrl* m_spinCtrlAutoRetryCount;
		wxStaticText* m_staticTextAutoRetryDelay;
		wxSpinCtrl* m_spinCtrlAutoRetryDelay;
		wxStaticLine* m_staticline191;
		wxStaticText* m_staticText85;
		wxGrid* m_gridCustomCommand;
		wxBitmapButton* m_bpButtonAddRow;
		wxBitmapButton* m_bpButtonRemoveRow;
		wxHyperlinkCtrl* m_hyperlink17;
		wxStaticLine* m_staticline192;
		zen::BitmapTextButton* m_buttonResetDialogs;
		wxStaticLine* m_staticline40;
		wxStaticLine* m_staticline36;
		wxBoxSizer* bSizerStdButtons;
		wxButton* m_buttonDefault;
		wxButton* m_buttonOkay;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnToggleAutoRetryCount( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAddRow( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRemoveRow( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnHelpShowExamples( wxHyperlinkEvent& event ) { event.Skip(); }
		virtual void OnResetDialogs( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDefault( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOkay( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		OptionsDlgGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Options"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~OptionsDlgGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class TooltipDialogGenerated
///////////////////////////////////////////////////////////////////////////////
class TooltipDialogGenerated : public wxDialog 
{
	private:
	
	protected:
	
	public:
		wxStaticBitmap* m_bitmapLeft;
		wxStaticText* m_staticTextMain;
		
		TooltipDialogGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~TooltipDialogGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SelectTimespanDlgGenerated
///////////////////////////////////////////////////////////////////////////////
class SelectTimespanDlgGenerated : public wxDialog 
{
	private:
	
	protected:
		wxPanel* m_panel35;
		wxCalendarCtrl* m_calendarFrom;
		wxCalendarCtrl* m_calendarTo;
		wxStaticLine* m_staticline21;
		wxBoxSizer* bSizerStdButtons;
		wxButton* m_buttonOkay;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnChangeSelectionFrom( wxCalendarEvent& event ) { event.Skip(); }
		virtual void OnChangeSelectionTo( wxCalendarEvent& event ) { event.Skip(); }
		virtual void OnOkay( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		SelectTimespanDlgGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Select Time Span"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~SelectTimespanDlgGenerated();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class AboutDlgGenerated
///////////////////////////////////////////////////////////////////////////////
class AboutDlgGenerated : public wxDialog 
{
	private:
	
	protected:
		wxPanel* m_panel41;
		wxStaticBitmap* m_bitmapLogo;
		wxStaticLine* m_staticline341;
		wxStaticText* m_staticText96;
		wxHyperlinkCtrl* m_hyperlink11;
		wxHyperlinkCtrl* m_hyperlink7;
		wxHyperlinkCtrl* m_hyperlink14;
		wxHyperlinkCtrl* m_hyperlink16;
		wxHyperlinkCtrl* m_hyperlink15;
		wxHyperlinkCtrl* m_hyperlink12;
		wxHyperlinkCtrl* m_hyperlink13;
		wxHyperlinkCtrl* m_hyperlink10;
		wxHyperlinkCtrl* m_hyperlink18;
		wxHyperlinkCtrl* m_hyperlink9;
		wxPanel* m_panelDonate;
		wxPanel* m_panel39;
		wxStaticBitmap* m_bitmapDonate;
		wxStaticText* m_staticText83;
		wxButton* m_buttonDonate;
		wxStaticText* m_staticText94;
		wxStaticBitmap* m_bitmapHomepage;
		wxHyperlinkCtrl* m_hyperlink1;
		wxStaticBitmap* m_bitmapEmail;
		wxHyperlinkCtrl* m_hyperlink2;
		wxStaticLine* m_staticline34;
		wxStaticText* m_staticText93;
		wxStaticBitmap* m_bitmapGpl;
		wxHyperlinkCtrl* m_hyperlink5;
		wxStaticLine* m_staticline37;
		wxStaticText* m_staticText54;
		wxScrolledWindow* m_scrolledWindowTranslators;
		wxFlexGridSizer* fgSizerTranslators;
		wxStaticLine* m_staticline36;
		wxBoxSizer* bSizerStdButtons;
		wxButton* m_buttonClose;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnDonate( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOK( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		AboutDlgGenerated( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("About"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~AboutDlgGenerated();
	
};

#endif //__GUI_GENERATED_H__
