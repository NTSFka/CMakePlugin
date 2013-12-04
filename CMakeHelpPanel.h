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

#ifndef CMAKE_HELP_PANEL_H_
#define CMAKE_HELP_PANEL_H_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// C++
#include <map>

// UI
#include "CMakePluginUi.h"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief Panel for help dialog where on the left side is a list of values
 * and on the right side is value description.
 */
class CMakeHelpPanel : public CMakeHelpPanelBase
{

// Public Ctors & Dtors
public:


    /**
     * @brief Constructor.
     */
    CMakeHelpPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(500,300), long style = wxTAB_TRAVERSAL);


    /**
     * @brief Destructor.
     */
    virtual ~CMakeHelpPanel();


// Public Mutators
public:


    /**
     * @brief Set panel data.
     *
     * @param data
     */
    void SetData(const std::map<wxString, wxString>* data);


// Public Events
public:


    /**
     * @brief On list item selection.
     *
     * @param event
     */
    void OnSelect(wxCommandEvent& event);


// Private Data Members
private:


    /// Panel data.
    const std::map<wxString, wxString>* m_data;

};

/* ************************************************************************ */

#endif // CMAKE_HELP_PANEL_H_
