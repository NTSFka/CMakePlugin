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

// Codelite
#include "file_logger.h"
#include "imanager.h"
#include "ieditor.h"

// CMakePlugin
#include "CMakePlugin.h"
#include "CMake.h"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeHelpTab::CMakeHelpTab(wxWindow* parent, CMakePlugin* plugin)
    : CMakeHelpTabBase(parent)
    , m_plugin(plugin)
    , m_force(false)
    , m_busy(false)
{
    wxASSERT(plugin);

    Bind(wxEVT_CLOSE_WINDOW, &CMakeHelpTab::OnClose, this);
    Bind(EVT_UPDATE_THREAD, &CMakeHelpTab::OnThreadUpdate, this);

    // Initial load
    LoadData();
}

/* ************************************************************************ */

void
CMakeHelpTab::OnChangeTopic(wxCommandEvent& event)
{
    const CMake* cmake = m_plugin->GetCMake();
    wxASSERT(cmake);

    switch(event.GetInt()) {
    default:
        m_data = NULL;
        break;

    case 0:
        m_data = &cmake->GetModules();
        break;

    case 1:
        m_data = &cmake->GetCommands();
        break;

    case 2:
        m_data = &cmake->GetVariables();
        break;

    case 3:
        m_data = &cmake->GetProperties();
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
    IEditor* editor = m_plugin->GetManager()->GetActiveEditor();

    // No active editor
    if (!editor)
        return;

    // Insert value
    editor->InsertText(editor->GetCurrentPosition(), m_listBoxList->GetString(event.GetInt()));
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
    wxUnusedVar(event);

    if (!m_plugin->GetCMake()->IsOk()) {
        wxMessageBox(_("CMake application path is invalid!"), wxMessageBoxCaptionStr,  wxOK | wxCENTER | wxICON_ERROR);
        return;
    }

    LoadData(true);
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
    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &CMakeHelpTab::OnSplitterSwitch, this);
    PopupMenu(&menu);
}

/* ************************************************************************ */

void
CMakeHelpTab::OnSplitterSwitch(wxCommandEvent& event)
{
    switch (m_splitter->GetSplitMode()) {
    default:
        break;

    case wxSPLIT_HORIZONTAL:
        m_splitter->Unsplit();
        m_splitter->SplitVertically(m_splitterPageList, m_splitterPageText);
        break;

    case wxSPLIT_VERTICAL:
        m_splitter->Unsplit();
        m_splitter->SplitHorizontally(m_splitterPageList, m_splitterPageText);
        break;
    }
}

/* ************************************************************************ */

void
CMakeHelpTab::OnThreadUpdate(wxThreadEvent& event)
{
    // Notify about update
    m_gaugeLoad->SetValue(event.GetInt());
    m_gaugeLoad->Update();
}

/* ************************************************************************ */

void
CMakeHelpTab::OnClose(wxCloseEvent& event)
{
    // Wait for thread
    if (GetThread() && GetThread()->IsRunning())
        GetThread()->Wait();

    Destroy();
}

/* ************************************************************************ */

void
CMakeHelpTab::OnUpdateUi(wxUpdateUIEvent& event)
{
    event.Enable(!m_busy);
}

/* ************************************************************************ */

wxThread::ExitCode
CMakeHelpTab::Entry()
{
    // Show gauge
    if (!m_gaugeLoad->IsShown()) {
        m_gaugeLoad->Show();
        Layout();
    }

    m_busy = true;

    // Load data
    bool done = m_plugin->GetCMake()->LoadData(m_force, this);

    m_busy = false;

    // Done
    if (done) {
        m_gaugeLoad->Hide();
        Layout();

        // Set CMake version
        m_staticTextVersionValue->SetLabel(m_plugin->GetCMake()->GetVersion());

        wxCommandEvent event;
        event.SetInt(0);
        OnChangeTopic(event);
    }

    return static_cast<wxThread::ExitCode>(0);
}

/* ************************************************************************ */

void
CMakeHelpTab::LoadData(bool force)
{
    // Thread is busy
    if (m_busy) {
        return;
    }

    // Unable to reload data
    if (!m_plugin->GetCMake()->IsOk()) {
        return;
    }

    m_force = force;

    if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR) {
        CL_ERROR("Could not create the worker thread!");
        return;
    }

    if (GetThread()->Run() != wxTHREAD_NO_ERROR) {
        CL_ERROR("Could not run the worker thread!");
        return;
    }
}

/* ************************************************************************ */
