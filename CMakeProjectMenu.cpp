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
#include "CMakeProjectMenu.hpp"

// Codelite
#include "manager.h"
#include "project.h"
#include "workspace.h"
#include "build_config.h"
#include "macromanager.h"
#include "dirsaver.h"

// CMakePlugin
#include "CMakeBuilder.hpp"
#include "CMakeProjectSettings.hpp"
#include "CMakeGenerator.hpp"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

void
CMakeProjectMenu::OnExport(wxCommandEvent& event) noexcept
{
    m_plugin->GetGenerator()->Generate(
        m_plugin->GetSelectedProject(),
        m_plugin->GetSelectedBuildConfig()
    );
}

/* ************************************************************************ */

void
CMakeProjectMenu::OnConfigure(wxCommandEvent& event) noexcept
{
    // Get settings
    const CMakeProjectSettings* settings = m_plugin->GetSelectedProjectSettings();

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled)
        return;

    // Macro manager
    MacroManager* macro = MacroManager::Instance();
    wxASSERT(macro);

    // Get CMAKE
    const wxString cmake = m_plugin->GetConfiguration()->GetProgramPath();
    const ProjectPtr project = m_plugin->GetSelectedProject();
    const wxString config = m_plugin->GetSelectedProjectConfig();

    // Create command and Expand macros
    const wxString cmd = macro->Expand(
        CMakeBuilder::CreateConfigureCmd(cmake, *settings),
        m_plugin->GetManager(), project->GetName(), config
    );

    const wxString buildDir = macro->Expand(settings->buildDirectory,
        m_plugin->GetManager(), project->GetName(), config);

    // Get builder
    CMakeBuilder* builder = m_plugin->GetBuilder();
    wxASSERT(builder);

    // Create directory if missing
    if (!wxDir::Exists(buildDir))
        wxDir::Make(buildDir);

    {
        DirSaver dir;

        wxSetWorkingDirectory(wxGetCwd() + "/" + buildDir);

        // Configure
        builder->Run(cmd);
    }
}

/* ************************************************************************ */

void
CMakeProjectMenu::OnBuild(wxCommandEvent& event) noexcept
{
    // Get settings
    const CMakeProjectSettings* settings = m_plugin->GetSelectedProjectSettings();

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled)
        return;

    // Macro manager
    MacroManager* macro = MacroManager::Instance();
    wxASSERT(macro);

    const ProjectPtr project = m_plugin->GetSelectedProject();
    const wxString config = m_plugin->GetSelectedProjectConfig();

    // Get workspace
    Workspace* workspace = m_plugin->GetManager()->GetWorkspace();
    // Get build configuration
    BuildConfigPtr bldConf = workspace->GetProjBuildConf(project->GetName(), config);

    // Get Make
    const wxString make = bldConf->GetCompiler()->GetTool("MAKE");

    // Create command and Expand macros
    const wxString cmd = macro->Expand(
        CMakeBuilder::CreateBuildCmd(make, *settings),
        m_plugin->GetManager(), project->GetName(), config
    );

    const wxString buildDir = macro->Expand(settings->buildDirectory,
        m_plugin->GetManager(), project->GetName(), config);

    // Get builder
    CMakeBuilder* builder = m_plugin->GetBuilder();
    wxASSERT(builder);

    {
        DirSaver dir;

        wxSetWorkingDirectory(wxGetCwd() + "/" + buildDir);

        // Build
        builder->Run(cmd);
    }
}

/* ************************************************************************ */

void
CMakeProjectMenu::OnClear(wxCommandEvent& event) noexcept
{
    // Get settings
    const CMakeProjectSettings* settings = m_plugin->GetSelectedProjectSettings();

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled)
        return;

    // Macro manager
    MacroManager* macro = MacroManager::Instance();
    wxASSERT(macro);

    const ProjectPtr project = m_plugin->GetSelectedProject();
    const wxString config = m_plugin->GetSelectedProjectConfig();

    // Get workspace
    Workspace* workspace = m_plugin->GetManager()->GetWorkspace();
    // Get build configuration
    BuildConfigPtr bldConf = workspace->GetProjBuildConf(project->GetName(), config);

    // Get Make
    const wxString make = bldConf->GetCompiler()->GetTool("MAKE");

    // Create command and Expand macros
    const wxString cmd = macro->Expand(
        CMakeBuilder::CreateBuildCmd(make, *settings, "clean"),
        m_plugin->GetManager(), project->GetName(), config
    );

    const wxString buildDir = macro->Expand(settings->buildDirectory,
        m_plugin->GetManager(), project->GetName(), config);

    // Get builder
    CMakeBuilder* builder = m_plugin->GetBuilder();
    wxASSERT(builder);

    {
        DirSaver dir;

        wxSetWorkingDirectory(wxGetCwd() + "/" + buildDir);

        // Build
        builder->Run(cmd);
    }
}

/* ************************************************************************ */
