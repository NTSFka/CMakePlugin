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
#include "CMakeSettingsDialog.hpp"

// wxWidgets
#include <wx/utils.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

// Codelite
#include "windowattrmanager.h"

// CMakePlugin
#include "CMake.hpp"
#include "CMakeHelpDialog.hpp"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeSettingsDialog::CMakeSettingsDialog(wxWindow* parent, CMake* cmake)
    : wxDialog(parent, wxID_ANY, _("CMake Plugin Settings"))
    , m_cmake(cmake)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);

    {
        wxBoxSizer* appSizer = new wxBoxSizer(wxHORIZONTAL);
        sizer->Add(appSizer, wxSizerFlags().Expand().Border());

        m_staticCMake = new wxStaticText(this, wxID_ANY, _("CMake program:"));
        appSizer->Add(m_staticCMake, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border());

        m_textCtrlCMake = new wxTextCtrl(this, wxID_ANY, "cmake");
        appSizer->Add(m_textCtrlCMake, wxSizerFlags(1).Expand().Align(wxALIGN_CENTER_VERTICAL).Border());

        // Add find button
        m_buttonFind = new wxButton(this, wxID_FILEDLGG, "...", wxDefaultPosition, wxSize(30, -1));
        appSizer->Add(m_buttonFind, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    }

    // Add spacer
    sizer->AddStretchSpacer();

    sizer->Add(CreateStdDialogButtonSizer(wxHELP | wxCANCEL | wxOK), wxSizerFlags().Expand().Border(wxBOTTOM, 10));

    GetSizer()->Fit(this);
    Centre(wxBOTH);

    // Minimum size by calculated size
    SetSizeHints(GetSize());

    // Bind events
    Bind(wxEVT_BUTTON, &CMakeSettingsDialog::OnCMakeFind, this, wxID_FILEDLGG);
    Bind(wxEVT_BUTTON, &CMakeSettingsDialog::OnShowHelp, this, wxID_HELP);

    // Load window layout
    WindowAttrManager::Load(this, "CMakeSettingsDialog", NULL);
}

/* ************************************************************************ */

CMakeSettingsDialog::~CMakeSettingsDialog()
{
    // Save window layout
    WindowAttrManager::Save(this, "CMakeSettingsDialog", NULL);
}

/* ************************************************************************ */

void
CMakeSettingsDialog::OnCMakeFind(wxCommandEvent& event)
{
    wxString filename = wxFileSelector("CMake program");

    if (!filename.IsEmpty())
        SetCMakePath(filename);
}

/* ************************************************************************ */

void
CMakeSettingsDialog::OnShowHelp(wxCommandEvent& event)
{
    wxASSERT(m_cmake);

    if (!m_cmake->IsOk())
    {
        wxMessageBox(_("CMake program not found!"), wxMessageBoxCaptionStr, wxOK | wxCENTER | wxICON_ERROR);
        return;
    }

    if (m_cmake->IsDirty())
        m_cmake->LoadData();

    CMakeHelpDialog(NULL, m_cmake).ShowModal();
}

/* ************************************************************************ */
