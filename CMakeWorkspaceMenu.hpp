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

#ifndef CMAKE_WORKSPACE_MENU_HPP_
#define CMAKE_WORKSPACE_MENU_HPP_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// wxWidgets
#include <wx/menu.h>
#include <wx/app.h>

// CMakePlugin
#include "CMakePlugin.hpp"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief CMake Workspace popup menu.
 */
class CMakeWorkspaceMenu : public wxMenu
{

// Public Enums
public:

    enum IDs
    {
        ID_OPEN_CMAKELISTS = 2456,
        ID_EXPORT_CMAKELISTS
    };

// Public Ctors & Dtors
public:


    /**
     * @brief Create a workspace menu.
     *
     * @param plugin A pointer to CMake Plugin
     */
    explicit CMakeWorkspaceMenu(CMakePlugin* plugin)
        : wxMenu()
        , m_plugin(plugin)
    {
        // Open file
        Append(new wxMenuItem(this, ID_OPEN_CMAKELISTS, _("Open CMakeLists.txt")));

        AppendSeparator();

        // Export
        Append(new wxMenuItem(this, ID_EXPORT_CMAKELISTS, _("Export CMakeLists.txt")));

        // Bind events
        Bind(wxEVT_MENU, &CMakeWorkspaceMenu::OnCMakeListsOpen, this, ID_OPEN_CMAKELISTS);
        Bind(wxEVT_MENU, &CMakeWorkspaceMenu::OnExport, this, ID_EXPORT_CMAKELISTS);

        wxTheApp->Bind(wxEVT_UPDATE_UI, &CMakeWorkspaceMenu::OnFileExists, this, ID_OPEN_CMAKELISTS);
    }


    /**
     * @brief Destructor.
     */
    ~CMakeWorkspaceMenu()
    {
        wxTheApp->Unbind(wxEVT_UPDATE_UI, &CMakeWorkspaceMenu::OnFileExists, this, ID_OPEN_CMAKELISTS);
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
        m_plugin->OpenCMakeLists(m_plugin->GetWorkspaceDirectory());
    }


    /**
     * @brief Enable open CMakeLists.txt only if exists
     *
     * @param
     */
    void OnFileExists(wxUpdateUIEvent& event)
    {
        event.Enable(m_plugin->ExistsCMakeLists(m_plugin->GetWorkspaceDirectory()));
    }


    /**
     * @brief On CMakeLists.txt export request.
     *
     * @param event
     */
    void OnExport(wxCommandEvent& event);



// Private Data Members
private:


    /// A pointer to plugin.
    CMakePlugin* const m_plugin;

};

/* ************************************************************************ */

#endif // CMAKE_WORKSPACE_MENU_HPP_
