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

#ifndef CMAKE_BUILDER_HPP_
#define CMAKE_BUILDER_HPP_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// wxWidgets
#include <wx/string.h>
#include <wx/scopedptr.h>

// Codelite
#include "async_executable_cmd.h"

/* ************************************************************************ */
/* FORWARD DECLARATIONS                                                     */
/* ************************************************************************ */

class CMakePlugin;
class CMakeProjectSettings;

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief CMake builder.
 *
 * It's an interface to everything about building workspace and project.
 *
 * @todo Get rid of AsyncExeCmd and use internal building system.
 */
class CMakeBuilder
{

// Public Ctors
public:


    /**
     * @brief Create CMake builder.
     *
     * @param plugin
     */
    explicit CMakeBuilder(CMakePlugin* plugin);



// Public Operations
public:


    /**
     * @brief Creates a command for project configuration.
     *
     * @param cmake     CMake program.
     * @param sourceDir Directory with sources and CMakeLists.txt
     * @param buildDir  Output directory.
     * @param args      Configure arguments.
     *
     * @return Result command in form: cd $buildDir && $cmake $args $sourceDir
     */
    static wxString CreateConfigureCmd(const wxString& cmake,
                                       const wxString& sourceDir,
                                       const wxString& buildDir,
                                       const wxArrayString& args);


    /**
     * @brief Creates a command for project configuration.
     *
     * @param cmake    CMake program.
     * @param settings Used settings.
     *
     * @return Result command in form: cd $buildDir && $cmake $args $sourceDir
     */
    static wxString CreateConfigureCmd(const wxString& cmake,
                                       const CMakeProjectSettings& settings);


    /**
     * @brief Creates a command for project building.
     *
     * @param make      The Make program.
     * @param buildDir  Output directory.
     * @param target    Build target.
     *
     * @return Result command in form: cd $buildDir && $make
     */
    static wxString CreateBuildCmd(const wxString& make,
                                   const wxString& buildDir,
                                   const wxString& target = wxEmptyString);


    /**
     * @brief Creates a command for project building.
     *
     * @param make      The Make program.
     * @param buildDir  Output directory.
     * @param target    Build target.
     *
     * @return Result command in form: cd $buildDir && $make
     */
    static wxString CreateBuildCmd(const wxString& make,
                                   const CMakeProjectSettings& settings,
                                   const wxString& target = wxEmptyString);


    /**
     * @brief Configure project defined by CMakeLists.txt in projectDir into
     * buildDir with configuration arguments args.
     *
     * @param cmake    CMake program.
     * @param sourceDir Directory with sources and CMakeLists.txt
     * @param buildDir  Output directory.
     * @param args      Configure arguments.
     */
    void Configure(const wxString& cmake,
                   const wxString& sourceDir,
                   const wxString& buildDir,
                   const wxArrayString& args) noexcept;


    /**
     * @brief Build project defined in build directory.
     *
     * @param make      The Make program.
     * @param buildDir  Output directory.
     * @param target    Build target.
     */
    void Build(const wxString& make, const wxString& buildDir,
               const wxString& target = wxEmptyString) noexcept;


    /**
     * @brief Run given program asynchronously.
     */
    void Run(const wxString& command) noexcept;


// Public Events
public:


    /**
     * @brief On async process end.
     *
     * @param event
     */
    void OnProcessEnd(wxProcessEvent& event) noexcept;


// Private Data Members
private:


    /// CMake plugin pointer.
    CMakePlugin* const m_plugin;

    /// CMake command.
    wxScopedPtr<AsyncExeCmd> m_asyncCmd;

};

/* ************************************************************************ */

#endif // CMAKE_BUILDER_HPP_
