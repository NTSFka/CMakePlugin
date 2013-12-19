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
#include "CMakeOutput.h"

// wxWidgets
#include <wx/xrc/xmlres.h>

// CodeLite
#include "manager.h"
#include "imanager.h"
#include "pluginmanager.h"
#include "editor_config.h"
#include "notebook_ex.h"

// CMakePlugin
#include "CMakePlugin.h"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeOutput::CMakeOutput(wxWindow* parent, CMakePlugin* plugin)
    : CMakeOutputBase(parent)
    , m_plugin(plugin)
{
    // Bind events
    Bind(wxEVT_UPDATE_UI, &CMakeOutput::OnHoldOpenUpdateUI, this, XRCID("hold_pane_open"));
}

/* ************************************************************************ */

void
CMakeOutput::SetOutput(const wxArrayString& output)
{
    m_textCtrlOut->Clear();

    // Foreach lines
    for (wxArrayString::const_iterator it = output.begin(), ite = output.end();
         it != ite; ++it) {
        m_textCtrlOut->AppendText(*it + "\n");
    }

    m_panelOut->SetFocus();
}

/* ************************************************************************ */

void
CMakeOutput::SetErrors(const wxArrayString& errors)
{
    m_textCtrlErr->Clear();

    if (!errors.IsEmpty()) {
        // Foreach lines
        for (wxArrayString::const_iterator it = errors.begin(), ite = errors.end();
             it != ite; ++it) {
            m_textCtrlErr->AppendText(*it + "\n"); // Missing EOL
        }

        m_panelErr->SetFocus();
    }
}

/* ************************************************************************ */

void
CMakeOutput::Clear()
{
    m_textCtrlOut->Clear();
    m_textCtrlErr->Clear();
}

/* ************************************************************************ */

void
CMakeOutput::OnHoldOpenUpdateUI(wxUpdateUIEvent& event)
{
    Notebook* notebook = m_plugin->GetManager()->GetOutputPaneNotebook();

    if (notebook->GetPage(notebook->GetSelection()) != this)
        return;

    OptionsConfigPtr options = EditorConfigST::Get()->GetOptions();

    if (options->GetHideOutpuPaneOnUserClick()) {
        event.Enable(true);
        event.Check(options->GetHideOutputPaneNotIfOutput());
    } else {
        event.Enable(false);
        event.Check(false);
    }
}

/* ************************************************************************ */
