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

#ifndef CMAKE_HPP_
#define CMAKE_HPP_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// wxWidgets
#include <wx/string.h>
#include <wx/filename.h>
#include <wx/arrstr.h>

// Codelite
#include "macros.h"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief Class representing the cmake application.
 *
 * Class can be used for extracting help data from current installed
 * version of CMake like supported commands, modules, etc.
 *
 * @todo Use Unix Man output from CMake to get help.
 */
class CMake
{

// Public Ctors
public:


    /**
     * @brief Constructor.
     *
     * @param path Path to cmake application.
     */
    explicit CMake(const wxFileName& path = wxFileName());


// Public Accessors
public:


    /**
     * @brief Returns a list of CMake versions.
     *
     * The list is hard-written and must be manually updated.
     *
     * @return
     */
    static wxArrayString GetVersions();


    /**
     * @brief Returns path to the CMake application.
     *
     * Default value is 'cmake'.
     *
     * @return
     */
    inline const wxFileName& GetPath() const {
        return m_path;
    }


    /**
     * @brief Check if CMake path is OK.
     *
     * @return
     */
    bool IsOk() const;


    /**
     * @brief Returns if data is dirty.
     *
     * @return
     */
    inline bool IsDirty() const {
        return m_dirty;
    }


    /**
     * @brief Returns CMake version.
     *
     * @return
     */
    inline const wxString& GetVersion() const {
        return m_version;
    }


    /**
     * @brief Returns a list of available commands.
     *
     * @return
     */
    inline const wxStringMap_t& GetCommands() const {
        return m_commands;
    }


    /**
     * @brief Returns a list of available modules.
     *
     * @return
     */
    inline const wxStringMap_t& GetModules() const {
        return m_modules;
    }


    /**
     * @brief Returns a list of available properties.
     *
     * @return
     */
    inline const wxStringMap_t& GetProperties() const {
        return m_properties;
    }


    /**
     * @brief Returns a list of available variables.
     *
     * @return
     */
    inline const wxStringMap_t& GetVariables() const {
        return m_variables;
    }


    /**
     * @brief Returns CMake copyright.
     *
     * @return
     */
    inline const wxString& GetCopyright() const {
        return m_copyright;
    }


    /**
     * @brief Returns CMake generators.
     *
     * @return
     */
    inline const wxArrayString& GetGenerators() const {
        return m_generators;
    }


// Public Mutators
public:


    /**
     * @brief Changes cmake application path.
     *
     * @param path
     */
    void SetPath(const wxFileName& path) {
        m_path = path;
        m_dirty = true;
    }


// Public Operations
public:


    /**
     * @brief Loads data from CMake application.
     */
    void LoadData();


// Private Operations
private:


    /**
     * @brief Parses CMake man page and fill internal structures
     * with available data.
     */
    void ParseCMakeManPage();


// Private Data Members
private:


    /// If data is dirty and need to be reloaded.
    bool m_dirty;

    /// CMake application path.
    wxFileName m_path;

    /// Cached CMake version.
    wxString m_version;

    /// List of commands.
    wxStringMap_t m_commands;

    /// List of modules.
    wxStringMap_t m_modules;

    /// List of properties.
    wxStringMap_t m_properties;

    /// List of variables.
    wxStringMap_t m_variables;

    /// CMake copyright.
    wxString m_copyright;

    /// List of generators.
    wxArrayString m_generators;

};

/* ************************************************************************ */

#endif // CMAKE_BUILDER_HPP_
