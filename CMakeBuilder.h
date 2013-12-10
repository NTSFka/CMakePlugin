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

#ifndef CMAKE_BUILDER_H_
#define CMAKE_BUILDER_H_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// wxWidgets
#include <wx/string.h>
#include <wx/arrstr.h>

/* ************************************************************************ */
/* FORWARD DECLARATIONS                                                     */
/* ************************************************************************ */

class CMakeProjectSettings;

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief CMake builder.
 *
 * It's an interface to everything about building workspace and project.
 */
class CMakeBuilder
{

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

};

/* ************************************************************************ */

#endif // CMAKE_BUILDER_H_
