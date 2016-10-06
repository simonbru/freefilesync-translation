// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef HELP_PROVIDER_H_85930427583421563126
#define HELP_PROVIDER_H_85930427583421563126

#if 1
namespace zen
{
inline void displayHelpEntry(const wxString& topic, wxWindow* parent) { wxLaunchDefaultBrowser(L"http://www.freefilesync.org/manual.php?topic=" + topic); }
inline void uninitializeHelp() {}
}

#else
#ifdef ZEN_WIN
    #include <zen/zstring.h>
    #include <wx/msw/helpchm.h>

#elif defined ZEN_LINUX || defined ZEN_MAC
    #include <wx/html/helpctrl.h>
#endif

#include "ffs_paths.h"


namespace zen
{
void displayHelpEntry(wxWindow* parent);
void displayHelpEntry(const wxString& topic, wxWindow* parent);

void uninitializeHelp(); //clean up gracefully during app shutdown: leaving this up to static destruction crashes on Win 8.1!






//######################## implementation ########################
namespace impl
{
//finish wxWidgets' job:
#ifdef ZEN_WIN
class FfsHelpController
{
public:
    static FfsHelpController& instance()
    {
        static FfsHelpController inst; //external linkage, despite inline definition!
        return inst;
    }

    void openSection(const wxString& section, wxWindow* parent)
    {
        //don't put in constructor: not needed if only uninitialize() is ever called!
        if (!chmHlp)
        {
            chmHlp = std::make_unique<wxCHMHelpController>();
            chmHlp->Initialize(utfCvrtTo<wxString>(zen::getResourceDir()) + L"FreeFileSync.chm");
        }

        if (section.empty())
            chmHlp->DisplayContents();
        else
            chmHlp->DisplaySection(replaceCpy(section, L'/', utfCvrtTo<wxString>(FILE_NAME_SEPARATOR)));
    }

    void uninitialize() //avoid static init/teardown order fiasco
    {
        if (chmHlp)
        {
            chmHlp->Quit(); //don't let help windows open while app is shut down! => crash on Win 8.1!
            chmHlp.reset();
        }
    }

private:
    FfsHelpController() {}
    ~FfsHelpController() { assert(!chmHlp); }

    std::unique_ptr<wxCHMHelpController> chmHlp;
};

#elif defined ZEN_LINUX || defined ZEN_MAC
struct FfsHelpController
{
    static FfsHelpController& instance()
    {
        static FfsHelpController inst;
        return inst;
    }

    void openSection(const wxString& section, wxWindow* parent)
    {
        wxHtmlModalHelp dlg(parent, utfCvrtTo<wxString>(zen::getResourceDir()) + L"Help/FreeFileSync.hhp", section,
                            wxHF_DEFAULT_STYLE | wxHF_DIALOG | wxHF_MODAL | wxHF_MERGE_BOOKS);
        (void)dlg;
        //-> solves modal help craziness on OSX!
        //-> Suse Linux: avoids program hang on exit if user closed help parent dialog before the help dialog itself was closed (why is this even possible???)
        //               avoids ESC key not being recognized by help dialog (but by parent dialog instead)
    }
    void uninitialize() {}
};
#endif
}


inline
void displayHelpEntry(const wxString& topic, wxWindow* parent)
{
    impl::FfsHelpController::instance().openSection(L"html/" + topic + L".html", parent);
}


inline
void displayHelpEntry(wxWindow* parent)
{
    impl::FfsHelpController::instance().openSection(wxString(), parent);
}

inline
void uninitializeHelp()
{
    impl::FfsHelpController::instance().uninitialize();

}
}
#endif

#endif //HELP_PROVIDER_H_85930427583421563126
