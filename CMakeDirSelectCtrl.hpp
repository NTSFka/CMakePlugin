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

#ifndef CMAKE_DIR_SELECT_CTRL_HPP_
#define CMAKE_DIR_SELECT_CTRL_HPP_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// wxWidgets
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dirdlg.h>

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief Control for selecting directory.
 */
class CMakeDirSelectCtrl : public wxPanel
{

// Public Ctors & Dtors
public:


    /**
     * @brief Create a control.
     *
     * @param parent A pointer to parent widget.
     */
    explicit CMakeDirSelectCtrl(wxWindow* parent)
        : wxPanel(parent)
    {
        SetSizer(new wxBoxSizer(wxHORIZONTAL));

        // Build directory
        m_textCtrlPath = new wxTextCtrl(this, wxID_ANY);
        GetSizer()->Add(m_textCtrlPath, wxSizerFlags(1).Expand().Border(wxRIGHT));

        m_button = new wxButton(this, wxID_ANY, "...");
        m_button->SetSizeHints(25, -1);
        GetSizer()->Add(m_button, wxSizerFlags().Expand());

        // Bind events
        Bind(wxEVT_BUTTON, &CMakeDirSelectCtrl::OnDirectorySelect, this);
    }


// Public Accessors
public:


    /**
     * @brief Returns selected directory.
     *
     * @return
     */
    wxString GetDirectory() const
    {
        return m_textCtrlPath->GetValue();
    }


    /**
     * @brief Returns root directory.
     *
     * @return
     */
    wxString GetRootDirectory() const
    {
        return m_rootDirectory;
    }


// Public Mutators
public:


    /**
     * @brief Sets directory.
     *
     * @param dir
     */
    void SetDirectory(const wxString& dir)
    {
        m_textCtrlPath->SetValue(dir);
    }


    /**
     * @brief Sets root directory.
     *
     * @param dir
     */
    void SetRootDirectory(const wxString& dir)
    {
        m_rootDirectory = dir;
    }


// Public Events
public:


    /**
     * @brief On CMakeLists.txt open request.
     *
     * @param event
     */
    void OnDirectorySelect(wxCommandEvent& event)
    {
        wxUnusedVar(event);

        // Create path
        wxFileName path(GetDirectory());
        path.MakeAbsolute(GetRootDirectory());

        // Directory doesn't exists, use workspace directory
        if (!path.DirExists())
            path.Assign(GetRootDirectory());

        // Create dialog with current path
        wxDirDialog dlg(this);
        dlg.SetPath(path.GetFullPath());

        // Show dialog
        if (dlg.ShowModal() == wxID_OK)
        {
            wxFileName newPath(dlg.GetPath());

            if (newPath.GetFullPath() != GetRootDirectory())
                newPath.MakeRelativeTo(GetRootDirectory());

            SetDirectory(newPath.GetFullPath());
        }
    }


// Private Data Members
private:


    /// Root directory.
    wxString m_rootDirectory;

    /// Control with result path.
    wxTextCtrl* m_textCtrlPath;

    /// Dialog button
    wxButton* m_button;

};

/* ************************************************************************ */

#endif // CMAKE_DIR_SELECT_CTRL_HPP_
