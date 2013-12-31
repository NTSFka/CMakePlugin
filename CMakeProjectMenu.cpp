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
#include "CMakeProjectMenu.h"

// wxWidgets
#include <wx/app.h>

// Codelite
#include "dirsaver.h"

// CMakePlugin
#include "CMakeGenerator.h"
#include "CMakeProjectSettings.h"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeProjectMenu::CMakeProjectMenu(CMakePlugin* plugin)
    : wxMenu()
    , m_plugin(plugin)
{
    // Open file
    Append(new wxMenuItem(this, ID_OPEN_CMAKELISTS, _("Open CMakeLists.txt")));

    AppendSeparator();

    // Export
    Append(new wxMenuItem(this, ID_EXPORT_CMAKELISTS, _("Export CMakeLists.txt")));
    Append(new wxMenuItem(this, ID_MAKE_DIRTY, _("Make dirty"),
        _("Marks CMake output files as dirty and forces cmake configuration to be call again. "
        "This is very handy when you made some changes which don't change CMakeLists.txt")
    ));

    // Binding directly to the wxMenu doesn't work
    wxTheApp->Bind(wxEVT_MENU, &CMakeProjectMenu::OnCMakeListsOpen, this, ID_OPEN_CMAKELISTS);
    wxTheApp->Bind(wxEVT_MENU, &CMakeProjectMenu::OnExport, this, ID_EXPORT_CMAKELISTS);
    wxTheApp->Bind(wxEVT_MENU, &CMakeProjectMenu::OnMakeDirty, this, ID_MAKE_DIRTY);

    wxTheApp->Bind(wxEVT_UPDATE_UI, &CMakeProjectMenu::OnFileExists, this, ID_OPEN_CMAKELISTS);
    wxTheApp->Bind(wxEVT_UPDATE_UI, &CMakeProjectMenu::OnCMakeEnabled, this, ID_MAKE_DIRTY);
}

/* ************************************************************************ */

CMakeProjectMenu::~CMakeProjectMenu()
{
    wxTheApp->Unbind(wxEVT_UPDATE_UI, &CMakeProjectMenu::OnCMakeEnabled, this, ID_MAKE_DIRTY);
    wxTheApp->Unbind(wxEVT_UPDATE_UI, &CMakeProjectMenu::OnFileExists, this, ID_OPEN_CMAKELISTS);

    wxTheApp->Unbind(wxEVT_MENU, &CMakeProjectMenu::OnMakeDirty, this, ID_MAKE_DIRTY);
    wxTheApp->Unbind(wxEVT_MENU, &CMakeProjectMenu::OnExport, this, ID_EXPORT_CMAKELISTS);
    wxTheApp->Unbind(wxEVT_MENU, &CMakeProjectMenu::OnCMakeListsOpen, this, ID_OPEN_CMAKELISTS);
}

/* ************************************************************************ */

void
CMakeProjectMenu::OnCMakeListsOpen(wxCommandEvent& event)
{
    wxUnusedVar(event);

    ProjectPtr project = m_plugin->GetSelectedProject();

    if (project)
        m_plugin->OpenCMakeLists(m_plugin->GetProjectDirectory(project->GetName()));
}

/* ************************************************************************ */

void
CMakeProjectMenu::OnExport(wxCommandEvent& event)
{
    CMakeGenerator::Generate(
        m_plugin->GetSelectedProject(),
        m_plugin->GetSelectedBuildConfig()
    );
}

/* ************************************************************************ */

void
CMakeProjectMenu::OnMakeDirty(wxCommandEvent& event)
{
    // Get settings
    const CMakeProjectSettings* settings = m_plugin->GetSelectedProjectSettings();
    // Event shouldn't be called when project is not enabled
    wxASSERT(settings && settings->enabled);

    // This function just touch .cmake_dirty file
    ProjectPtr project = m_plugin->GetSelectedProject();

    // Real project
    wxString projectName = project->GetName();

    // Project has parent project -> touch dirty file there
    if (!settings->parentProject.IsEmpty()) {
        projectName = settings->parentProject;
    }

    DirSaver ds;

    // Move to project directory
    wxSetWorkingDirectory(m_plugin->GetProjectDirectory(projectName));

    // There is no touch command in Windows
#ifdef __WXMSW__
    // @see http://stackoverflow.com/questions/51435/windows-version-of-the-unix-touch-command
    wxArrayString output;
    wxShell("type nul >> .cmake_dirty & copy .cmake_dirty +,,", output);
#else
    wxArrayString output;
    wxShell("touch .cmake_dirty", output);
#endif
}

/* ************************************************************************ */

void
CMakeProjectMenu::OnFileExists(wxUpdateUIEvent& event)
{
    ProjectPtr project = m_plugin->GetSelectedProject();

    if (project)
        event.Enable(m_plugin->ExistsCMakeLists(m_plugin->GetProjectDirectory(project->GetName())));
}

/* ************************************************************************ */

void
CMakeProjectMenu::OnCMakeEnabled(wxUpdateUIEvent& event)
{
    event.Enable(m_plugin->IsSeletedProjectEnabled());
}

/* ************************************************************************ */
