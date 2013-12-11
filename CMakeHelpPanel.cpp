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

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// Declaration
#include "CMakeHelpPanel.h"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeHelpPanel::CMakeHelpPanel(wxWindow* parent, wxWindowID id,
                               const wxPoint& pos, const wxSize& size,
                               long style)
    : CMakeHelpPanelBase(parent, id, pos, size, style)
{
    // Nothing to do
}

/* ************************************************************************ */

CMakeHelpPanel::~CMakeHelpPanel()
{
    // Nothing to do
}

/* ************************************************************************ */

void
CMakeHelpPanel::SetData(const std::map<wxString, wxString>* data)
{
    m_data = data;

    // Remove old data
    m_listBoxList->Clear();
    m_textCtrlText->Clear();

    // Foreach data and store names into list
    for (std::map<wxString, wxString>::const_iterator it = data->begin(),
        ite = data->end(); it != ite; ++it) {
        m_listBoxList->Append(it->first);
    }
}

/* ************************************************************************ */

void
CMakeHelpPanel::OnSelect(wxCommandEvent& event)
{
    wxASSERT(m_data);

    // Get selected name
    const wxString name = m_listBoxList->GetString(event.GetInt());

    // Find name in the data
    std::map<wxString, wxString>::const_iterator it = m_data->find(name);

    // Data found
    if (it != m_data->end()) {
        // Show required data
        m_textCtrlText->SetValue(it->second);
    }
}

/* ************************************************************************ */