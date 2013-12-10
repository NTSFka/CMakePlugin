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

#ifndef CMAKE_OUTPUT_HPP_
#define CMAKE_OUTPUT_HPP_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// wxWidgets
#include <wx/string.h>
#include <wx/panel.h>

/* ************************************************************************ */
/* FORWARD DECLARATIONS                                                     */
/* ************************************************************************ */

class CMakePlugin;
class AsyncExeCmd;
class wxTextCtrl;

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief Output tab for CMake.
 *
 * @deprecated
 */
class CMakeOutput : public wxPanel
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
    CMakeOutput(wxWindow* parent, wxWindowID id, CMakePlugin* plugin);


// Public Operations
public:


    /**
     * @brief Appends text to output text widget.
     *
     * @param text
     */
    void AppendText(const wxString& text);


    /**
     * @brief Clears the output.
     */
    void Clear();


// Public Events
public:


    /**
     * @brief On process started.
     *
     * @param event
     */
    void OnProcStarted(wxCommandEvent& event);


    /**
     * @brief On process output.
     *
     * @param event
     */
    void OnProcOutput(wxCommandEvent& event);


    /**
     * @brief On process output error.
     *
     * @param event
     */
    void OnProcError(wxCommandEvent& event);


    /**
     * @brief On process ended.
     *
     * @param event
     */
    void OnProcEnded(wxCommandEvent& event);


    /**
     * @brief On update UI.
     *
     * @param event
     */
    void OnHoldOpenUpdateUI(wxUpdateUIEvent& event);


// Private Data Members
private:


    /// Output text widget.
    wxTextCtrl* m_output;

    /// Current command
    AsyncExeCmd* m_command;

    /// CMake plugin.
    CMakePlugin* m_plugin;

};

/* ************************************************************************ */

#endif // CMAKE_OUTPUT_HPP_
