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
#include "CMakeHelpDialog.hpp"

// wxWidgets
#include <wx/utils.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/splitter.h>
#include <wx/stattext.h>

// Codelite
#include "windowattrmanager.h"

// CMakePlugin
#include "CMake.hpp"

/* ************************************************************************ */
/* FUNCTIONS                                                                */
/* ************************************************************************ */

static void CreatePanel(wxNotebook* notebook, wxTextCtrl*& textCtrl,
                        void (CMakeHelpDialog::*f)(wxCommandEvent&),
                        CMakeHelpDialog* handler,
                        const wxString& title, const wxArrayString& choices,
                        bool select = false)
{
    wxPanel* panel = new wxPanel(notebook);
    panel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    wxPanel* lpanel;
    wxPanel* rpanel;

    wxSplitterWindow* splitter = new wxSplitterWindow(panel);
    splitter->SetSashGravity(0.3);
    splitter->SetMinimumPaneSize(100);
    panel->GetSizer()->Add(splitter, wxSizerFlags(1).Expand());

    // Left panel
    {
        lpanel = new wxPanel(splitter);
        lpanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));

        wxListBox* listBox = new wxListBox(lpanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices, wxLB_SINGLE | wxLB_SORT);
        lpanel->GetSizer()->Add(listBox, wxSizerFlags(1).Expand().Border(wxLEFT | wxTOP | wxBOTTOM));

        // Bind events
        listBox->Bind(wxEVT_LISTBOX, f, handler);
    }

    // Right panel
    {
        rpanel = new wxPanel(splitter);
        rpanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));

        textCtrl = new wxTextCtrl(rpanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
        rpanel->GetSizer()->Add(textCtrl, wxSizerFlags(1).Expand().Border(wxRIGHT | wxTOP | wxBOTTOM));
    }

    splitter->SplitVertically(lpanel, rpanel);

    panel->Layout();
    panel->GetSizer()->Fit(panel);

    notebook->AddPage(panel, title, select);
}

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeHelpDialog::CMakeHelpDialog(wxWindow* parent, CMake* cmake)
    : wxDialog(parent, wxID_ANY, _("CMake Help"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_cmake(cmake)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);

    {
        wxBoxSizer* versionSizer = new wxBoxSizer(wxHORIZONTAL);
        versionSizer->Add(new wxStaticText(this, wxID_ANY, _("Version:")), wxSizerFlags().Border());
        versionSizer->Add(new wxStaticText(this, wxID_ANY, cmake->GetVersion()), wxSizerFlags().Border());

        sizer->Add(versionSizer, wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT | wxTOP));
    }

    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

    // Modules
    CreatePanel(notebook, m_textCtrlModule, &CMakeHelpDialog::OnModuleSelect, this, _("Modules"), cmake->GetModules(), true);

    // Commands
    CreatePanel(notebook, m_textCtrlCommand, &CMakeHelpDialog::OnCommandSelect, this, _("Commands"), cmake->GetCommands());

    // Properties
    CreatePanel(notebook, m_textCtrlProperty, &CMakeHelpDialog::OnPropertySelect, this, _("Properties"), cmake->GetProperties());

    // Variables
    CreatePanel(notebook, m_textCtrlVariable, &CMakeHelpDialog::OnVariableSelect, this, _("Variables"), cmake->GetVariables());

    // Copyright
    {
        wxPanel* panel = new wxPanel(notebook);
        panel->SetSizer(new wxBoxSizer(wxVERTICAL));

        wxTextCtrl* textCtrl = new wxTextCtrl(panel, wxID_ANY, cmake->GetCopyright(), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
        panel->GetSizer()->Add(textCtrl, wxSizerFlags(1).Expand().Border());

        panel->Layout();
        panel->GetSizer()->Fit(panel);

        notebook->AddPage(panel, _("Copyright"));
    }

    sizer->Add(notebook, wxSizerFlags(1).Expand().Border());

    // Add buttons
    sizer->Add(CreateStdDialogButtonSizer(wxOK), wxSizerFlags().Expand().Border(wxBOTTOM));

    GetSizer()->Fit(this);
    Centre(wxBOTH);

    // Minimum size by calculated size
    SetSizeHints(GetSize());

    // Load window layout
    WindowAttrManager::Load(this, "CMakeHelpDialog", NULL);
}

/* ************************************************************************ */

CMakeHelpDialog::~CMakeHelpDialog()
{
    // Save window layout
    WindowAttrManager::Save(this, "CMakeHelpDialog", NULL);
}

/* ************************************************************************ */

void
CMakeHelpDialog::OnModuleSelect(wxCommandEvent& event)
{
    m_textCtrlModule->SetValue(m_cmake->GetModuleHelp(event.GetString()));
}

/* ************************************************************************ */

void
CMakeHelpDialog::OnCommandSelect(wxCommandEvent& event)
{
    m_textCtrlCommand->SetValue(m_cmake->GetCommandHelp(event.GetString()));
}

/* ************************************************************************ */

void
CMakeHelpDialog::OnPropertySelect(wxCommandEvent& event)
{
    m_textCtrlProperty->SetValue(m_cmake->GetPropertyHelp(event.GetString()));
}

/* ************************************************************************ */

void
CMakeHelpDialog::OnVariableSelect(wxCommandEvent& event)
{
    m_textCtrlVariable->SetValue(m_cmake->GetVariableHelp(event.GetString()));
}

/* ************************************************************************ */
