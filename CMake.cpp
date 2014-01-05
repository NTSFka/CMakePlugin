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
#include "CMake.h"

// wxWidgets
#include <wx/regex.h>
#include <wx/event.h>
#include <wx/thread.h>
#include <wx/scopedptr.h>

// Codelite
#include "procutils.h"
#include "workspace.h"
#include "globals.h"
#include "file_logger.h"

/* ************************************************************************ */
/* DEFINITIONS                                                              */
/* ************************************************************************ */

wxDEFINE_EVENT(EVT_UPDATE_THREAD, wxThreadEvent);

/* ************************************************************************ */
/* FUNCTIONS                                                                */
/* ************************************************************************ */

/**
 * @brief Joins array of strings into single string.
 *
 * @param array Input array.
 *
 * @return Result string.
 */
static wxString JoinHtml(const wxArrayString& array)
{
    wxString result;

    for (wxArrayString::const_iterator it = array.begin(), ite = array.end(); it != ite; ++it) {
        if (it != array.begin()) {
            result += "<br />";
        }
        result += *it;
    }

    return result;
}

/* ************************************************************************ */

/**
 * @brief Loads help of type from command into list.
 *
 * @param command  CMake command.
 * @param type     Help type.
 * @param list     Output variable.
 * @param progress Optional progress dialog.
 */
static void LoadList(const wxString& command, const wxString& type,
                     CMake::HelpMap& list)
{
    // Get list
    wxArrayString names;
    const wxString cmdList = command + " --help-" + type + "-list";
    ProcUtils::SafeExecuteCommand(cmdList, names);

    // Remove version
    if (!names.IsEmpty())
        names.RemoveAt(0);

    // Foreach names
    for (wxArrayString::const_iterator it = names.begin(), ite = names.end(); it != ite; ++it) {
        // Export help
        wxArrayString desc;
        const wxString cmdItem = command + " --help-" + type + " \"" + *it + "\"";
        ProcUtils::SafeExecuteCommand(cmdItem, desc);

        // Skip empty results
        if (desc.IsEmpty())
            continue;

        // Remove first line (cmake version)
        if (desc.Item(0).Matches("*cmake version*"))
            desc.RemoveAt(0);

        // Store help page
        list[*it] = JoinHtml(desc);
    }
}

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMake::CMake(const wxFileName& path)
    : m_path(path)
    , m_dbFileName(wxStandardPaths::Get().GetUserDataDir(), "cmake.db")
{
    // Prepare database
    PrepareDatabase();
}

/* ************************************************************************ */

wxArrayString
CMake::GetVersions()
{
    static const wxString VERSIONS[] = {
        "2.8.11",
        "2.8.10",
        "2.8.9",
        "2.8.8",
        "2.8.7",
        "2.8.6",
        "2.8.5",
        "2.8.4",
        "2.8.3",
        "2.8.2",
        "2.8.1",
        "2.8.0",
        "2.6.4",
        "2.6.3",
        "2.6.2",
        "2.6.1",
        "2.6.0",
        "2.4.8",
        "2.4.7",
        "2.4.6",
        "2.4.5",
        "2.4.4",
        "2.4.3",
        "2.2.3",
        "2.0.6",
        "1.8.3"
    };

    return wxArrayString(sizeof(VERSIONS) / sizeof(VERSIONS[0]), VERSIONS);
}

/* ************************************************************************ */

bool
CMake::IsOk() const
{
    wxArrayString output;
    ProcUtils::SafeExecuteCommand(GetPath().GetFullPath() + " -h", output);

    // SafeExecuteCommand doesn't return status code so the only way
    // to test the success is the output emptiness.
    return !output.empty();
}

/* ************************************************************************ */

bool
CMake::LoadData(bool force, wxEvtHandler* handler)
{
    // Update thread event
    wxScopedPtr<wxThreadEvent> event(new wxThreadEvent(EVT_UPDATE_THREAD));

    // Clear old data
    m_version.clear();
    m_commands.clear();
    m_modules.clear();
    m_properties.clear();
    m_variables.clear();

    // Load data from database
    if (!force && m_dbInitialized && LoadFromDatabase()) {
        return true;
    }

    // Unable to use CMake
    if (!IsOk())
        return false;

    // Get cmake program path
    const wxString program = GetPath().GetFullPath();

    // Version
    {
        wxArrayString output;
        ProcUtils::SafeExecuteCommand(program + " --version", output);

        // Unable to find version
        if (!output.IsEmpty()) {
            const wxString& versionLine = output[0];
            wxRegEx expression("cmake version ([0-9\\.]+)");

            if (expression.IsValid() && expression.Matches(versionLine)) {
                m_version = expression.GetMatch(versionLine, 1);
            }
        }
    }

    // Load data
    LoadFromCMake(handler);

    // Database is open so we can store result into database
    if (m_dbInitialized) {
        StoreIntoDatabase();
    }

    if (handler) {
        event->SetInt(100);
        handler->QueueEvent(event->Clone());
    }

    return true;
}

/* ************************************************************************ */

void
CMake::PrepareDatabase()
{
    m_dbInitialized = false;

    try {
        // Try to open database
        m_db.Open(GetDatabaseFileName().GetFullPath());

        // Not opened
        if (!m_db.IsOpen())
            return;

        m_db.Begin();

        // Create tables
        m_db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS commands (name TEXT, desc TEXT)");
        m_db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS modules (name TEXT, desc TEXT)");
        m_db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS properties (name TEXT, desc TEXT)");
        m_db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS variables (name TEXT, desc TEXT)");
        m_db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS strings (name TEXT, desc TEXT)");

        // Create indices
        m_db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS commands_idx ON commands(name)");
        m_db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS modules_idx ON modules(name)");
        m_db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS properties_idx ON properties(name)");
        m_db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS variables_idx ON variables(name)");
        m_db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS strings_idx ON strings(name)");

        m_db.Commit();

        // Database is not needed to be opened the whole time
        m_db.Close();

        // Everything is OK
        m_dbInitialized = true;

    } catch (const wxSQLite3Exception& e) {
        // Unable to use SQLite database
        CL_ERROR("CMake DoPrepareDatabase error: %s", e.GetMessage());
    }
}

/* ************************************************************************ */

void
CMake::LoadFromCMake(wxEvtHandler* handler)
{
    // Get cmake program path
    const wxString program = GetPath().GetFullPath();

    wxScopedPtr<wxThreadEvent> event(new wxThreadEvent(EVT_UPDATE_THREAD));

    if (handler) {
        event->SetInt(0);
        handler->QueueEvent(event->Clone());
    }

    LoadList(program, "command",  m_commands);

    if (handler) {
        event->SetInt(25);
        handler->QueueEvent(event->Clone());
    }

    LoadList(program, "module",   m_modules);


    if (handler) {
        event->SetInt(50);
        handler->QueueEvent(event->Clone());
    }

    LoadList(program, "property", m_properties);


    if (handler) {
        event->SetInt(75);
        handler->QueueEvent(event->Clone());
    }

    LoadList(program, "variable", m_variables);
}

/* ************************************************************************ */

bool
CMake::LoadFromDatabase()
{
    if (!m_dbInitialized) {
        return false;
    }

    try
    {
        // Open
        m_db.Open(GetDatabaseFileName().GetFullPath());

        // Not opened
        if (!m_db.IsOpen())
            return false;

        // Strings - Version
        {
            wxSQLite3ResultSet res = m_db.ExecuteQuery("SELECT desc FROM strings WHERE name = 'version'");
            if (res.NextRow()) {
                m_version = res.GetAsString(0);
            }
        }

        // No data stored
        if (m_version.IsEmpty())
            return false;

        // Commands
        {
            wxSQLite3ResultSet res = m_db.ExecuteQuery("SELECT name, desc FROM commands");
            while (res.NextRow()) {
                m_commands[res.GetAsString(0)] = res.GetAsString(1);
            }
        }

        // Modules
        {
            wxSQLite3ResultSet res = m_db.ExecuteQuery("SELECT name, desc FROM modules");
            while (res.NextRow()) {
                m_modules[res.GetAsString(0)] = res.GetAsString(1);
            }
        }

        // Properties
        {
            wxSQLite3ResultSet res = m_db.ExecuteQuery("SELECT name, desc FROM properties");
            while (res.NextRow()) {
                m_properties[res.GetAsString(0)] = res.GetAsString(1);
            }
        }

        // Variables
        {
            wxSQLite3ResultSet res = m_db.ExecuteQuery("SELECT name, desc FROM variables");
            while (res.NextRow()) {
                m_variables[res.GetAsString(0)] = res.GetAsString(1);
            }
        }

    } catch (const wxSQLite3Exception& e) {
        CL_ERROR("Error occured while loading data from CMake database: %s", e.GetMessage());
    }

    // Not needed anymore
    m_db.Close();

    // Everything is loaded
    return true;
}

/* ************************************************************************ */

void
CMake::StoreIntoDatabase()
{
    if (!m_dbInitialized) {
        CL_WARNING("CMake: can't store data into database. Database was not initialized properly");
        return;
    }

    try
    {
        // Open
        m_db.Open(GetDatabaseFileName().GetFullPath());

        // Not opened
        if (!m_db.IsOpen())
            return;

        m_db.Begin();

        // Commands
        {
            m_db.ExecuteUpdate("DELETE FROM commands");
            wxSQLite3Statement stmt = m_db.PrepareStatement("INSERT INTO commands (name, desc) VALUES(?, ?)");
            for (HelpMap::const_iterator it = m_commands.begin(), ite = m_commands.end(); it != ite; ++it) {
                stmt.Bind(1, it->first);
                stmt.Bind(2, it->second);
                stmt.ExecuteUpdate();
            }
        }

        // Modules
        {
            m_db.ExecuteUpdate("DELETE FROM modules");
            wxSQLite3Statement stmt = m_db.PrepareStatement("INSERT INTO modules (name, desc) VALUES(?, ?)");
            for (HelpMap::const_iterator it = m_modules.begin(), ite = m_modules.end(); it != ite; ++it) {
                stmt.Bind(1, it->first);
                stmt.Bind(2, it->second);
                stmt.ExecuteUpdate();
            }
        }

        // Properties
        {
            m_db.ExecuteUpdate("DELETE FROM properties");
            wxSQLite3Statement stmt = m_db.PrepareStatement("INSERT INTO properties (name, desc) VALUES(?, ?)");
            for (HelpMap::const_iterator it = m_properties.begin(), ite = m_properties.end(); it != ite; ++it) {
                stmt.Bind(1, it->first);
                stmt.Bind(2, it->second);
                stmt.ExecuteUpdate();
            }
        }

        // Variables
        {
            m_db.ExecuteUpdate("DELETE FROM variables");
            wxSQLite3Statement stmt = m_db.PrepareStatement("INSERT INTO variables (name, desc) VALUES(?, ?)");
            for (HelpMap::const_iterator it = m_variables.begin(), ite = m_variables.end(); it != ite; ++it) {
                stmt.Bind(1, it->first);
                stmt.Bind(2, it->second);
                stmt.ExecuteUpdate();
            }
        }

        // Strings - Version
        {
            wxSQLite3Statement stmt = m_db.PrepareStatement("REPLACE INTO strings (name, desc) VALUES('version', ?)");
            stmt.Bind(1, m_version);
            stmt.ExecuteUpdate();
        }

        m_db.Commit();

    } catch (wxSQLite3Exception &e) {
        CL_ERROR("An error occured while storing CMake data into database: %s", e.GetMessage());
    }

    // Not needed anymore
    m_db.Close();

}

/* ************************************************************************ */
