// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef CONTEXT_MENU_H_18047302153418174632141234
#define CONTEXT_MENU_H_18047302153418174632141234

#include <map>
#include <vector>
#include <functional>
#include <wx/menu.h>
#include <wx/app.h>

/*
A context menu supporting C++11 lambda callbacks!

Usage:
    ContextMenu menu;
    menu.addItem(L"Some Label", [&]{ ...do something... }); -> capture by reference is fine, as long as captured variables have at least scope of ContextMenu::popup()!
    ...
    menu.popup(wnd);
*/

namespace zen
{
class ContextMenu : private wxEvtHandler
{
public:
    ContextMenu() : menu(std::make_unique<wxMenu>()) {}

    void addItem(const wxString& label, const std::function<void()>& command, const wxBitmap* bmp = nullptr, bool enabled = true)
    {
        wxMenuItem* newItem = new wxMenuItem(menu.get(), wxID_ANY, label); //menu owns item!
        if (bmp) newItem->SetBitmap(*bmp); //do not set AFTER appending item! wxWidgets screws up for yet another crappy reason
        menu->Append(newItem);
        if (!enabled) newItem->Enable(false); //do not enable BEFORE appending item! wxWidgets screws up for yet another crappy reason
        commandList[newItem->GetId()] = command; //defer event connection, this may be a submenu only!
    }

    void addCheckBox(const wxString& label, const std::function<void()>& command, bool checked, bool enabled = true)
    {
        wxMenuItem* newItem = menu->AppendCheckItem(wxID_ANY, label);
        newItem->Check(checked);
        if (!enabled) newItem->Enable(false);
        commandList[newItem->GetId()] = command;
    }

    void addRadio(const wxString& label, const std::function<void()>& command, bool selected, bool enabled = true)
    {
        wxMenuItem* newItem = menu->AppendRadioItem(wxID_ANY, label);
        newItem->Check(selected);
        if (!enabled) newItem->Enable(false);
        commandList[newItem->GetId()] = command;
    }

    void addSeparator() { menu->AppendSeparator(); }

    void addSubmenu(const wxString& label, ContextMenu& submenu, const wxBitmap* bmp = nullptr) //invalidates submenu!
    {
        //transfer submenu commands:
        commandList.insert(submenu.commandList.begin(), submenu.commandList.end());
        submenu.commandList.clear();

        submenu.menu->SetNextHandler(menu.get()); //on wxGTK submenu events are not propagated to their parent menu by default!

        wxMenuItem* newItem = new wxMenuItem(menu.get(), wxID_ANY, label, L"", wxITEM_NORMAL, submenu.menu.release()); //menu owns item, item owns submenu!
        if (bmp) newItem->SetBitmap(*bmp); //do not set AFTER appending item! wxWidgets screws up for yet another crappy reason
        menu->Append(newItem);
    }

    void popup(wxWindow& wnd, const wxPoint& pos = wxDefaultPosition) //show popup menu + process lambdas
    {
        //eventually all events from submenu items will be received by this menu
        for (const auto& item : commandList)
            menu->Connect(item.first, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(ContextMenu::onSelection), new GenericCommand(item.second) /*pass ownership*/, this);

        wnd.PopupMenu(menu.get(), pos);
        wxTheApp->ProcessPendingEvents(); //make sure lambdas are evaluated before going out of scope;
        //although all events seem to be processed within wxWindows::PopupMenu, we shouldn't trust wxWidgets in this regard
    }

private:
    void onSelection(wxCommandEvent& event)
    {
        if (auto cmd = dynamic_cast<GenericCommand*>(event.m_callbackUserData))
            (cmd->fun_)();
    }

    struct GenericCommand : public wxObject
    {
        GenericCommand(const std::function<void()>& fun) : fun_(fun) {}
        std::function<void()> fun_;
    };

    std::unique_ptr<wxMenu> menu;
    std::map<int, std::function<void()>> commandList; //(item id, command)
};
}

#endif //CONTEXT_MENU_H_18047302153418174632141234
