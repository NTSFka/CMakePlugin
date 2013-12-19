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

#ifndef CMAKE_OUTPUT_H_
#define CMAKE_OUTPUT_H_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// UI
#include "CMakeOutputBase.h"

/* ************************************************************************ */
/* FORWARD DECLARATIONS                                                     */
/* ************************************************************************ */

class CMakePlugin;

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief Output tab for CMake.
 *
 * @deprecated
 */
class CMakeOutput : public CMakeOutputBase
{

// Public Ctors & Dtors
public:


    /**
     * @brief Create a CMake output tab.
     *
     * @param parent Pointer to parent window.
     * @param id
     * @param plugin
     */
    CMakeOutput(wxWindow* parent, CMakePlugin* plugin);


// Public Operations
public:


    /**
     * @brief Set output text.
     *
     * @param output Lines.
     */
    void SetOutput(const wxArrayString& output);


    /**
     * @brief Set errors text.
     *
     * @param errors Lines.
     */
    void SetErrors(const wxArrayString& errors);


    /**
     * @brief Clears the output.
     */
    void Clear();


// Public Events
public:


    /**
     * @brief On update UI.
     *
     * @param event
     */
    void OnHoldOpenUpdateUI(wxUpdateUIEvent& event);


// Private Data Members
private:


    /// CMake plugin.
    CMakePlugin* m_plugin;

};

/* ************************************************************************ */

#endif // CMAKE_OUTPUT_H_
