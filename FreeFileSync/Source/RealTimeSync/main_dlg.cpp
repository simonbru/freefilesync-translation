// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "main_dlg.h"
#include <wx/wupdlock.h>
#include <wx/filedlg.h>
#include <wx+/bitmap_button.h>
#include <wx+/string_conv.h>
#include <wx+/font_size.h>
#include <wx+/popup_dlg.h>
#include <wx+/image_resources.h>
#include <zen/file_access.h>
#include <zen/build_info.h>
#include "xml_proc.h"
#include "tray_menu.h"
#include "app_icon.h"
#include "../lib/help_provider.h"
#include "../lib/process_xml.h"
#include "../lib/ffs_paths.h"

#ifdef ZEN_WIN
    #include <wx+/mouse_move_dlg.h>

#elif defined ZEN_LINUX
    #include <gtk/gtk.h>
#elif defined ZEN_MAC
    #include <ApplicationServices/ApplicationServices.h>
    #include <wx/app.h>
#endif

using namespace zen;


namespace
{
#ifdef ZEN_WIN_VISTA_AND_LATER
bool acceptDialogFileDrop(const std::vector<Zstring>& shellItemPaths)
{
    return std::any_of(shellItemPaths.begin(), shellItemPaths.end(), [](const Zstring& shellItemPath)
    {
        return pathEndsWith(shellItemPath, Zstr(".ffs_real")) ||
               pathEndsWith(shellItemPath, Zstr(".ffs_batch"));
    });
}
#endif
}


class DirectoryPanel : public FolderGenerated
{
public:
    DirectoryPanel(wxWindow* parent) :
        FolderGenerated(parent),
        folderSelector_(*this, *m_buttonSelectFolder, *m_txtCtrlDirectory, nullptr /*staticText*/) {}

    void setPath(const Zstring& dirpath) { folderSelector_.setPath(dirpath); }
    Zstring getPath() const { return folderSelector_.getPath(); }

private:
    zen::FolderSelector2 folderSelector_;
};


void MainDialog::create(const Zstring& cfgFile)
{
    /*MainDialog* frame = */ new MainDialog(nullptr, cfgFile);
}


MainDialog::MainDialog(wxDialog* dlg, const Zstring& cfgFileName)
    : MainDlgGenerated(dlg),
      lastRunConfigPath(zen::getConfigDir() + Zstr("LastRun.ffs_real"))
{
#ifdef ZEN_WIN
    new MouseMoveWindow(*this); //ownership passed to "this"
    wxWindowUpdateLocker dummy(this); //leads to GUI corruption problems on Linux/OS X!
#endif

    SetIcon(getRtsIcon()); //set application icon

    setRelativeFontSize(*m_buttonStart, 1.5);

    m_bpButtonRemoveTopFolder->Hide();
    m_panelMainFolder->Layout();

    m_bpButtonAddFolder      ->SetBitmapLabel(getResourceImage(L"item_add"));
    m_bpButtonRemoveTopFolder->SetBitmapLabel(getResourceImage(L"item_remove"));
    setBitmapTextLabel(*m_buttonStart, getResourceImage(L"startRts").ConvertToImage(), m_buttonStart->GetLabel(), 5, 8);

    //register key event
    Connect(wxEVT_CHAR_HOOK, wxKeyEventHandler(MainDialog::OnKeyPressed), nullptr, this);

    //prepare drag & drop
    dirpathFirst = std::make_unique<FolderSelector2>(*m_panelMainFolder, *m_buttonSelectFolderMain, *m_txtCtrlDirectoryMain, m_staticTextFinalPath);

    //--------------------------- load config values ------------------------------------
    xmlAccess::XmlRealConfig newConfig;

    const Zstring currentConfigFile = cfgFileName.empty() ? lastRunConfigPath : cfgFileName;
    bool loadCfgSuccess = false;
    if (!cfgFileName.empty() || fileExists(lastRunConfigPath))
        try
        {
            std::wstring warningMsg;
            xmlAccess::readRealOrBatchConfig(currentConfigFile, newConfig, warningMsg); //throw FileError

            if (!warningMsg.empty())
                showNotificationDialog(this, DialogInfoType::WARNING, PopupDialogCfg().setDetailInstructions(warningMsg));

            loadCfgSuccess = warningMsg.empty();
        }
        catch (const FileError& e)
        {
            showNotificationDialog(this, DialogInfoType::ERROR2, PopupDialogCfg().setDetailInstructions(e.toString()));
        }

    const bool startWatchingImmediately = loadCfgSuccess && !cfgFileName.empty();

    setConfiguration(newConfig);
    setLastUsedConfig(currentConfigFile);
    //-----------------------------------------------------------------------------------------

    Center(); //needs to be re-applied after a dialog size change! (see addFolder() within setConfiguration())

    if (startWatchingImmediately) //start watch mode directly
    {
        wxCommandEvent dummy2(wxEVT_COMMAND_BUTTON_CLICKED);
        this->OnStart(dummy2);
        //don't Show()!
    }
    else
    {
        m_buttonStart->SetFocus(); //don't "steal" focus if program is running from sys-tray"
        Show();
#ifdef ZEN_MAC
        ProcessSerialNumber psn = { 0, kCurrentProcess };
        ::TransformProcessType(&psn, kProcessTransformToForegroundApplication); //show dock icon, even if we're not an application bundle
        ::SetFrontProcess(&psn);
        //if the executable is not yet in a bundle or if it is called through a launcher, we need to set focus manually:
#endif
    }

    //drag and drop .ffs_real and .ffs_batch on main dialog
#ifdef ZEN_WIN_VISTA_AND_LATER
    setupShellItemDrop(*this, acceptDialogFileDrop);
#else
    setupFileDrop(*this);
#endif
    Connect(EVENT_DROP_FILE, FileDropEventHandler(MainDialog::onFilesDropped), nullptr, this);
}


MainDialog::~MainDialog()
{
    //save current configuration
    const xmlAccess::XmlRealConfig currentCfg = getConfiguration();

    try //write config to XML
    {
        writeConfig(currentCfg, lastRunConfigPath); //throw FileError
    }
    catch (const FileError& e)
    {
        showNotificationDialog(this, DialogInfoType::ERROR2, PopupDialogCfg().setDetailInstructions(e.toString()));
    }
}


void MainDialog::onQueryEndSession()
{
    try { writeConfig(getConfiguration(), lastRunConfigPath); } //throw FileError
    catch (const FileError&) {} //we try our best do to something useful in this extreme situation - no reason to notify or even log errors here!
}


void MainDialog::OnShowHelp(wxCommandEvent& event)
{
    zen::displayHelpEntry(L"realtimesync", this);
}


void MainDialog::OnMenuAbout(wxCommandEvent& event)
{
    wxString build = __TDATE__;
    build += L" - Unicode";
#ifndef wxUSE_UNICODE
#error what is going on?
#endif

    build +=
#ifdef ZEN_BUILD_32BIT
        L" x86";
#elif defined ZEN_BUILD_64BIT
        L" x64";
#endif

    showNotificationDialog(this, DialogInfoType::INFO, PopupDialogCfg().
                           setTitle(_("About")).
                           setMainInstructions(L"RealTimeSync" L"\n\n" + replaceCpy(_("Build: %x"), L"%x", build)));
}


void MainDialog::OnKeyPressed(wxKeyEvent& event)
{
    const int keyCode = event.GetKeyCode();
    if (keyCode == WXK_ESCAPE)
    {
        Close();
        return;
    }
    event.Skip();
}


void MainDialog::OnStart(wxCommandEvent& event)
{
    Hide();
#ifdef ZEN_MAC
    //hide dock icon: else user is able to forcefully show the hidden main dialog by clicking on the icon!!
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    ::TransformProcessType(&psn, kProcessTransformToUIElementApplication);
    wxTheApp->Yield(); //required to complete TransformProcessType: else a subsequent modal dialog will be erroneously hidden!
#endif

    xmlAccess::XmlRealConfig currentCfg = getConfiguration();

    switch (rts::startDirectoryMonitor(currentCfg, xmlAccess::extractJobName(utfCvrtTo<Zstring>(currentConfigFileName))))
    {
        case rts::EXIT_APP:
            Close();
            return;

        case rts::SHOW_GUI:
            break;
    }

    Show(); //don't show for EXIT_APP
#ifdef ZEN_MAC
    ::TransformProcessType(&psn, kProcessTransformToForegroundApplication); //show dock icon again
    ::SetFrontProcess(&psn); //why isn't this covered by wxWindows::Raise()??
#endif
    Raise();
}


void MainDialog::OnConfigSave(wxCommandEvent& event)
{
    Zstring defaultFileName = currentConfigFileName.empty() ? Zstr("Realtime.ffs_real") : currentConfigFileName;
    //attention: currentConfigFileName may be an imported *.ffs_batch file! We don't want to overwrite it with a GUI config!
    if (pathEndsWith(defaultFileName, Zstr(".ffs_batch")))
        defaultFileName = beforeLast(defaultFileName, Zstr("."), IF_MISSING_RETURN_NONE) + Zstr(".ffs_real");

    wxFileDialog filePicker(this,
                            wxString(),
                            //OS X really needs dir/file separated like this:
                            utfCvrtTo<wxString>(beforeLast(defaultFileName, FILE_NAME_SEPARATOR, IF_MISSING_RETURN_NONE)), //default dir
                            utfCvrtTo<wxString>(afterLast (defaultFileName, FILE_NAME_SEPARATOR, IF_MISSING_RETURN_ALL)), //default file
                            wxString(L"RealTimeSync (*.ffs_real)|*.ffs_real") + L"|" +_("All files") + L" (*.*)|*",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filePicker.ShowModal() != wxID_OK)
        return;

    const Zstring newFileName = utfCvrtTo<Zstring>(filePicker.GetPath());

    //write config to XML
    const xmlAccess::XmlRealConfig currentCfg = getConfiguration();
    try
    {
        writeConfig(currentCfg, newFileName); //throw FileError
        setLastUsedConfig(newFileName);
    }
    catch (const FileError& e)
    {
        showNotificationDialog(this, DialogInfoType::ERROR2, PopupDialogCfg().setDetailInstructions(e.toString()));
    }
}


void MainDialog::loadConfig(const Zstring& filepath)
{
    xmlAccess::XmlRealConfig newConfig;

    try
    {
        std::wstring warningMsg;
        xmlAccess::readRealOrBatchConfig(filepath, newConfig, warningMsg); //throw FileError

        if (!warningMsg.empty())
            showNotificationDialog(this, DialogInfoType::WARNING, PopupDialogCfg().setDetailInstructions(warningMsg));
    }
    catch (const FileError& e)
    {
        showNotificationDialog(this, DialogInfoType::ERROR2, PopupDialogCfg().setDetailInstructions(e.toString()));
        return;
    }

    setConfiguration(newConfig);
    setLastUsedConfig(filepath);
}


void MainDialog::setLastUsedConfig(const Zstring& filepath)
{
    //set title
    if (filepath == lastRunConfigPath)
    {
        SetTitle(L"RealTimeSync - " + _("Automated Synchronization"));
        currentConfigFileName.clear();
    }
    else
    {
        SetTitle(utfCvrtTo<wxString>(filepath));
        currentConfigFileName = filepath;
    }
}


void MainDialog::OnConfigLoad(wxCommandEvent& event)
{
    wxFileDialog filePicker(this,
                            wxString(),
                            utfCvrtTo<wxString>(beforeLast(currentConfigFileName, FILE_NAME_SEPARATOR, IF_MISSING_RETURN_NONE)), //default dir
                            wxString(),
                            wxString(L"RealTimeSync (*.ffs_real; *.ffs_batch)|*.ffs_real;*.ffs_batch") + L"|" +_("All files") + L" (*.*)|*",
                            wxFD_OPEN);
    if (filePicker.ShowModal() == wxID_OK)
        loadConfig(utfCvrtTo<Zstring>(filePicker.GetPath()));
}


void MainDialog::onFilesDropped(FileDropEvent& event)
{
    const auto& filePaths = event.getPaths();
    if (!filePaths.empty())
        loadConfig(utfCvrtTo<Zstring>(filePaths[0]));
}


void MainDialog::setConfiguration(const xmlAccess::XmlRealConfig& cfg)
{
    //clear existing folders
    dirpathFirst->setPath(Zstring());
    clearAddFolders();

    if (!cfg.directories.empty())
    {
        //fill top folder
        dirpathFirst->setPath(*cfg.directories.begin());

        //fill additional folders
        addFolder(std::vector<Zstring>(cfg.directories.begin() + 1, cfg.directories.end()));
    }

    //fill commandline
    m_textCtrlCommand->SetValue(utfCvrtTo<wxString>(cfg.commandline));

    //set delay
    m_spinCtrlDelay->SetValue(static_cast<int>(cfg.delay));
}


xmlAccess::XmlRealConfig MainDialog::getConfiguration()
{
    xmlAccess::XmlRealConfig output;

    output.directories.push_back(utfCvrtTo<Zstring>(dirpathFirst->getPath()));
    for (const DirectoryPanel* dne : dirpathsExtra)
        output.directories.push_back(utfCvrtTo<Zstring>(dne->getPath()));

    output.commandline = utfCvrtTo<Zstring>(m_textCtrlCommand->GetValue());
    output.delay       = m_spinCtrlDelay->GetValue();

    return output;
}


void MainDialog::OnAddFolder(wxCommandEvent& event)
{
    const Zstring topFolder = utfCvrtTo<Zstring>(dirpathFirst->getPath());

    //clear existing top folder first
    dirpathFirst->setPath(Zstring());

    std::vector<Zstring> newFolders;
    newFolders.push_back(topFolder);

    addFolder(newFolders, true); //add pair in front of additonal pairs
}


void MainDialog::OnRemoveFolder(wxCommandEvent& event)
{
    //find folder pair originating the event
    const wxObject* const eventObj = event.GetEventObject();
    for (auto it = dirpathsExtra.begin(); it != dirpathsExtra.end(); ++it)
        if (eventObj == static_cast<wxObject*>((*it)->m_bpButtonRemoveFolder))
        {
            removeAddFolder(it - dirpathsExtra.begin());
            return;
        }
}


void MainDialog::OnRemoveTopFolder(wxCommandEvent& event)
{
    if (dirpathsExtra.size() > 0)
    {
        dirpathFirst->setPath(dirpathsExtra[0]->getPath());
        removeAddFolder(0); //remove first of additional folders
    }
}


#ifdef ZEN_WIN
    static const size_t MAX_ADD_FOLDERS = 8;
#elif defined ZEN_LINUX || defined ZEN_MAC
    static const size_t MAX_ADD_FOLDERS = 6;
#endif


void MainDialog::addFolder(const std::vector<Zstring>& newFolders, bool addFront)
{
    if (newFolders.size() == 0)
        return;

#ifdef ZEN_WIN
    wxWindowUpdateLocker dummy(this); //leads to GUI corruption problems on Linux/OS X!
#endif

    int folderHeight = 0;
    for (const Zstring& dirpath : newFolders)
    {
        //add new folder pair
        DirectoryPanel* newFolder = new DirectoryPanel(m_scrolledWinFolders);
        newFolder->m_bpButtonRemoveFolder->SetBitmapLabel(getResourceImage(L"item_remove"));

        //get size of scrolled window
        folderHeight = newFolder->GetSize().GetHeight();

        if (addFront)
        {
            bSizerFolders->Insert(0, newFolder, 0, wxEXPAND, 5);
            dirpathsExtra.insert(dirpathsExtra.begin(), newFolder);
        }
        else
        {
            bSizerFolders->Add(newFolder, 0, wxEXPAND, 5);
            dirpathsExtra.push_back(newFolder);
        }

        //register events
        newFolder->m_bpButtonRemoveFolder->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainDialog::OnRemoveFolder), nullptr, this );

        //insert directory name
        newFolder->setPath(dirpath);
    }

    //set size of scrolled window
    const size_t additionalRows = std::min(dirpathsExtra.size(), MAX_ADD_FOLDERS); //up to MAX_ADD_FOLDERS additional folders shall be shown
    m_scrolledWinFolders->SetMinSize(wxSize( -1, folderHeight * static_cast<int>(additionalRows)));

    //adapt delete top folder pair button
    m_bpButtonRemoveTopFolder->Show();

    GetSizer()->SetSizeHints(this); //~=Fit() + SetMinSize()
    Layout();
    Refresh(); //remove a little flicker near the start button
}


void MainDialog::removeAddFolder(size_t pos)
{
#ifdef ZEN_WIN
    wxWindowUpdateLocker dummy(this); //leads to GUI corruption problems on Linux/OS X!
#endif

    if (pos < dirpathsExtra.size())
    {
        //remove folder pairs from window
        DirectoryPanel* pairToDelete = dirpathsExtra[pos];
        const int folderHeight = pairToDelete->GetSize().GetHeight();

        bSizerFolders->Detach(pairToDelete); //Remove() does not work on Window*, so do it manually
        dirpathsExtra.erase(dirpathsExtra.begin() + pos); //remove last element in vector
        //more (non-portable) wxWidgets bullshit: on OS X wxWindow::Destroy() screws up and calls "operator delete" directly rather than
        //the deferred deletion it is expected to do (and which is implemented correctly on Windows and Linux)
        //http://bb10.com/python-wxpython-devel/2012-09/msg00004.html
        //=> since we're in a mouse button callback of a sub-component of "pairToDelete" we need to delay deletion ourselves:
        guiQueue.processAsync([] {}, [pairToDelete] { pairToDelete->Destroy(); });

        //set size of scrolled window
        const size_t additionalRows = std::min(dirpathsExtra.size(), MAX_ADD_FOLDERS); //up to MAX_ADD_FOLDERS additional folders shall be shown
        m_scrolledWinFolders->SetMinSize(wxSize( -1, folderHeight * static_cast<int>(additionalRows)));

        //adapt delete top folder pair button
        if (dirpathsExtra.size() == 0)
        {
            m_bpButtonRemoveTopFolder->Hide();
            m_panelMainFolder->Layout();
        }

        GetSizer()->SetSizeHints(this); //~=Fit() + SetMinSize()
        Layout();
        Refresh(); //remove a little flicker near the start button
    }
}


void MainDialog::clearAddFolders()
{
#ifdef ZEN_WIN
    wxWindowUpdateLocker dummy(this); //leads to GUI corruption problems on Linux/OS X!
#endif

    bSizerFolders->Clear(true);
    dirpathsExtra.clear();

    m_scrolledWinFolders->SetMinSize(wxSize(-1, 0));

    m_bpButtonRemoveTopFolder->Hide();
    m_panelMainFolder->Layout();

    GetSizer()->SetSizeHints(this); //~=Fit()
    Layout();
    Refresh(); //remove a little flicker near the start button
}
