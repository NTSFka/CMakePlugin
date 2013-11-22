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
#include "CMakePlugin.hpp"

// wxWidgets
#include <wx/app.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>
#include <wx/xrc/xmlres.h>
#include <wx/mimetype.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/dir.h>
#include <wx/regex.h>

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
#include "CMake.hpp"
#include "CMakeGenerator.hpp"
#include "CMakeWorkspaceMenu.hpp"
#include "CMakeProjectMenu.hpp"
#include "CMakeBuilder.hpp"
#include "CMakeHelpDialog.hpp"
#include "CMakeSettingsDialog.hpp"
#include "CMakeSettingsManager.hpp"
#include "CMakeProjectSettings.hpp"
#include "CMakeProjectSettingsPanel.hpp"
#include "CMakeOutput.hpp"
#include "CMakeGenerator.hpp"

/* ************************************************************************ */
/* VARIABLES                                                                */
/* ************************************************************************ */

static CMakePlugin* g_plugin = nullptr;

/* ************************************************************************ */

const wxString CMakePlugin::CMAKELISTS_FILE = wxT("CMakeLists.txt");

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
    if (g_plugin == nullptr)
    {
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
    info.SetVersion("0.1");

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

CMakePlugin::CMakePlugin(IManager* manager) noexcept
    : IPlugin(manager)
    , m_configuration(nullptr)
    , m_cmake(nullptr)
    , m_builder(new CMakeBuilder(this))
    , m_settingsManager(new CMakeSettingsManager(this))
    , m_generator(new CMakeGenerator())
    , m_panel(nullptr)
    , m_output(nullptr)
{
    m_longName = _("CMake integration with CodeLite");
    m_shortName = "CMakePlugin";

    // Create CMake configuration file
    m_configuration.reset(new CMakeConfiguration(wxStandardPaths::Get().
        GetUserDataDir() + wxFileName::GetPathSeparator() + "config/cmake.ini")
    );

    // Create cmake application
    m_cmake.reset(new CMake(m_configuration->GetProgramPath()));

    // Add custom tab
    Notebook* notebook = m_mgr->GetOutputPaneNotebook();
    m_output = new CMakeOutput(notebook, wxID_ANY, this);
    notebook->AddPage(m_output, "CMake", false, LoadBitmapFile("cmake.png"));

    // Bind events
    EventNotifier::Get()->Bind(wxEVT_CMD_PROJ_SETTINGS_SAVED, wxCommandEventHandler(CMakePlugin::OnSaveConfig), this);
    //EventNotifier::Get()->Bind(wxEVT_BUILD_STARTING, wxCommandEventHandler(CMakePlugin::OnBuildStarting), this);
    //EventNotifier::Get()->Bind(wxEVT_GET_PROJECT_BUILD_CMD, wxCommandEventHandler(CMakePlugin::OnGetBuildCommand), this);
    //EventNotifier::Get()->Bind(wxEVT_GET_PROJECT_CLEAN_CMD, wxCommandEventHandler(CMakePlugin::OnGetCleanCommand), this);
    //EventNotifier::Get()->Bind(wxEVT_GET_IS_PLUGIN_MAKEFILE, wxCommandEventHandler(CMakePlugin::OnGetIsPluginMakefile), this);
    //EventNotifier::Get()->Bind(wxEVT_PLUGIN_EXPORT_MAKEFILE, wxCommandEventHandler(CMakePlugin::OnExportMakefile), this);
    EventNotifier::Get()->Bind(wxEVT_WORKSPACE_LOADED, wxCommandEventHandler(CMakePlugin::OnWorkspaceLoaded), this);
}

/* ************************************************************************ */

CMakePlugin::~CMakePlugin() noexcept
{
    // Nothing to do
}

/* ************************************************************************ */

wxString
CMakePlugin::GetWorkspaceDirectory() const noexcept
{
    return m_mgr->GetWorkspace()->GetWorkspaceFileName().GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME);
}

/* ************************************************************************ */

wxString
CMakePlugin::GetProjectDirectory(const wxString& projectName) const noexcept
{
    wxString errMsg;
    ProjectPtr proj = m_mgr->GetWorkspace()->FindProjectByName(projectName, errMsg);

    return proj->GetFileName().GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME);
}

/* ************************************************************************ */

wxString
CMakePlugin::GetSelectedProjectConfig() const noexcept
{
    BuildConfigPtr configPtr = GetSelectedBuildConfig();

    if (configPtr)
        return configPtr->GetName();

    return wxEmptyString;
}

/* ************************************************************************ */

BuildConfigPtr
CMakePlugin::GetSelectedBuildConfig() const noexcept
{
    ProjectPtr projectPtr = GetSelectedProject();
    return m_mgr->GetWorkspace()->GetProjBuildConf(projectPtr->GetName(), wxEmptyString);
}

/* ************************************************************************ */

const CMakeProjectSettings*
CMakePlugin::GetSelectedProjectSettings() const noexcept
{
    ProjectPtr projectPtr = GetSelectedProject();
    wxASSERT(projectPtr);

    wxString project = projectPtr->GetName();
    wxString config = GetSelectedProjectConfig();

    return m_settingsManager->GetProjectSettings(project, config);
}

/* ************************************************************************ */

bool
CMakePlugin::IsSeletedProjectEnabled() const noexcept
{
    const CMakeProjectSettings* settings = GetSelectedProjectSettings();

    return settings && settings->enabled;
}

/* ************************************************************************ */

clToolBar*
CMakePlugin::CreateToolBar(wxWindow* parent)
{
    wxUnusedVar(parent);
    return nullptr;
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
    if (type == MenuTypeFileView_Project)
    {
        if (!menu->FindItem(XRCID("cmake_project_menu")))
        {
            menu->PrependSeparator();
            menu->Prepend(XRCID("cmake_project_menu"), _("CMake"), new CMakeProjectMenu(this));
        }
    }
    else if (type == MenuTypeFileView_Workspace)
    {
        if (!menu->FindItem(XRCID("cmake_workspace_menu")))
        {
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

    if (!m_panel)
    {
        wxASSERT(m_mgr);
        wxASSERT(m_mgr->GetWorkspace());

        wxString err;
        ProjectPtr project = m_mgr->GetWorkspace()->FindProjectByName(projectName, err);
        wxASSERT(project);

        // Create panel
        m_panel = new CMakeProjectSettingsPanel(notebook, project, this);

        // Add panel to the notebook
        notebook->AddPage(m_panel, wxT("CMake"), true);
    }

    wxASSERT(m_panel);
    // Check panel owner
    wxASSERT(notebook == m_panel->GetParent());

    // It's not exactly good idea to load config everytime
    m_settingsManager->LoadProject(projectName);

    // Find settings or create new one
    m_panel->SetSettings(m_settingsManager->GetProjectSettings(projectName, configName, true));
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

    if (pos != wxNOT_FOUND)
    {
        // Remove and destroy
        notebook->RemovePage(pos);
        m_panel->Destroy();
        m_panel = nullptr;
    }
}

/* ************************************************************************ */

void
CMakePlugin::UnPlug()
{
    // Remove output tab
    Notebook* notebook = m_mgr->GetOutputPaneNotebook();
    int pos = notebook->FindPage(m_output);
    if (pos != wxNOT_FOUND)
        notebook->DeletePage(pos);

    // Unbind events
    wxTheApp->Unbind(wxEVT_COMMAND_MENU_SELECTED, &CMakePlugin::OnSettings, this, XRCID("cmake_settings"));
    wxTheApp->Unbind(wxEVT_COMMAND_MENU_SELECTED, &CMakePlugin::OnHelp, this, XRCID("cmake_help"));

    EventNotifier::Get()->Unbind(wxEVT_CMD_PROJ_SETTINGS_SAVED, wxCommandEventHandler(CMakePlugin::OnSaveConfig), this);
    //EventNotifier::Get()->Unbind(wxEVT_BUILD_STARTING, wxCommandEventHandler(CMakePlugin::OnBuildStarting), this);
    //EventNotifier::Get()->Unbind(wxEVT_GET_PROJECT_BUILD_CMD, wxCommandEventHandler(CMakePlugin::OnGetBuildCommand), this);
    //EventNotifier::Get()->Unbind(wxEVT_GET_PROJECT_CLEAN_CMD, wxCommandEventHandler(CMakePlugin::OnGetCleanCommand), this);
    //EventNotifier::Get()->Unbind(wxEVT_GET_IS_PLUGIN_MAKEFILE, wxCommandEventHandler(CMakePlugin::OnGetIsPluginMakefile), this);
    //EventNotifier::Get()->Unbind(wxEVT_PLUGIN_EXPORT_MAKEFILE, wxCommandEventHandler(CMakePlugin::OnExportMakefile), this);
    EventNotifier::Get()->Unbind(wxEVT_WORKSPACE_LOADED, wxCommandEventHandler(CMakePlugin::OnWorkspaceLoaded), this);
}

/* ************************************************************************ */

bool
CMakePlugin::ExistsCMakeLists(const wxString& directory) const noexcept
{
    return wxFileExists(wxFileName(directory, CMAKELISTS_FILE).GetFullPath());
}

/* ************************************************************************ */

void
CMakePlugin::OpenCMakeLists(const wxString& directory) const noexcept
{
    if (!m_mgr->OpenFile(wxFileName(directory, CMAKELISTS_FILE).GetFullPath()))
        wxMessageBox("Unable to open " + CMAKELISTS_FILE, wxMessageBoxCaptionStr, wxOK | wxCENTER | wxICON_ERROR);
}

/* ************************************************************************ */

void
CMakePlugin::OnSettings(wxCommandEvent& event) noexcept
{
    CMakeSettingsDialog dlg(nullptr, GetCMake());

    // Set original value
    dlg.SetCMakePath(m_configuration->GetProgramPath());

    // Store change
    if (dlg.ShowModal() == wxID_OK)
    {
        m_configuration->SetProgramPath(dlg.GetCMakePath());
    }
}

/* ************************************************************************ */

void
CMakePlugin::OnHelp(wxCommandEvent& event) noexcept
{
    wxASSERT(m_cmake);

    if (!m_cmake->IsOk())
    {
        wxMessageBox(_("CMake program not found! Please check if cmake path is set properly."),
                     wxMessageBoxCaptionStr, wxOK | wxCENTER | wxICON_ERROR);
        return;
    }

    if (m_cmake->IsDirty())
        m_cmake->LoadData();

    CMakeHelpDialog(nullptr, m_cmake.get()).ShowModal();
}

/* ************************************************************************ */

void
CMakePlugin::OnSaveConfig(wxCommandEvent& event) noexcept
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
CMakePlugin::OnBuildStarting(wxCommandEvent& event) noexcept
{
    // call Skip() to allow the standard compilation to take place
    event.Skip();

    // Get settings
    const CMakeProjectSettings* settings = GetSettings(event);

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled)
        return;

    // TODO check if buildDir exists
}

/* ************************************************************************ */

void
CMakePlugin::OnGetCleanCommand(wxCommandEvent& event) noexcept
{
    // Get settings
    const CMakeProjectSettings* settings = GetSettings(event);

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled)
    {
        // Cannot provide clean command
        event.Skip();
        return;
    }

    // Set clean command
    // cd $(BuildDir) && $(MAKE) -j8 clean

    const wxString* cd = reinterpret_cast<wxString*>(event.GetClientData());
    wxASSERT(cd);
    const wxString project = *cd;
    const wxString config  = event.GetString();

    // Macro expander
    MacroManager* macro = MacroManager::Instance();
    wxASSERT(macro);

    wxASSERT(m_mgr);
    Workspace* workspace = m_mgr->GetWorkspace();
    wxASSERT(workspace);
    BuildConfigPtr bldConf = workspace->GetProjBuildConf(project, config);
    wxASSERT(bldConf);
    CompilerPtr compiler = bldConf->GetCompiler();
    wxASSERT(compiler);

    // Arguments
    const wxString buildDir = macro->Expand(settings->buildDirectory, m_mgr, project, config);
    const wxString make = compiler->GetTool("MAKE");

    // Clean project command
    event.SetString(CMakeBuilder::CreateBuildCmd(make, buildDir, "clean"));
}

/* ************************************************************************ */

void
CMakePlugin::OnGetBuildCommand(wxCommandEvent& event) noexcept
{
    // Get settings
    const CMakeProjectSettings* settings = GetSettings(event);

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled)
    {
        // Cannot provide build command
        event.Skip();
        return;
    }

    // TODO set build command
    // cd $(BuildDir) && $(MAKE) clean

    const wxString* cd = reinterpret_cast<wxString*>(event.GetClientData());
    wxASSERT(cd);
    const wxString project = *cd;
    const wxString config  = event.GetString();

    // Macro expander
    MacroManager* macro = MacroManager::Instance();
    wxASSERT(macro);

    wxASSERT(m_mgr);
    Workspace* workspace = m_mgr->GetWorkspace();
    wxASSERT(workspace);
    BuildConfigPtr bldConf = workspace->GetProjBuildConf(project, config);
    wxASSERT(bldConf);
    CompilerPtr compiler = bldConf->GetCompiler();
    wxASSERT(compiler);

    // Arguments
    const wxString buildDir = macro->Expand(settings->buildDirectory, m_mgr, project, config);
    const wxString make = compiler->GetTool("MAKE");

    // Clean project command
    event.SetString(CMakeBuilder::CreateBuildCmd(make, buildDir));
}

/* ************************************************************************ */

void
CMakePlugin::OnGetIsPluginMakefile(wxCommandEvent& event) noexcept
{
    // Get settings
    const CMakeProjectSettings* settings = GetSettings(event);

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled)
    {
        // Cannot provide custom makefile
        event.Skip();
        return;
    }

    // Custom makefile is provided
}

/* ************************************************************************ */

void
CMakePlugin::OnExportMakefile(wxCommandEvent& event) noexcept
{
    // Get settings
    const CMakeProjectSettings* settings = GetSettings(event);

    // Doesn't exists or not enabled
    if (!settings || !settings->enabled)
    {
        // Unable to export makefile
        event.Skip();
        return;
    }

    const wxString* cd = reinterpret_cast<wxString*>(event.GetClientData());
    wxASSERT(cd);
    const wxString project = *cd;
    const wxString config  = event.GetString();

    // Macro expander
    MacroManager* macro = MacroManager::Instance();
    wxASSERT(macro);

    // Generate makefiles
    // cd $(BuildDir) && $(CMAKE) $(Arguments) $(SourceDir)

    // Get CMAKE
    const wxString cmake = GetConfiguration()->GetProgramPath();
    const wxString sourceDir = macro->Expand(settings->sourceDirectory, m_mgr, project, config);
    const wxString buildDir = macro->Expand(settings->buildDirectory, m_mgr, project, config);
    wxArrayString args;

    // Generator
    if (!settings->generator.IsEmpty())
        args.Add("-G " + settings->generator);

    // Build Type
    if (!settings->buildType.IsEmpty())
        args.Add("-DCMAKE_BUILD_TYPE=" + settings->buildType);

    // Copy additional arguments
    for (wxString str : settings->arguments)
        args.Add(macro->Expand(str, m_mgr, project, config));

    // Configure project
    event.SetString(CMakeBuilder::CreateConfigureCmd(cmake, sourceDir, buildDir, args));
}

/* ************************************************************************ */

void
CMakePlugin::OnWorkspaceLoaded(wxCommandEvent& event) noexcept
{
    // Allow others to do something
    event.Skip();

    // Load everything into memory
    m_settingsManager->Load();
}

/* ************************************************************************ */

const CMakeProjectSettings*
CMakePlugin::GetSettings(wxCommandEvent& event) noexcept
{
    // Get project name
    const wxString* proj = reinterpret_cast<wxString*>(event.GetClientData());
    wxASSERT(proj);

    const wxString project = *proj;
    const wxString config = event.GetString();

    const CMakeSettingsManager* mgr = m_settingsManager.get();
    wxASSERT(mgr);

    // Get project settings
    return mgr->GetProjectSettings(project, config);
}

/* ************************************************************************ */
