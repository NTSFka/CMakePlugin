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

#ifndef CMAKE_PROJECT_MENU_HPP_
#define CMAKE_PROJECT_MENU_HPP_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// wxWidgets
#include <wx/menu.h>
#include <wx/app.h>

// CMakePlugin
#include "CMakePlugin.h"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief Project menu.
 */
class CMakeProjectMenu : public wxMenu
{

// Public Enums
public:


    enum
    {
        ID_OPEN_CMAKELISTS = 2556,
        ID_EXPORT_CMAKELISTS
    };


// Public Ctors & Dtors
public:


    /**
     * @brief Create a project menu.
     *
     * @param plugin A pointer to CMake Plugin.
     */
    explicit CMakeProjectMenu(CMakePlugin* plugin)
        : wxMenu()
        , m_plugin(plugin)
    {
        // Open file
        Append(new wxMenuItem(this, ID_OPEN_CMAKELISTS, _("Open CMakeLists.txt")));

        AppendSeparator();

        // Export
        Append(new wxMenuItem(this, ID_EXPORT_CMAKELISTS, _("Export CMakeLists.txt")));

        // Binding directly to the wxMenu doesn't work
        wxTheApp->Bind(wxEVT_MENU, &CMakeProjectMenu::OnCMakeListsOpen, this, ID_OPEN_CMAKELISTS);

        wxTheApp->Bind(wxEVT_UPDATE_UI, &CMakeProjectMenu::OnFileExists, this, ID_OPEN_CMAKELISTS);
    }


    /**
     * @brief Destructor.
     */
    ~CMakeProjectMenu()
    {
        wxTheApp->Unbind(wxEVT_UPDATE_UI, &CMakeProjectMenu::OnFileExists, this, ID_OPEN_CMAKELISTS);

        wxTheApp->Unbind(wxEVT_MENU, &CMakeProjectMenu::OnCMakeListsOpen, this, ID_OPEN_CMAKELISTS);

    }


// Public Accessors
public:


    /**
     * @brief Returns currently selected project.
     *
     * @return A pointer to selected project.
     */
    ProjectPtr GetProject() const
    {
        return m_plugin->GetSelectedProject();
    }


// Public Events
public:


    /**
     * @brief On CMakeLists.txt open request.
     *
     * @param event
     */
    void OnCMakeListsOpen(wxCommandEvent& event)
    {
        wxUnusedVar(event);

        ProjectPtr project = GetProject();

        if (project)
            m_plugin->OpenCMakeLists(m_plugin->GetProjectDirectory(project->GetName()));
    }


    /**
     * @brief On CMakeLists.txt export request.
     *
     * @param event
     */
    void OnExport(wxCommandEvent& event);


    /**
     * @brief Enable open CMakeLists.txt only if exists
     *
     * @param
     */
    void OnFileExists(wxUpdateUIEvent& event)
    {
        ProjectPtr project = GetProject();

        if (project)
            event.Enable(m_plugin->ExistsCMakeLists(m_plugin->GetProjectDirectory(project->GetName())));
    }


// Private Data Members
private:


    /// A pointer to plugin.
    CMakePlugin* const m_plugin;

};

/* ************************************************************************ */

#endif // CMAKE_PROJECT_MENU_HPP_
