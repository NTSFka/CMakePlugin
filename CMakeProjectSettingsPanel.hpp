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

#ifndef CMAKE_PROJECT_SETTINGS_TAB_HPP_
#define CMAKE_PROJECT_SETTINGS_TAB_HPP_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// wxWidgets
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/combobox.h>

// Codelite
#include "project.h"

// CMakePlugin
#include "CMakeProjectSettings.hpp"
#include "CMakeDirSelectCtrl.hpp"

/* ************************************************************************ */
/* FORWARD DECLARATIONS                                                     */
/* ************************************************************************ */

class CMakePlugin;

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief Tab for project settings for CMake.
 */
class CMakeProjectSettingsPanel : public wxPanel
{

// Public Ctors & Dtors
public:


    /**
     * @brief Create a CMake output tab.
     *
     * @param parent  Pointer to parent window.
     * @param project Project pointer.
     */
    explicit CMakeProjectSettingsPanel(wxWindow* parent, ProjectPtr project,
                                       CMakePlugin* plugin) noexcept;


// Public Accessors
public:


    /**
     * @brief Returns if custom CMakeLists.txt is enabled.
     *
     * @return
     */
    bool IsCMakeEnabled() const noexcept
    {
        return m_checkBoxEnable->IsChecked();
    }


    /**
     * @brief Returns source directory.
     *
     * @return
     */
    wxString GetSourceDirectory() const noexcept
    {
        return m_dirSelectCtrlSourceDirectory->GetDirectory();
    }


    /**
     * @brief Returns build directory.
     *
     * @return
     */
    wxString GetBuildDirectory() const noexcept
    {
        return m_dirSelectCtrlBuildDirectory->GetDirectory();
    }


    /**
     * @brief Returns generator name.
     *
     * @return
     */
    wxString GetGenerator() const noexcept
    {
        return m_choiceGenerator->GetStringSelection();
    }


    /**
     * @brief Returns build type.
     *
     * @return
     */
    wxString GetBuildType() const noexcept
    {
        return m_choiceBuildType->GetStringSelection();
    }


    /**
     * @brief Returns cmake arguments.
     *
     * @return
     */
    wxArrayString GetArguments() const noexcept
    {
        return wxSplit(m_textCtrlArguments->GetValue(), '\n');
    }


    /**
     * @brief Returns a pointer to project settings.
     *
     * @return
     */
    CMakeProjectSettings* GetSettings() const noexcept
    {
        return m_settings;
    }


// Public Mutators
public:


    /**
     * @brief Set if custom CMakeLists.txt is enabled.
     *
     * @param value
     */
    void SetCMakeEnabled(bool value) const noexcept
    {
        m_checkBoxEnable->SetValue(value);
    }


    /**
     * @brief Change source directory.
     *
     * @param dir Directory.
     */
    void SetSourceDirectory(const wxString& dir) noexcept
    {
        m_dirSelectCtrlSourceDirectory->SetDirectory(dir);
    }


    /**
     * @brief Change build directory.
     *
     * @param dir Directory.
     */
    void SetBuildDirectory(const wxString& dir) noexcept
    {
        m_dirSelectCtrlBuildDirectory->SetDirectory(dir);
    }


    /**
     * @brief Changes CMake generator.
     *
     * @param generator
     */
    void SetGenerator(const wxString& generator) noexcept
    {
        m_choiceGenerator->SetStringSelection(generator);
    }


    /**
     * @brief Changes CMake build type.
     *
     * @param buildType
     */
    void SetBuildType(const wxString& buildType) noexcept
    {
        m_choiceBuildType->SetStringSelection(buildType);
    }


    /**
     * @brief Set cmake arguments.
     *
     * @param arguments
     */
    void SetArguments(const wxArrayString& arguments) noexcept
    {
        m_textCtrlArguments->SetValue(wxJoin(arguments, '\n'));
    }


    /**
     * @brief Set project setting pointer.
     *
     * @param settings
     */
    void SetSettings(CMakeProjectSettings* settings) noexcept
    {
        m_settings = settings;
        LoadSettings();
    }


// Public Events
public:


    /**
     * @brief On enable or disable.
     *
     * @param event
     */
    void OnCheck(wxUpdateUIEvent& event) noexcept
    {
        event.Enable(m_checkBoxEnable->IsChecked());
    }


// Public Operations
public:


    /**
     * @brief Loads from settings pointer to GUI widgets.
     */
    void LoadSettings();


    /**
     * @brief Stores settings from GUI widgets into settings pointer.
     */
    void StoreSettings();


    /**
     * @brief Clear settings from GUI widgets into settings pointer.
     */
    void ClearSettings();


// Private Data Members
private:

    /// Project pointer.
    ProjectPtr m_project;

    /// A pointer to plugin.
    CMakePlugin* const m_plugin;

    /// Checkbox for enabling cmake support.
    wxCheckBox* m_checkBoxEnable;

    /// Static text for source directory.
    wxStaticText* m_staticTextSourceDirectory;

    /// Source directory.
    CMakeDirSelectCtrl* m_dirSelectCtrlSourceDirectory;

    /// Static text for build directory.
    wxStaticText* m_staticTextBuildDirectory;

    /// Build directory value.
    CMakeDirSelectCtrl* m_dirSelectCtrlBuildDirectory;

    /// Static text for generator.
    wxStaticText* m_staticTextGenerator;

    /// Selected generator.
    wxComboBox* m_choiceGenerator;

    /// Static text for build type.
    wxStaticText* m_staticTextBuildType;

    /// Selected build type.
    wxComboBox* m_choiceBuildType;

    /// Label for Arguments for CMake.
    wxStaticText* m_staticTextArguments;

    /// Arguments for CMake.
    wxTextCtrl* m_textCtrlArguments;

    /// Pointer to settings.
    CMakeProjectSettings* m_settings;

};

/* ************************************************************************ */

#endif //CMAKE_PROJECT_SETTINGS_TAB_HPP_
