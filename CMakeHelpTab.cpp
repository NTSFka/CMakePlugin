/* ************************************************************************ */
/*                                                                          */
/* CMakePlugin for Codelite                                                 */
/* Copyright (C) 2013 Jiří Fatka <ntsfka@gmail.com>                         */
/*                                                                          */
/* This program is free software: you can redistribute it and/or modify     */
/* it under the terms of the GNU General Public License as published by     */
/* the Free Software Foundation, either version 3 of the License, or        */
/* (at your option) any later version.                                      */
/*                                                                          */
/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program. If not, see <http://www.gnu.org/licenses/>.     */
/*                                                                          */
/* ************************************************************************ */

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// Declaration
#include "CMakeHelpTab.h"

// wxWidgets
#include <wx/msgdlg.h>
#include <wx/busyinfo.h>
#include <wx/menu.h>

// CMakePlugin
#include "CMake.h"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeHelpTab::CMakeHelpTab(wxWindow* parent, CMake* cmake)
    : CMakeHelpTabBase(parent)
    , m_cmake(cmake)
{
    wxASSERT(cmake);

    // Load data (cached)
    LoadData();
}

/* ************************************************************************ */

bool
CMakeHelpTab::LoadData(bool force)
{
    // Unable to reload data
    if (!m_cmake->IsOk())
        return false;

    wxBusyInfo("Please wait, loading CMake Help data...");

    // Reload data forced
    m_cmake->LoadData(force, !force);

    // Set CMake version
    m_staticTextVersionValue->SetLabel(m_cmake->GetVersion());
    m_staticTextVersionValue->SetForegroundColour(wxNullColour);

    m_radioBoxTopic->Select(0);

    return true;
}

/* ************************************************************************ */

void
CMakeHelpTab::OnChangeTopic(wxCommandEvent& event)
{
    switch(event.GetInt()) {
    default:
        m_data = NULL;
        break;
    case 0:
        m_data = &m_cmake->GetModules();
        break;
    case 1:
        m_data = &m_cmake->GetCommands();
        break;
    case 2:
        m_data = &m_cmake->GetVariables();
        break;
    case 3:
        m_data = &m_cmake->GetProperties();
        break;
    }

    // Clear filter
    m_searchCtrlFilter->Clear();

    // List all items
    ListAll();
}

/* ************************************************************************ */

void
CMakeHelpTab::OnInsert(wxCommandEvent& event)
{
    // TODO insert value
}

/* ************************************************************************ */

void
CMakeHelpTab::OnSearch(wxCommandEvent& event)
{
    // List subset
    ListFiltered(event.GetString());
}

/* ************************************************************************ */

void
CMakeHelpTab::OnSearchCancel(wxCommandEvent& event)
{
    wxUnusedVar(event);

    // List all items
    ListAll();
}

/* ************************************************************************ */

void
CMakeHelpTab::OnSelect(wxCommandEvent& event)
{
    wxASSERT(m_data);

    // Get selected name
    const wxString name = m_listBoxList->GetString(event.GetInt());

    // Find name in the data
    std::map<wxString, wxString>::const_iterator it = m_data->find(name);

    // Data found
    if (it != m_data->end()) {
        // Show required data
        m_htmlWinText->SetPage(it->second);
    }
}

/* ************************************************************************ */

void
CMakeHelpTab::OnReload(wxCommandEvent& event)
{
    if (!LoadData(true)) {
        wxMessageBox(_("Unable to find cmake. Please, check if path to cmake binary is right."),
                     wxMessageBoxCaptionStr, wxOK | wxCENTER | wxICON_ERROR);
    }
}

/* ************************************************************************ */

void
CMakeHelpTab::ListAll()
{
    // Remove old data
    m_listBoxList->Clear();
    m_htmlWinText->SetPage("");

    if (!m_data)
        return;

    // Foreach data and store names into list
    for (std::map<wxString, wxString>::const_iterator it = m_data->begin(),
         ite = m_data->end(); it != ite; ++it) {
        m_listBoxList->Append(it->first);
    }
}

/* ************************************************************************ */

void
CMakeHelpTab::ListFiltered(const wxString& search)
{
    const wxString searchMatches = "*" + search + "*";

    // Remove old data
    m_listBoxList->Clear();
    m_htmlWinText->SetPage("");

    if (!m_data)
        return;

    // Foreach data and store names into list
    for (std::map<wxString, wxString>::const_iterator it = m_data->begin(),
         ite = m_data->end(); it != ite; ++it) {
        // Store only that starts with given string
        if (it->first.Matches(searchMatches))
            m_listBoxList->Append(it->first);
    }
}

/* ************************************************************************ */

void
CMakeHelpTab::OnRightClick(wxMouseEvent& event)
{
    wxMenu menu;
    menu.Append(wxID_ANY, "Switch view", "Changes view between horizontal and vertical splitting");
    //menu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&MyFrame::OnPopupClick, NULL, this);
    PopupMenu(&menu);
}

/* ************************************************************************ */
