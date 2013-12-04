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
#include "CMakeSettingsDialog.h"

// Codelite
#include "windowattrmanager.h"

// CMakePlugin
#include "CMake.hpp"
#include "CMakeHelpDialog.h"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeSettingsDialog::CMakeSettingsDialog(wxWindow* parent, CMake* cmake)
    : CMakeSettingsDialogBase(parent)
    , m_cmake(cmake)
{
    // Load window layout
    WindowAttrManager::Load(this, "CMakeSettingsDialog", NULL);
}

/* ************************************************************************ */

CMakeSettingsDialog::~CMakeSettingsDialog()
{
    // Save window layout
    WindowAttrManager::Save(this, "CMakeSettingsDialog", NULL);
}

/* ************************************************************************ */

void
CMakeSettingsDialog::OnShowHelp(wxCommandEvent& event)
{
    wxASSERT(m_cmake);

    if (!m_cmake->IsOk())
    {
        wxMessageBox(_("CMake program not found!"), wxMessageBoxCaptionStr, wxOK | wxCENTER | wxICON_ERROR);
        return;
    }

    if (m_cmake->IsDirty())
        m_cmake->LoadData();

    CMakeHelpDialog(NULL, m_cmake).ShowModal();
}

/* ************************************************************************ */
