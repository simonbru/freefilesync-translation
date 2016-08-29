// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef FONT_SIZE_H_23849632846734343234532
#define FONT_SIZE_H_23849632846734343234532

#include <zen/basic_math.h>
#include <wx/window.h>
#include <zen/scope_guard.h>
#ifdef ZEN_WIN_VISTA_AND_LATER
    #include <zen/win.h>
    #include <Uxtheme.h>
    #include <vsstyle.h> //TEXT_MAININSTRUCTION
    #include <vssym32.h> //TMT_TEXTCOLOR
#endif


namespace zen
{
//set portable font size in multiples of the operating system's default font size
void setRelativeFontSize(wxWindow& control, double factor);
void setMainInstructionFont(wxWindow& control); //following Windows/Gnome/OS X guidelines










//###################### implementation #####################
inline
void setRelativeFontSize(wxWindow& control, double factor)
{
    wxFont font = control.GetFont();
    font.SetPointSize(numeric::round(wxNORMAL_FONT->GetPointSize() * factor));
    control.SetFont(font);
};


inline
void setMainInstructionFont(wxWindow& control)
{
    wxFont font = control.GetFont();
#ifdef ZEN_WIN //http://msdn.microsoft.com/de-DE/library/windows/desktop/aa974176#fonts
    font.SetPointSize(wxNORMAL_FONT->GetPointSize() * 4 / 3); //integer round down

#ifdef ZEN_WIN_VISTA_AND_LATER
    //get main instruction color: don't hard-code, respect accessibility!
    if (HTHEME hTheme = ::OpenThemeData(NULL,          //__in  HWND hwnd,
                                        L"TEXTSTYLE")) //__in  LPCWSTR pszClassList
    {
        ZEN_ON_SCOPE_EXIT(::CloseThemeData(hTheme));

        COLORREF cr = {};
        if (::GetThemeColor(hTheme,               //_In_   HTHEME hTheme,
                            TEXT_MAININSTRUCTION, //  _In_   int iPartId,
                            0,                    //  _In_   int iStateId,
                            TMT_TEXTCOLOR,        //  _In_   int iPropId,
                            &cr) == S_OK)         //  _Out_  COLORREF *pColor
            control.SetForegroundColour(wxColor(cr));
    }
#endif

#elif defined ZEN_LINUX //https://developer.gnome.org/hig-book/3.2/hig-book.html#alert-text
    font.SetPointSize(numeric::round(wxNORMAL_FONT->GetPointSize() * 12.0 / 11));
    font.SetWeight(wxFONTWEIGHT_BOLD);

#elif defined ZEN_MAC //https://developer.apple.com/library/mac/documentation/UserExperience/Conceptual/AppleHIGuidelines/Windows/Windows.html#//apple_ref/doc/uid/20000961-TP10
    font.SetWeight(wxFONTWEIGHT_BOLD);
#endif
    control.SetFont(font);
};
}

#endif //FONT_SIZE_H_23849632846734343234532
