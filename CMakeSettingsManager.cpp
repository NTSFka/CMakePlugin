
/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// Declaration
#include "CMakeSettingsManager.hpp"

// wxWidgets
#include <wx/arrstr.h>

// Codelite
#include "json_node.h"
#include "workspace.h"

// CMakePlugin
#include "CMakePlugin.hpp"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeSettingsManager::CMakeSettingsManager(CMakePlugin* plugin)
    : m_plugin(plugin)
{
    // Nothing to do
}

/* ************************************************************************ */

CMakeProjectSettingsMap*
CMakeSettingsManager::GetProjectSettings(const wxString& project, bool create) noexcept
{
    if (create)
    {
        return &(m_projectSettings[project]);
    }
    else
    {
        auto it = m_projectSettings.find(project);

        if (it == m_projectSettings.end())
            return nullptr;

        return &(it->second);
    }
}

/* ************************************************************************ */

const CMakeProjectSettingsMap*
CMakeSettingsManager::GetProjectSettings(const wxString& project) const noexcept
{
    auto it = m_projectSettings.find(project);

    if (it == m_projectSettings.end())
        return nullptr;

    return &(it->second);
}

/* ************************************************************************ */

CMakeProjectSettings*
CMakeSettingsManager::GetProjectSettings(const wxString& project, const wxString& config, bool create) noexcept
{
    // Get project settings
    CMakeProjectSettingsMap* settings = GetProjectSettings(project, create);

    if (create)
    {
        // GetProjectSettings should create the new one
        wxASSERT(settings);

        // Find or create configuration
        return &(*settings)[config];
    }
    else
    {
        // Not found
        if (!settings)
            return nullptr;

        // Find configuration
        auto it = settings->find(config);

        // Not found
        if (it == settings->end())
            return nullptr;

        return &(it->second);
    }
}

/* ************************************************************************ */

const CMakeProjectSettings*
CMakeSettingsManager::GetProjectSettings(const wxString& project, const wxString& config) const noexcept
{
    // Get project settings
    const CMakeProjectSettingsMap* settings = GetProjectSettings(project);

    // Not found
    if (!settings)
        return nullptr;

    // Find configuration
    auto it = settings->find(config);

    // Not found
    if (it == settings->end())
        return nullptr;

    return &(it->second);
}

/* ************************************************************************ */

bool
CMakeSettingsManager::IsProjectEnabled(const wxString& project, const wxString& config) const noexcept
{
    // Get project settings
    const CMakeProjectSettings* settings = GetProjectSettings(project, config);

    return settings && settings->enabled;
}

/* ************************************************************************ */

void
CMakeSettingsManager::SaveProjects()
{
    Workspace* workspace = m_plugin->GetManager()->GetWorkspace();
    wxASSERT(workspace);

    wxArrayString projects;
    workspace->GetProjectList(projects);

    for (const auto& project : projects)
    {
        SaveProject(project);
    }
}

/* ************************************************************************ */

void
CMakeSettingsManager::SaveProject(const wxString& name)
{
    Workspace* workspace = m_plugin->GetManager()->GetWorkspace();
    wxASSERT(workspace);

    wxString err;
    ProjectPtr project = workspace->FindProjectByName(name, err);

    if (!project)
        return;

    // Find project settings
    auto it = m_projectSettings.find(name);

    // Ehm...
    if (it == m_projectSettings.end())
        return;

    // Create JSON object
    JSONElement json = JSONElement::createArray("configurations");

    // Foreach settings
    for (const auto& pair : it->second)
    {
        // Get settings
        const CMakeProjectSettings& settings = pair.second;

        // Create item
        JSONElement item = JSONElement::createObject("configuration");

        // Store name
        item.addProperty("name", pair.first);

        // Store settings
        item.addProperty("enabled", settings.enabled);
        item.addProperty("buildDirectory", settings.buildDirectory);
        item.addProperty("sourceDirectory", settings.sourceDirectory);
        item.addProperty("generator", settings.generator);
        item.addProperty("buildType", settings.buildType);
        item.addProperty("arguments", settings.arguments);

        // Add array
        json.arrayAppend(item);
    }

    // Must be an array
    wxASSERT(json.getType() == cJSON_Array);

    // Store plugin data
    project->SetPluginData("CMakePlugin", json.format());
}

/* ************************************************************************ */

void
CMakeSettingsManager::LoadProjects()
{
    Workspace* workspace = m_plugin->GetManager()->GetWorkspace();
    wxASSERT(workspace);

    wxArrayString projects;
    workspace->GetProjectList(projects);

    for (const auto& project : projects)
    {
        LoadProject(project);
    }
}

/* ************************************************************************ */

void
CMakeSettingsManager::LoadProject(const wxString& name)
{
    Workspace* workspace = m_plugin->GetManager()->GetWorkspace();
    wxASSERT(workspace);

    wxString err;
    ProjectPtr project = workspace->FindProjectByName(name, err);

    if (!project)
        return;

    // Find for project or create new one
    CMakeProjectSettingsMap* projectSettings = GetProjectSettings(name, true);
    wxASSERT(projectSettings);

    // Alias for map
    CMakeProjectSettingsMap& settingsMap = *projectSettings;

    // Load JSON string
    const wxString jsonStr = project->GetPluginData("CMakePlugin");

    // Create JSON object
    JSONRoot jsonRoot(jsonStr);
    // JSONRoot cannot be temporary, because destructor deletes cJSON object.
    JSONElement json = jsonRoot.toElement();

    // Unable to parse
    if (!json.isOk())
        return;

    // Expected array with config names
    if (json.getType() != cJSON_Array)
        return;

    // Foreach array
    for (int i = 0; i < json.arraySize(); ++i)
    {
        // Get item
        const JSONElement& item = json.arrayItem(i);

        // Name
        const wxString name = item.namedObject("name").toString();

        // (Create and) get settings
        CMakeProjectSettings& settings = settingsMap[name];
        settings.enabled = item.namedObject("enabled").toBool();
        settings.buildDirectory = item.namedObject("buildDirectory").toString("build");
        settings.sourceDirectory = item.namedObject("sourceDirectory").toString("build");
        settings.generator = item.namedObject("generator").toString();
        settings.buildType = item.namedObject("buildType").toString();
        settings.arguments = item.namedObject("arguments").toArrayString();
    }
}

/* ************************************************************************ */
