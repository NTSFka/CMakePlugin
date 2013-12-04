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

#ifndef CMAKE_CONFIGURATION_HPP_
#define CMAKE_CONFIGURATION_HPP_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// wxWidgets
#include <wx/string.h>
#include <wx/fileconf.h>

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief Global CMake configuration.
 */
class CMakeConfiguration : public wxFileConfig
{

// Public Ctors
public:


    /**
     * @brief Constructor.
     *
     * @param confPath Configuration path.
     */
    inline explicit CMakeConfiguration(const wxString& confPath)
        : wxFileConfig(wxEmptyString, wxEmptyString, confPath,
                       wxEmptyString, wxCONFIG_USE_LOCAL_FILE)
    {}


    /**
     * @brief Destructor.
     */
    virtual ~CMakeConfiguration()
    {
        Flush();
    }


// Public Accessors
public:


    /**
     * @brief Returns CMake program path.
     *
     * @return
     */
    inline wxString GetProgramPath() const
    {
        return Read("CMakePath", "cmake");
    }


// Public Mutators
public:


    /**
     * @brief Sets program path.
     *
     * @param path
     */
    inline void SetProgramPath(const wxString& path)
    {
        Write("CMakePath", path);
    }

};

/* ************************************************************************ */

#endif // CMAKE_CONFIGURATION_HPP_
