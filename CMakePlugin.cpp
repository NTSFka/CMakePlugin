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
#include "CMakePlugin.h"

// wxWidgets
#include <wx/app.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>
#include <wx/xrc/xmlres.h>
#include <wx/mimetype.h>
#include <wx/menu.h>
#include <wx/dir.h>

// CodeLite
#include "environmentconfig.h"
#include "event_notifier.h"
#include "globals.h"
#include "dirsaver.h"
#include "procutils.h"
#include "project.h"
#include "workspace.h"
#include "build_settings_config.h"
#include "build_config.h"
#include "build_system.h"
#include "file_logger.h"
#include "macromanager.h"

// CMakePlugin
#include "CMake.h"
#include "CMakeGenerator.h"
#include "CMakeWorkspaceMenu.h"
#include "CMakeProjectMenu.h"
#include "CMakeBuilder.h"
#include "CMakeHelpDialog.h"
#include "CMakeSettingsDialog.h"
#include "CMakeSettingsManager.h"
#include "CMakeProjectSettings.h"
#include "CMakeProjectSettingsPanel.h"
#include "CMakeGenerator.h"

/* ************************************************************************ */
/* VARIABLES                                                                */
/* ************************************************************************ */

static CMakePlugin* g_plugin = NULL;

/* ************************************************************************ */

const wxString CMakePlugin::CMAKELISTS_FILE = "CMakeLists.txt";

/* ************************************************************************ */
/* FUNCTIONS                                                                */
/* ************************************************************************ */

/**
 * @brief Creates plugin.
 *
 * @param manager Pointer to plugin manager.
 *
 * @return CMake plugin instance.
 */
extern "C" EXPORT IPlugin* CreatePlugin(IManager* manager)
{
    if (!g_plugin) {
        g_plugin = new CMakePlugin(manager);
    }

    return g_plugin;
}

/* ************************************************************************ */

/**
 * @brief Returns plugin's info.
 *
 * @return Plugin info.
 */
extern "C" EXPORT PluginInfo GetPluginInfo()
{
    PluginInfo info;

    info.SetAuthor(L"Jiří Fatka");
    info.SetName("CMakePlugin");
    info.SetDescription(_("CMake integration for CodeLite"));
    info.SetVersion("0.2");

    return info;
}

/* ************************************************************************ */

/**
 * @brief Returns required Codelite interface version.
 *
 * @return Interface version.
 */
extern "C" EXPORT int GetPluginInterfaceVersion()
{
    return PLUGIN_INTERFACE_VERSION;
}

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakePlugin::CMakePlugin(IManager* manager)
    : IPlugin(manager)
    , m_configuration(NULL)
    , m_cmake(NULL)
    , m_settingsManager(new CMakeSettingsManager(this))
    , m_generator(new CMakeGenerator())
    , m_panel(NULL)
{
    m_longName = _("CMake integration with CodeLite");
    m_shortName = "CMakePlugin";

    // Create CMake configuration file
    m_configuration.reset(new CMakeConfiguration(wxStandardPaths::Get().
        GetUserDataDir() + wxFileName::GetPathSeparator() + "config/cmake.ini")
    );

    // Create cmake application
    m_cmake.reset(new CMake(m_configuration->GetProgramPath()));

    // Bind events
    EventNotifier::Get()->Bind(wxEVT_CMD_PROJ_SETTINGS_SAVED, wxCommandEventHandler(CMakePlugin::OnSaveConfig), this);
    EventNotifier::Get()->Bind(wxEVT_BUILD_STARTING, clBuildEventHandler(CMakePlugin::OnBuildStarting), this);
    EventNotifier::Get()->Bind(wxEVT_GET_PROJECT_BUILD_CMD, clBuildEventHandler(CMakePlugin::OnGetBuildCommand), this);
    EventNotifier::Get()->Bind(wxEVT_GET_PROJECT_CLEAN_CMD, clBuildEventHandler(CMakePlugin::OnGetCleanCommand), this);
    EventNotifier::Get()->Bind(wxEVT_GET_IS_PLUGIN_MAKEFILE, clBuildEventHandler(CMakePlugin::OnGetIsPluginMakefile), this);
    EventNotifier::Get()->Bind(wxEVT_PLUGIN_EXPORT_MAKEFILE, clBuildEventHandler(CMakePlugin::OnExportMakefile), this);
    EventNotifier::Get()->Bind(wxEVT_WORKSPACE_LOADED, wxCommandEventHandler(CMakePlugin::OnWorkspaceLoaded), this);
}

/* ************************************************************************ */

CMakePlugin::~CMakePlugin()
{
    // Nothing to do
}

/* ************************************************************************ */

wxString
CMakePlugin::GetWorkspaceDirectory() const
{
    const Workspace* workspace = m_mgr->GetWorkspace();
    wxASSERT(workspace);

    return workspace->GetWorkspaceFileName().GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME);
}

/* ************************************************************************ */

wxString
CMakePlugin::GetProjectDirectory(const wxString& projectName) const
{
    const Workspace* workspace = m_mgr->GetWorkspace();
    wxASSERT(workspace);

    wxString errMsg;
    const ProjectPtr proj = workspace->FindProjectByName(projectName, errMsg);
    wxASSERT(proj);

    return proj->GetFileName().GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME);
}

/* ************************************************************************ */

wxString
CMakePlugin::GetSelectedProjectConfig() const
{
    BuildConfigPtr configPtr = GetSelectedBuildConfig();

    if (configPtr)
        return configPtr->GetName();

    return wxEmptyString;
}

/* ************************************************************************ */

BuildConfigPtr
CMakePlugin::GetSelectedBuildConfig() const
{
    const Workspace* workspace = m_mgr->GetWorkspace();
    wxASSERT(workspace);

    const ProjectPtr projectPtr = GetSelectedProject();
    wxASSERT(projectPtr);

    return workspace->GetProjBuildConf(projectPtr->GetName(), wxEmptyString);
}

/* ************************************************************************ */

const CMakeProjectSettings*
CMakePlugin::GetSelectedProjectSettings() const
{
    const ProjectPtr projectPtr = GetSelectedProject();
    wxASSERT(projectPtr);

    const wxString project = projectPtr->GetName();
    const wxString config = GetSelectedProjectConfig();

    wxASSERT(m_settingsManager);
    return m_settingsManager->GetProjectSettings(project, config);
}

/* ************************************************************************ */

bool
CMakePlugin::IsSeletedProjectEnabled() const
{
    const CMakeProjectSettings* settings = GetSelectedProjectSettings();

    return settings && settings->enabled;
}

/* ************************************************************************ */

clToolBar*
CMakePlugin::CreateToolBar(wxWindow* parent)
{
    wxUnusedVar(parent);
    return NULL;
}

/* ************************************************************************ */

void
CMakePlugin::CreatePluginMenu(wxMenu* pluginsMenu)
{
    wxMenu* menu = new wxMenu();
    menu->Append(new wxMenuItem(menu, XRCID("cmake_settings"), _("Settings...")));
    menu->AppendSeparator();
    menu->Append(new wxMenuItem(menu, XRCID("cmake_help"), _("Help...")));

    pluginsMenu->Append(wxID_ANY, "CMake", menu);

    wxTheApp->Bind(wxEVT_COMMAND_MENU_SELECTED, &CMakePlugin::OnSettings, this, XRCID("cmake_settings"));
    wxTheApp->Bind(wxEVT_COMMAND_MENU_SELECTED, &CMakePlugin::OnHelp, this, XRCID("cmake_help"));
}

/* ************************************************************************ */

void
CMakePlugin::HookPopupMenu(wxMenu* menu, MenuType type)
{
    // Add menu to project menu
    if (type == MenuTypeFileView_Project) {
        if (!menu->FindItem(XRCID("cmake_project_menu"))) {
            menu->PrependSeparator();
            menu->Prepend(XRCID("cmake_project_menu"), _("CMake"), new CMakeProjectMenu(this));
        }
    } else if (type == MenuTypeFileView_Workspace) {
        if (!menu->FindItem(XRCID("cmake_workspace_menu"))) {
            menu->PrependSeparator();
            menu->Prepend(XRCID("cmake_workspace_menu"), _("CMake"), new CMakeWorkspaceMenu(this));
        }
    }
}

/* ************************************************************************ */

void
CMakePlugin::HookProjectSettingsTab(wxBookCtrlBase* notebook,
                                    const wxString& projectName,
                                    const wxString& configName)
{
    wxASSERT(notebook);

    /*
     * Let's save some memory and use only one panel for all
     * configurations.
     */

    if (!m_panel) {
        wxASSERT(m_mgr);
        wxASSERT(m_mgr->GetWorkspace());

        // Create panel
        m_panel = new CMakeProjectSettingsPanel(notebook, this);

        // Add panel to the notebook
        notebook->AddPage(m_panel, wxT("CMake"), true);
    }

    wxASSERT(m_panel);
    // Check panel owner
    wxASSERT(notebook == m_panel->GetParent());

    // It's not exactly good idea to load config everytime
    m_settingsManager->LoadProject(projectName);

    // Find settings or create new one
    m_panel->SetSettings(
        m_settingsManager->GetProjectSettings(projectName, configName, true),
        projectName,
        configName
    );
}

/* ************************************************************************ */

void
CMakePlugin::UnHookProjectSettingsTab(wxBookCtrlBase* notebook,
                                      const wxString& projectName,
                                      const wxString& configName)
{
    // The configName is empty :(
    wxASSERT(notebook);

    wxUnusedVar(projectName);
    wxUnusedVar(configName);

    // Try to find panel
    int pos = notebook->FindPage(m_panel);

    if (pos != wxNOT_FOUND) {
        // Remove and destroy
        notebook->RemovePage(pos);
        m_panel->Destroy();
        m_panel = NULL;
    }
}

/* ************************************************************************ */

void
CMakePlugin::UnPlug()
{
    // Unbind events
    wxTheApp->Unbind(wxEVT_COMMAND_MENU_SELECTED, &CMakePlugin::OnSettings, this, XRCID("cmake_settings"));
    wxTheApp->Unbind(wxEVT_COMMAND_MENU_SELECTED, &CMakePlugin::OnHelp, this, XRCID("cmake_help"));

    EventNotifier::Get()->Unbind(wxEVT_CMD_PROJ_SETTINGS_SAVED, wxCommandEventHandler(CMakePlugin::OnSaveConfig), this);
    EventNotifier::Get()->Unbind(wxEVT_BUILD_STARTING, clBuildEventHandler(CMakePlugin::OnBuildStarting), this);
    EventNotifier::Get()->Unbind(wxEVT_GET_PROJECT_BUILD_CMD, clBuildEventHandler(CMakePlugin::OnGetBuildCommand), this);
    EventNotifier::Get()->Unbind(wxEVT_GET_PROJECT_CLEAN_CMD, clBuildEventHandler(CMakePlugin::OnGetCleanCommand), this);
    EventNotifier::Get()->Unbind(wxEVT_GET_IS_PLUGIN_MAKEFILE, clBuildEventHandler(CMakePlugin::OnGetIsPluginMakefile), this);
    EventNotifier::Get()->Unbind(wxEVT_PLUGIN_EXPORT_MAKEFILE, clBuildEventHandler(CMakePlugin::OnExportMakefile), this);
    EventNotifier::Get()->Unbind(wxEVT_WORKSPACE_LOADED, wxCommandEventHandler(CMakePlugin::OnWorkspaceLoaded), this);
}

/* ************************************************************************ */

bool
CMakePlugin::ExistsCMakeLists(const wxString& directory) const
{
    return wxFileExists(wxFileName(directory, CMAKELISTS_FILE).GetFullPath());
}

/* ************************************************************************ */

void
CMakePlugin::OpenCMakeLists(const wxString& directory) const
{
    if (!m_mgr->OpenFile(wxFileName(directory, CMAKELISTS_FILE).GetFullPath()))
        wxMessageBox("Unable to open " + CMAKELISTS_FILE, wxMessageBoxCaptionStr, wxOK | wxCENTER | wxICON_ERROR);
}

/* ************************************************************************ */

void
CMakePlugin::OnSettings(wxCommandEvent& event)
{
    CMakeSettingsDialog dlg(NULL, GetCMake());

    // Set original value
    dlg.SetCMakePath(m_configuration->GetProgramPath());

    // Store change
    if (dlg.ShowModal() == wxID_OK) {
        m_configuration->SetProgramPath(dlg.GetCMakePath());
    }
}

/* ************************************************************************ */

void
CMakePlugin::OnHelp(wxCommandEvent& event)
{
    wxASSERT(m_cmake);

    if (!m_cmake->IsOk()) {
        wxMessageBox(_("CMake program not found! Please check if cmake path is set properly."),
                     wxMessageBoxCaptionStr, wxOK | wxCENTER | wxICON_ERROR);
        return;
    }

    if (m_cmake->IsDirty())
        m_cmake->LoadData();

    CMakeHelpDialog(NULL, m_cmake.get()).ShowModal();
}

/* ************************************************************************ */

void
CMakePlugin::OnSaveConfig(wxCommandEvent& event)
{
    event.Skip();

    CL_DEBUG("Saving CMake config...");

    // Panel doesn't exists
    if (!m_panel)
        return;

    // Get project name
    const wxString* proj = reinterpret_cast<wxString*>(event.GetClientData());
    wxASSERT(proj);

    const wxString project = *proj;

    // Store settings into pointer
    m_panel->StoreSettings();

    wxASSERT(m_settingsManager);
    // Save settings
    m_settingsManager->SaveProject(project);
}

/* ************************************************************************ */

void
CMakePlugin::OnBuildStarting(clBuildEvent& event)
{
    // call Skip() to allow the standard compilation to take place
    event.Skip();

    wxString project = event.GetProjectName();
    const wxString config  = event.GetConfigurationName();

    // Get settings
    const CMakeProjectSettings* settings = GetSettingsManager()->GetProjectSettings(project, config);

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled)
        return;

    // Perform configuration
}

/* ************************************************************************ */

void
CMakePlugin::OnGetCleanCommand(clBuildEvent& event)
{
    ProcessBuildEvent(event, "clean");
}

/* ************************************************************************ */

void
CMakePlugin::OnGetBuildCommand(clBuildEvent& event)
{
    ProcessBuildEvent(event);
}

/* ************************************************************************ */

void
CMakePlugin::OnGetIsPluginMakefile(clBuildEvent& event)
{
    wxString project = event.GetProjectName();
    const wxString config  = event.GetConfigurationName();

    // Get settings
    const CMakeProjectSettings* settings = GetSettingsManager()->GetProjectSettings(project, config);

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled) {
        // Cannot provide custom makefile
        event.Skip();
        return;
    }

    // Custom makefile is provided
}

/* ************************************************************************ */

void
CMakePlugin::OnExportMakefile(clBuildEvent& event)
{
    wxString project = event.GetProjectName();
    const wxString config  = event.GetConfigurationName();

    // Get settings
    const CMakeProjectSettings* settings = GetSettingsManager()->GetProjectSettings(project, config);

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled) {
        // Unable to export makefile
        event.Skip();
        return;
    }

    // Parent project is set
    if (!settings->parentProject.IsEmpty()) {
        // Configure parent project instead
        project = settings->parentProject;
        settings = GetSettingsManager()->GetProjectSettings(project, config);
    }

    // Macro expander
    // FIXME use IMacroManager (unable to find it yet)
    MacroManager* macro = MacroManager::Instance();
    wxASSERT(macro);

    // Get CMAKE
    const wxString cmake = GetConfiguration()->GetProgramPath();
    const wxString sourceDir = settings->sourceDirectory;
    const wxString buildDir = settings->buildDirectory;
    wxArrayString args;

    // Create command and Expand macros
    const wxString cmd = macro->Expand(
        CMakeBuilder::CreateConfigureCmd(cmake, *settings),
        GetManager(), project, config
    );

    // Create build directory if missing
    if (!wxDir::Exists(buildDir))
        wxDir::Make(buildDir);

    {
        DirSaver dir;

        // TODO use wxFileName
        wxSetWorkingDirectory(wxGetCwd() + "/" + buildDir);

        // Configure
        // FIXME where goes the output?
        // TODO how we get information about configuration failure?
        wxArrayString output;
        ProcUtils::SafeExecuteCommand(cmd, output);
    }
}

/* ************************************************************************ */

void
CMakePlugin::OnWorkspaceLoaded(wxCommandEvent& event)
{
    // Allow others to do something
    event.Skip();

    // Load everything into memory
    m_settingsManager->Load();
}

/* ************************************************************************ */

void
CMakePlugin::ProcessBuildEvent(clBuildEvent& event, const wxString& param)
{
    wxString project = event.GetProjectName();
    const wxString config  = event.GetConfigurationName();

    // Get settings
    const CMakeProjectSettings* settings = GetSettingsManager()->GetProjectSettings(project, config);

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled) {
        // Unable to export makefile
        event.Skip();
        return;
    }

    // Parent project is set
    if (!settings->parentProject.IsEmpty()) {
        if (!event.IsProjectOnly()) {
            // Empty command
            event.SetCommand("@echo \"Handled by parent project: " + settings->parentProject + "\"");
            return;
        }

        settings = GetSettingsManager()->GetProjectSettings(settings->parentProject, config);
    }

    wxString target = param;

    // Build only project
    if (event.IsProjectOnly())
        target += " " + project;

    // Macro expander
    MacroManager* macro = MacroManager::Instance();
    wxASSERT(macro);

    // Create build command
    const wxString cmd = macro->Expand(
        CMakeBuilder::CreateBuildCmd("$(MAKE)", settings->buildDirectory, target),
        GetManager(), project, config
    );

    // Clean project command
    event.SetCommand(cmd);
}

/* ************************************************************************ */
