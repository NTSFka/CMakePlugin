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
#include "CMakeProjectSettingsPanel.h"

// Codelite
#include "workspace.h"
#include "imanager.h"

// CMakePlugin
#include "CMake.hpp"
#include "CMakeSettingsManager.hpp"
#include "CMakePlugin.hpp"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeProjectSettingsPanel::CMakeProjectSettingsPanel(wxWindow* parent,
                                                     CMakePlugin* plugin)
    : CMakeProjectSettingsPanelBase(parent, wxID_ANY)
    , m_plugin(plugin)
{
    if (m_plugin->GetCMake()->IsDirty())
        m_plugin->GetCMake()->LoadData();

    // Set available generators
    m_comboBoxGenerator->Insert("", 0);
    m_comboBoxGenerator->Append(m_plugin->GetCMake()->GetGenerators());

    m_comboBoxBuildType->Insert("", 0);

    // Set default setting
    ClearSettings();
}

/* ************************************************************************ */

void
CMakeProjectSettingsPanel::SetSettings(CMakeProjectSettings* settings, const wxString& config)
{
    // Remove old projects
    m_choiceParent->Clear();

    // Get all available projects
    wxArrayString projects;
    m_plugin->GetManager()->GetWorkspace()->GetProjectList(projects);

    // Foreach projects
    for (wxArrayString::const_iterator it = projects.begin(),
        ite = projects.end(); it != ite; ++it)
    {
        const CMakeSettingsManager* mgr = m_plugin->GetSettingsManager();
        wxASSERT(mgr);
        const CMakeProjectSettings* projectSettings = mgr->GetProjectSettings(*it, config);

        // Add project if CMake is enabled for it
        if (projectSettings && projectSettings->enabled && projectSettings != settings)
            m_choiceParent->Append(*it);
    }

    m_settings = settings;
    LoadSettings();
}

/* ************************************************************************ */

void
CMakeProjectSettingsPanel::LoadSettings()
{
    if (!m_settings) {
        ClearSettings();
    } else {
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
