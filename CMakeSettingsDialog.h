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

#ifndef CMAKE_SETTINGS_HPP_
#define CMAKE_SETTINGS_HPP_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// wxWidgets
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/button.h>

// UI
#include "CMakePluginUi.h"

/* ************************************************************************ */
/* FORWARD DECLARATIONS                                                     */
/* ************************************************************************ */

class CMake;

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief Dialog for global CMake settings.
 */
class CMakeSettingsDialog : public CMakeSettingsDialogBase
{

// Public Ctors & Dtors
public:


    /**
     * @brief Create a CMake settings dialog
     *
     * @param parent Pointer to parent window.
     * @param cmake  CMake pointer.
     */
    explicit CMakeSettingsDialog(wxWindow* parent, CMake* cmake);


    /**
     * @brief Destructor.
     */
    virtual ~CMakeSettingsDialog();


// Public Accessors
public:


    /**
     * @brief Returns path to cmake program.
     *
     * @return
     */
    wxString GetCMakePath() const
    {
        return m_filePickerProgram->GetPath();
    }


// Public Mutators
public:


    /**
     * @brief Set path to CMake progam.
     *
     * @param path
     */
    void SetCMakePath(const wxString& path)
    {
        m_filePickerProgram->SetPath(path);
    }


// Public Events
public:


    /**
     * @brief On showing CMake help.
     *
     * @param event
     */
    void OnShowHelp(wxCommandEvent& event);


// Private Data Members
private:

    /// Pointer to CMake object.
    CMake* const m_cmake;

};

/* ************************************************************************ */

#endif // CMAKE_SETTINGS_HPP_
