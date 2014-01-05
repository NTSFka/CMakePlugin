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

#ifndef CMAKE_HELP_TAB_H_
#define CMAKE_HELP_TAB_H_

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// C++
#include <map>

// wxWidgets
#include <wx/thread.h>

// UI
#include "CMakePluginUi.h"

/* ************************************************************************ */
/* FORWARD DECLARATIONS                                                     */
/* ************************************************************************ */

class CMakePlugin;

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

/**
 * @brief Dockable window with CMake help.
 */
class CMakeHelpTab : public CMakeHelpTabBase, public wxThreadHelper
{

// Public Ctors & Dtors
public:


    /**
     * @brief Constructor.
     *
     * @param parent Pointer to parent window.
     * @param plugin Pointer to the plugin.
     */
    CMakeHelpTab(wxWindow* parent, CMakePlugin* plugin);


// Protected Events
protected:


    /**
     * @brief On topic change.
     *
     * @param event
     */
    virtual void OnChangeTopic(wxCommandEvent& event);


    /**
     * @brief On item insert into editor.
     *
     * @param event
     */
    virtual void OnInsert(wxCommandEvent& event);


    /**
     * @brief On item search.
     *
     * @param event
     */
    virtual void OnSearch(wxCommandEvent& event);


    /**
     * @brief On item search cancel.
     *
     * @param event
     */
    virtual void OnSearchCancel(wxCommandEvent& event);


    /**
     * @brief On item select.
     *
     * @param event
     */
    virtual void OnSelect(wxCommandEvent& event);


    /**
     * @brief On CMake help data reload.
     *
     * @param event
     */
    virtual void OnReload(wxCommandEvent& event);


    /**
     * @brief On right mouse button click event.
     *
     * @param event
     */
    virtual void OnRightClick(wxMouseEvent& event);


    /**
     * @brief On splitter orientation switch.
     *
     * @param event
     */
    void OnSplitterSwitch(wxCommandEvent& event);


    /**
     * @brief Updates UI.
     *
     * @param event
     */
    void OnThreadUpdate(wxThreadEvent& event);


    /**
     * @brief On tab close.
     *
     * @param event
     */
    void OnClose(wxCloseEvent& event);


    /**
     * @brief Some items update UI.
     *
     * @param event
     */
    virtual void OnUpdateUi(wxUpdateUIEvent& event);


// Protected Operations
protected:


    /**
     * @brief Does the thread things.
     *
     * @return Exit code.
     */
    virtual wxThread::ExitCode Entry();


    /**
     * @brief Loads data from CMake object into UI objects.
     *
     * @param force If data should be reloaded from CMake instead
     *              from database.
     */
    void LoadData(bool force = false);


// Private Operations
private:


    /**
     * @brief List all items.
     */
    void ListAll();


    /**
     * @brief List only items that match search string.
     *
     * @param search
     */
    void ListFiltered(const wxString& search);


// Private Data Members
private:


    /// A pointer to cmake plugin.
    CMakePlugin* const m_plugin;

    /// Current topic data.
    const std::map<wxString, wxString>* m_data;

    /// Temporary variable.
    bool m_force;

    /// Busy flag
    bool m_busy;

};

/* ************************************************************************ */

#endif // CMAKE_HELP_TAB_H_
