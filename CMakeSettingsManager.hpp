#ifndef CMAKE_SETTINGS_MANAGER_HPP_
#define CMAKE_SETTINGS_MANAGER_HPP_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// C++
#include <map>

// wxWidgets
#include <wx/string.h>
#include <wx/scopedptr.h>

// CMakePlugin
#include "CMakeProjectSettings.hpp"

/* ************************************************************************ */
/* FORWARD DECLARATIONS                                                     */
/* ************************************************************************ */

class CMakePlugin;

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief Manager for CMake projects settings.
 *
 * Configuration is stored in JSON format:
 * [
 *   { name: "Debug", enabled: true, ...}
 *   { name: "Release", enabled: true, ...}
 * ]
 */
class CMakeSettingsManager
{

// Public Ctors
public:


    /**
     * @brief Create settings manager.
     *
     * @param plugin
     */
    explicit CMakeSettingsManager(CMakePlugin* plugin);


// Public Accessors
public:


    /**
     * @brief Returns all project configurations.
     *
     * @param project Project name.
     * @param create  If not exists, create one.
     *
     * @return A pointer to project settings for specific config or
     * nullptr if config doesn't exists.
     */
    CMakeProjectSettingsMap* GetProjectSettings(const wxString& project, bool create = false);


    /**
     * @brief Returns all project configurations.
     *
     * @param project Project name.
     *
     * @return A pointer to project settings for specific config or
     * nullptr if config doesn't exists.
     */
    const CMakeProjectSettingsMap* GetProjectSettings(const wxString& project) const;


    /**
     * @brief Return project configuration for given config.
     *
     * @param project Project name.
     * @param config  Configuration name.
     * @param create  If not exists, create one.
     *
     * @return A pointer to project settings for specific config or
     * nullptr if config doesn't exists.
     */
    CMakeProjectSettings* GetProjectSettings(const wxString& project,
        const wxString& config, bool create = false);


    /**
     * @brief Return project configuration for given config.
     *
     * @param project Project name.
     * @param config  Configuration name.
     *
     * @return A pointer to project settings for specific config or
     * nullptr if config doesn't exists.
     */
    const CMakeProjectSettings* GetProjectSettings(const wxString& project, const wxString& config) const;


    /**
     * @brief Returns if project settings exists and are use CMake.
     *
     * @param project Project name.
     * @param config  Configuration name.
     *
     * @return
     */
    bool IsProjectEnabled(const wxString& project, const wxString& config) const;


// Public Operations
public:


    /**
     * @brief Save all settings.
     */
    void Save()
    {
        SaveProjects();
    }


    /**
     * @brief Save all projects settings.
     */
    void SaveProjects();


    /**
     * @brief Save project settings.
     *
     * @param name Project name.
     */
    void SaveProject(const wxString& name);


    /**
     * @brief Load all settings.
     */
    void Load()
    {
        LoadProjects();
    }


    /**
     * @brief Load all projects settings.
     */
    void LoadProjects();


    /**
     * @brief Load project settings.
     *
     * @param name Project name.
     */
    void LoadProject(const wxString& name);


// Private Data Members
private:


    /// CMake plugin pointer.
    CMakePlugin* const m_plugin;

    /// Settings for all projects
    std::map<wxString, CMakeProjectSettingsMap> m_projectSettings;

};

/* ************************************************************************ */

#endif // CMAKE_SETTINGS_MANAGER_HPP_
