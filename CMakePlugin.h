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

#ifndef CMAKE_PLUGIN_HPP_
#define CMAKE_PLUGIN_HPP_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// C++
#include <utility>

// wxWidgets
#include <wx/scopedptr.h>
#include <wx/bookctrl.h>

// CodeLite
#include "plugin.h"
#include "project.h"
#include "build_config.h"
#include "cl_command_event.h"

// CMakePlugin
#include "CMakeConfiguration.h"

/* ************************************************************************ */
/* FORWARD DECLARATIONS                                                     */
/* ************************************************************************ */

class CMake;
class CMakeOutput;
class CMakeBuilder;
class CMakeSettingsManager;
class CMakeProjectSettingsPanel;
class CMakeProjectSettings;
class CMakeGenerator;

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief CMake plugin for CodeLite.
 */
class CMakePlugin : public IPlugin
{

// Public Constants
public:


    /**
     * @brief Name of the output file - CMakeLists.txt
     */
    static const wxString CMAKELISTS_FILE;

// Public Ctors & Dtors
public:


    /**
     * @brief Create plugin.
     *
     * @param manager
     */
    explicit CMakePlugin(IManager* manager);


    /**
     * @brief Destructor.
     */
    virtual ~CMakePlugin();


// Public Accessors
public:


    /**
     * @brief Returns a pointer to the manager object.
     *
     * @return
     */
    IManager* GetManager() const
    {
        return m_mgr;
    }


    /**
     * @brief Returns CMake application pointer.
     *
     * @return
     */
    CMake* GetCMake() const
    {
        return m_cmake.get();
    }


    /**
     * @brief Returns builder pointer.
     *
     * @return
     */
    CMakeBuilder* GetBuilder() const
    {
        return m_builder.get();
    }


    /**
     * @brief Returns settings manager pointer.
     *
     * @return
     */
    CMakeSettingsManager* GetSettingsManager() const
    {
        return m_settingsManager.get();
    }


    /**
     * @brief Returns CMake configuration.
     *
     * @return
     */
    CMakeConfiguration* GetConfiguration() const
    {
        return m_configuration.get();
    }


    /**
     * @brief Returns output window.
     *
     * @return
     */
    CMakeOutput* GetOutput() const
    {
        return m_output;
    }


    /**
     * @brief Returns generator.
     *
     * @return
     */
    CMakeGenerator* GetGenerator() const
    {
        return m_generator.get();
    }


    /**
     * @brief Returns directory where is workspace project stored.
     *
     * @return
     */
    wxString GetWorkspaceDirectory() const;


    /**
     * @brief Returns directory where is the given project stored.
     *
     * @param projectName
     *
     * @return
     */
    wxString GetProjectDirectory(const wxString& projectName) const;


    /**
     * @brief Returns seleted project.
     *
     * @return Pointer to project.
     */
    ProjectPtr GetSelectedProject() const
    {
        return m_mgr->GetSelectedProject();
    }


    /**
     * @brief Returns currently selected config for seleted project.
     *
     * @return
     */
    wxString GetSelectedProjectConfig() const;


    /**
     * @brief Returns currently selected build config.
     *
     * @return
     */
    BuildConfigPtr GetSelectedBuildConfig() const;


    /**
     * @brief Returns settings for currently selected project.
     *
     * @return Pointer to settings or nullptr if no project is seleted.
     */
    const CMakeProjectSettings* GetSelectedProjectSettings() const;


    /**
     * @brief Returns if currently selected project is enabled.
     *
     * @return
     */
    bool IsSeletedProjectEnabled() const;


// Public Operations
public:


    /**
     * @brief Creates a tool bar.
     *
     * @param parent Parent window.
     *
     * @return Codelite tool bar or NULL.
     */
    clToolBar* CreateToolBar(wxWindow* parent);


    /**
     * @brief Creates a menu for plugin.
     *
     * @param pluginsMenu
     */
    void CreatePluginMenu(wxMenu* pluginsMenu);


    /**
     * @brief Hook popup menu.
     *
     * @param menu
     * @param type
     */
    void HookPopupMenu(wxMenu* menu, MenuType type);


    /**
     * @brief allow the plugins to hook a tab in the project settings
     *
     * @param notebook the parent
     * @param configName the associated configuration name
     */
    void HookProjectSettingsTab(wxBookCtrlBase* notebook,
                                const wxString& projectName,
                                const wxString& configName);


    /**
     * @brief Unhook any tab from the project settings dialog.
     *
     * @param notebook the parent
     * @param configName the associated configuration name
     */
    void UnHookProjectSettingsTab(wxBookCtrlBase* notebook,
                                  const wxString& projectName,
                                  const wxString& configName);


    /**
     * @brief Unplug plugin.
     */
    void UnPlug();


    /**
     * @brief Check if CMakeLists.txt exists in given directory.
     *
     * @param directory
     */
    bool ExistsCMakeLists(const wxString& directory) const;


    /**
     * @brief Open CMakeLists.txt in given directory.
     *
     * @param directory
     */
    void OpenCMakeLists(const wxString& directory) const;


// Public Events
public:


    /**
     * @brief On setting dialog.
     *
     * @param event
     */
    void OnSettings(wxCommandEvent& event);


    /**
     * @brief On CMake help dialog.
     *
     * @param event
     */
    void OnHelp(wxCommandEvent& event);


    /**
     * @brief On project config saving.
     *
     * @param event
     */
    void OnSaveConfig(wxCommandEvent& event);


    /**
     * @brief On building.
     *
     * @param event
     */
    void OnBuildStarting(clBuildEvent& event);


    /**
     * @brief Returns clean command.
     *
     * @param event
     */
    void OnGetCleanCommand(clBuildEvent& event);


    /**
     * @brief Returns build command.
     *
     * @param event
     */
    void OnGetBuildCommand(clBuildEvent& event);


    /**
     * @brief Returns if custom makefile is generated.
     */
    void OnGetIsPluginMakefile(clBuildEvent& event);


    /**
     * @brief Generate custom makefile.
     *
     * @param event
     */
    void OnExportMakefile(clBuildEvent& event);


    /**
     * @brief On workspace is loaded.
     *
     * @param event
     */
    void OnWorkspaceLoaded(wxCommandEvent& event);


// Private Operations
private:


    /**
     * @brief Processes build event.
     *
     * @param event
     * @param param
     */
    void ProcessBuildEvent(clBuildEvent& event, const wxString& param = "");


// Private Data Members
private:


    /// CMake configuration.
    wxScopedPtr<CMakeConfiguration> m_configuration;

    /// CMake application
    wxScopedPtr<CMake> m_cmake;

    /// Builder
    wxScopedPtr<CMakeBuilder> m_builder;

    /// Settings manager.
    wxScopedPtr<CMakeSettingsManager> m_settingsManager;

    /// CMakeLists.txt generator
    wxScopedPtr<CMakeGenerator> m_generator;

    /// Only one is enough
    CMakeProjectSettingsPanel* m_panel;

    /// CMake output window.
    CMakeOutput* m_output;

};

/* ************************************************************************ */

#endif // CMAKE_PLUGIN_HPP_
