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
#include "CMakeOutput.hpp"

// wxWidgets
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>
#include <wx/regex.h>

// CodeLite
#include "async_executable_cmd.h"
#include "manager.h"
#include "imanager.h"
#include "frame.h"
#include "pluginmanager.h"
#include "editor_config.h"
#include "notebook_ex.h"

// CMakePlugin
#include "CMakePlugin.hpp"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeOutput::CMakeOutput(wxWindow* parent, wxWindowID id, CMakePlugin* plugin)
    : wxPanel(parent, id)
    , m_command(NULL)
    , m_plugin(plugin)
{
    SetSizer(new wxBoxSizer(wxVERTICAL));

    m_output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxTE_DONTWRAP);
    GetSizer()->Add(m_output, wxSizerFlags(1).Expand().Border());

    // Bind events
    // FIXME Codelite defined events cannot be used within Bind
    Connect(wxEVT_ASYNC_PROC_STARTED, wxCommandEventHandler(CMakeOutput::OnProcStarted), NULL, this);
    Connect(wxEVT_ASYNC_PROC_ADDLINE, wxCommandEventHandler(CMakeOutput::OnProcOutput), NULL, this);
    Connect(wxEVT_ASYNC_PROC_ADDERRLINE, wxCommandEventHandler(CMakeOutput::OnProcError), NULL, this);
    Connect(wxEVT_ASYNC_PROC_ENDED, wxCommandEventHandler(CMakeOutput::OnProcEnded), NULL, this);
    Bind(wxEVT_UPDATE_UI, &CMakeOutput::OnHoldOpenUpdateUI, this, XRCID("hold_pane_open"));

    // Default style
    m_output->SetDefaultStyle(wxTextAttr(*wxBLACK, wxNullColour, wxFont(wxFontInfo().Family(wxFONTFAMILY_TELETYPE))));
}

/* ************************************************************************ */

void
CMakeOutput::AppendText(const wxString& text)
{
    // Store default style
    static wxTextAttr defaultStyle = m_output->GetDefaultStyle();

    if (!m_output)
        return;

    // Show output
    if (m_output->IsEmpty())
    {
        // make the output pane visible
#ifndef _WIN32
        // On windows the Manager class is not exported
        ManagerST::Get()->GetPerspectiveManager().ToggleOutputPane(false);
#endif

        IManager* manager = m_plugin->GetManager();
        wxASSERT(manager);

        // Select current window
        Notebook* notebook = manager->GetOutputPaneNotebook();
        int pos = notebook->FindPage(this);
        if (pos != wxNOT_FOUND)
            notebook->SetSelection(pos);
    }

/*
 * TODO make filepath clickable
    // Match file path
    wxRegEx expression("^[^ ][a-zA-Z:]{0,2}[ a-zA-Z\\.0-9_/\\+\\-]+ *:[0-9]*(:[0-9]*)?:");
    size_t start = 0;
    size_t len = 0;

    if (expression.IsValid() && expression.Matches(text))
    {
        expression.GetMatch(&start, &len);
    }
*/
// Only on Linux (maybe on Unix)
#ifdef __linux__

    // Splitted text
    wxString textRest = text;

    // Parse text for color tags
    while (true)
    {
        // Find begin
        size_t beg = textRest.find("\033[");

        // Not found
        if (beg == wxString::npos)
            break;

        // Append text before style
        m_output->AppendText(textRest.substr(0, beg));

        // Move after begin tag
        beg += 2;

        // Find end tag
        const size_t end = textRest.find('m', beg);

        // Code between positions is color code
        const wxString formatCode = textRest.substr(beg, end - beg);

        // Get current style
        wxTextAttr style = m_output->GetDefaultStyle();

        // Split into separate codes
        const wxArrayString codes = wxSplit(formatCode, ';', 0);

        // Foreach codes
        for (wxArrayString::const_iterator it = codes.begin(), ite = codes.end(); it != ite; ++it)
        {
            /// @see http://misc.flogisoft.com/bash/tip_colors_and_formatting
            switch (wxAtoi(*it))
            {
                default: break;
                // Reset all attributes
                case 0:  style = defaultStyle; break;
                // Bold/Bright
                case 1:  style.SetFontWeight(wxFONTWEIGHT_BOLD); break;
                // Underlined
                case 4:  style.SetFontUnderlined(true); break;
                // Reset bold/bright
                case 21: style.SetFontWeight(wxFONTWEIGHT_NORMAL); break;
                // Reset underlined
                case 24: style.SetFontUnderlined(false); break;
                // Default foreground color
                case 49: style.SetBackgroundColour(wxNullColour); break;
                // Black
                case 40: style.SetBackgroundColour(*wxBLACK); break;
                // Red
                case 41: style.SetBackgroundColour(*wxRED); break;
                // Green
                case 42: style.SetBackgroundColour(*wxGREEN); break;
                // Yellow
                case 43: style.SetBackgroundColour(*wxYELLOW); break;
                // Blue
                case 44: style.SetBackgroundColour(*wxBLUE); break;
                // Magenta
                case 45: style.SetBackgroundColour(wxColour(255, 0, 255)); break;
                // Cyan
                case 46: style.SetBackgroundColour(*wxCYAN); break;
                // Light gray
                case 47: style.SetBackgroundColour(wxColour(211, 211, 211)); break;
                // Dark gray
                case 100: style.SetBackgroundColour(wxColour(169, 169, 169)); break;
                // Light red
                case 101: style.SetBackgroundColour(wxColour(254, 64, 67)); break;
                // Light green
                case 102: style.SetBackgroundColour(wxColour(144, 238, 144)); break;
                // Light yellow
                case 103: style.SetBackgroundColour(wxColour(255, 255, 224)); break;
                // Light blue
                case 104: style.SetBackgroundColour(wxColour(173, 216, 230)); break;
                // Light magenta
                case 105: style.SetBackgroundColour(wxColour(255, 119, 255)); break;
                // Light cyan
                case 106: style.SetBackgroundColour(wxColour(224, 255, 255)); break;
                // White
                case 107: style.SetBackgroundColour(*wxWHITE); break;
                // Black
                case 30: style.SetTextColour(*wxBLACK); break;
                // Red
                case 31: style.SetTextColour(*wxRED); break;
                // Green
                case 32: style.SetTextColour(*wxGREEN); break;
                // Yellow
                case 33: style.SetTextColour(*wxYELLOW); break;
                // Blue
                case 34: style.SetTextColour(*wxBLUE); break;
                // Magenta
                case 35: style.SetTextColour(wxColour(255, 0, 255)); break;
                // Cyan
                case 36: style.SetTextColour(*wxCYAN); break;
                // Light gray
                case 37: style.SetTextColour(wxColour(211, 211, 211)); break;
                // Dark gray
                case 90: style.SetTextColour(wxColour(169, 169, 169)); break;
                // Light red
                case 91: style.SetTextColour(wxColour(254, 64, 67)); break;
                // Light green
                case 92: style.SetTextColour(wxColour(144, 238, 144)); break;
                // Light yellow
                case 93: style.SetTextColour(wxColour(255, 255, 224)); break;
                // Light blue
                case 94: style.SetTextColour(wxColour(173, 216, 230)); break;
                // Light magenta
                case 95: style.SetTextColour(wxColour(255, 119, 255)); break;
                // Light cyan
                case 96: style.SetTextColour(wxColour(224, 255, 255)); break;
                // White
                case 97: style.SetTextColour(*wxWHITE); break;
            }
        }

        // Store final style
        m_output->SetDefaultStyle(style);

        // Get remaining text
        textRest = textRest.substr(end + 1);
    }

    // Append
    m_output->AppendText(textRest);

    // TODO mark file paths and make them clickable
#else

    // Append
    m_output->AppendText(text);

#endif
}

/* ************************************************************************ */

void
CMakeOutput::Clear()
{
    m_output->Clear();
}

/* ************************************************************************ */

void
CMakeOutput::OnProcStarted(wxCommandEvent& event)
{
    if ((m_command != NULL) && m_command->IsBusy())
    {
        // TODO: log message: already running a process
        return;
    }

    m_command = reinterpret_cast<AsyncExeCmd*>(event.GetEventObject());
    Clear();
    AppendText(event.GetString());
}

/* ************************************************************************ */

void
CMakeOutput::OnProcOutput(wxCommandEvent& event)
{
    AsyncExeCmd* cmd = reinterpret_cast<AsyncExeCmd*>(event.GetEventObject());

    // Different command
    if (cmd != m_command)
    {
        // TODO: log message
        return;
    }

    AppendText(event.GetString());
}

/* ************************************************************************ */

void
CMakeOutput::OnProcError(wxCommandEvent& event)
{
    AsyncExeCmd* cmd = reinterpret_cast<AsyncExeCmd*>(event.GetEventObject());

    if (cmd != m_command)
    {
        // TODO: log message
        return;
    }

    AppendText(event.GetString());
    // TODO: mark line
}

/* ************************************************************************ */

void
CMakeOutput::OnProcEnded(wxCommandEvent& event)
{
    AsyncExeCmd* cmd = reinterpret_cast<AsyncExeCmd*>(event.GetEventObject());

    if (cmd != m_command)
    {
        // TODO: log message
        return;
    }

    AppendText(event.GetString());
    m_command = NULL;
}

/* ************************************************************************ */

void
CMakeOutput::OnHoldOpenUpdateUI(wxUpdateUIEvent& event)
{
    Notebook* notebook = m_plugin->GetManager()->GetOutputPaneNotebook();

    if (notebook->GetPage(notebook->GetSelection()) != this)
        return;

    OptionsConfigPtr options = EditorConfigST::Get()->GetOptions();

    if (options->GetHideOutpuPaneOnUserClick())
    {
        event.Enable(true);
        event.Check(options->GetHideOutputPaneNotIfOutput());
    }
    else
    {
        event.Enable(false);
        event.Check(false);
    }
}

/* ************************************************************************ */
