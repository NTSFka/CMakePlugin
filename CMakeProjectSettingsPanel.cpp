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
#include "CMakeProjectSettingsPanel.hpp"

// wxWidgets
#include <wx/sizer.h>
#include <wx/dirdlg.h>
#include <wx/filename.h>

// CMakePlugin
#include "CMake.hpp"
#include "CMakePlugin.hpp"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeProjectSettingsPanel::CMakeProjectSettingsPanel(wxWindow* parent,
                                                     ProjectPtr project,
                                                     CMakePlugin* plugin) noexcept
    : wxPanel(parent, wxID_ANY)
    , m_project(project)
    , m_plugin(plugin)
{
    SetSizer(new wxBoxSizer(wxVERTICAL));

    // Flags for one row
    wxSizerFlags checkFlags = wxSizerFlags().Expand().Border(wxTOP | wxLEFT | wxRIGHT);

    // Enable checkbox
    m_checkBoxEnable = new wxCheckBox(this, wxID_ANY, _("Enable CMake for this project"));
    m_checkBoxEnable->SetValue(false);
    GetSizer()->Add(m_checkBoxEnable, checkFlags);

    // Settings
    {
        wxFlexGridSizer* flexSizer = new wxFlexGridSizer(2, wxSizerFlags::GetDefaultBorder(), 0);
        flexSizer->SetFlexibleDirection(wxBOTH);
        flexSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
        flexSizer->AddGrowableCol(1);
        GetSizer()->Add(flexSizer, wxSizerFlags().Expand().Border());

        {
            // Flags for staticText
            wxSizerFlags textFlags = wxSizerFlags().Right().Align(wxALIGN_CENTER_VERTICAL).Right().Border(wxRIGHT);

            // Working directory
            m_staticTextSourceDirectory = new wxStaticText(this, wxID_ANY, _("Sources directory:"));
            m_staticTextSourceDirectory->SetHelpText("Directory must contains CMakeLists.txt");
            flexSizer->Add(m_staticTextSourceDirectory, textFlags);

            m_dirSelectCtrlSourceDirectory = new CMakeDirSelectCtrl(this);
            flexSizer->Add(m_dirSelectCtrlSourceDirectory, wxSizerFlags().Expand());

            // Build directory
            m_staticTextBuildDirectory = new wxStaticText(this, wxID_ANY, _("Build directory:"));
            flexSizer->Add(m_staticTextBuildDirectory, textFlags);

            m_dirSelectCtrlBuildDirectory = new CMakeDirSelectCtrl(this);
            flexSizer->Add(m_dirSelectCtrlBuildDirectory, wxSizerFlags().Expand());

            // Generator
            m_staticTextGenerator = new wxStaticText(this, wxID_ANY, _("Generator:"));
            flexSizer->Add(m_staticTextGenerator, textFlags);

            m_choiceGenerator = new wxComboBox(this, wxID_ANY);
            m_choiceGenerator->SetHelpText("Only Unix Makefile is supported for building now");
            m_choiceGenerator->Append("");  // No generator
            m_choiceGenerator->Append(m_plugin->GetCMake()->GetGenerators());
            flexSizer->Add(m_choiceGenerator, wxSizerFlags().Expand());

            // Build type
            m_staticTextBuildType = new wxStaticText(this, wxID_ANY, _("Build Type:"));
            flexSizer->Add(m_staticTextBuildType, textFlags);

            m_choiceBuildType = new wxComboBox(this, wxID_ANY);
            m_choiceBuildType->Append("");  // Do not use it
            m_choiceBuildType->Append("None");
            m_choiceBuildType->Append("Debug");
            m_choiceBuildType->Append("Release");
            m_choiceBuildType->Append("RelWithDebInfo");
            m_choiceBuildType->Append("MinSizeRel");
            flexSizer->Add(m_choiceBuildType, wxSizerFlags().Expand());

        }

        m_staticTextArguments = new wxStaticText(this, wxID_ANY, _("CMake arguments (used for configuration)"));
        GetSizer()->Add(m_staticTextArguments, wxSizerFlags().Expand().Border());

        m_textCtrlArguments = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
        GetSizer()->Add(m_textCtrlArguments, wxSizerFlags(1).Expand().Border());
    }

    // Minimum size by calculated size
    SetSizeHints(GetSize());

    GetSizer()->Fit(this);

    // Bind events
    m_staticTextSourceDirectory->Bind(wxEVT_UPDATE_UI, &CMakeProjectSettingsPanel::OnCheck, this);
    m_dirSelectCtrlSourceDirectory->Bind(wxEVT_UPDATE_UI, &CMakeProjectSettingsPanel::OnCheck, this);
    m_staticTextBuildDirectory->Bind(wxEVT_UPDATE_UI, &CMakeProjectSettingsPanel::OnCheck, this);
    m_dirSelectCtrlBuildDirectory->Bind(wxEVT_UPDATE_UI, &CMakeProjectSettingsPanel::OnCheck, this);
    m_staticTextGenerator->Bind(wxEVT_UPDATE_UI, &CMakeProjectSettingsPanel::OnCheck, this);
    m_choiceGenerator->Bind(wxEVT_UPDATE_UI, &CMakeProjectSettingsPanel::OnCheck, this);
    m_staticTextBuildType->Bind(wxEVT_UPDATE_UI, &CMakeProjectSettingsPanel::OnCheck, this);
    m_choiceBuildType->Bind(wxEVT_UPDATE_UI, &CMakeProjectSettingsPanel::OnCheck, this);
    m_staticTextArguments->Bind(wxEVT_UPDATE_UI, &CMakeProjectSettingsPanel::OnCheck, this);
    m_textCtrlArguments->Bind(wxEVT_UPDATE_UI, &CMakeProjectSettingsPanel::OnCheck, this);

    // Set default setting
    ClearSettings();
}

/* ************************************************************************ */

void
CMakeProjectSettingsPanel::LoadSettings()
{
    if (!m_settings)
    {
        ClearSettings();
    }
    else
    {
        SetCMakeEnabled(m_settings->enabled);
        SetSourceDirectory(m_settings->sourceDirectory);
        SetBuildDirectory(m_settings->buildDirectory);
        SetGenerator(m_settings->generator);
        SetBuildType(m_settings->buildType);
        SetArguments(m_settings->arguments);
    }
}

/* ************************************************************************ */

void
CMakeProjectSettingsPanel::StoreSettings()
{
    if (!m_settings)
        return;

    m_settings->enabled = IsCMakeEnabled();
    m_settings->sourceDirectory = GetSourceDirectory();
    m_settings->buildDirectory = GetBuildDirectory();
    m_settings->generator = GetGenerator();
    m_settings->buildType = GetBuildType();
    m_settings->arguments = GetArguments();
}

/* ************************************************************************ */

void
CMakeProjectSettingsPanel::ClearSettings()
{
    SetCMakeEnabled(false);
    SetSourceDirectory("");
    SetBuildDirectory("");
    SetGenerator("");
    SetArguments(wxArrayString());
}

/* ************************************************************************ */
