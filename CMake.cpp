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
#include <wx/wxsqlite3.h>

// Codelite
#include "procutils.h"
#include "workspace.h"
#include "globals.h"
#include "file_logger.h"

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
                     CMake::HelpMap& list, wxProgressDialog* progress)
{
    // Get list
    wxArrayString names;
    const wxString cmdList = command + " --help-" + type + "-list";
    long res = wxExecute(cmdList, names);

    // Unable to load
    if (res != 0) {
        CL_ERROR("CMakePlugin: Unable to call: " + cmdList);
        return;
    }

    // Remove version
    names.RemoveAt(0);

    // Foreach names
    for (wxArrayString::const_iterator it = names.begin(), ite = names.end(); it != ite; ++it) {
        // Export help
        wxArrayString desc;
        const wxString cmdItem = command + " --help-" + type + " " + *it;
        res = wxExecute(cmdItem, desc);

        if (res != 0) {
            CL_ERROR("CMakePlugin: Unable to call: " + cmdItem);
            continue;
        }

        // Remove first line (cmake version)
        if (desc.Item(0).Matches("*cmake version*"))
            desc.RemoveAt(0);

        // Store help page
        list[*it] = JoinHtml(desc);
    }
}

/* ************************************************************************ */

/**
 * @brief Prepare database for CMake.
 *
 * @param db       Database
 * @param filename Path where the database is stored.
 *
 * @return If database can be usable
 */
static bool PrepareDatabase(wxSQLite3Database& db, const wxFileName& filename)
{
    try {
        // Try to open database
        db.Open(filename.GetFullPath());

        // Not opened
        if (!db.IsOpen())
            return false;

        db.Begin();

        // Create tables
        db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS commands (name TEXT, desc TEXT)");
        db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS modules (name TEXT, desc TEXT)");
        db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS properties (name TEXT, desc TEXT)");
        db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS variables (name TEXT, desc TEXT)");
        db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS strings (name TEXT, desc TEXT)");

        // Create indices
        db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS commands_idx ON commands(name)");
        db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS modules_idx ON modules(name)");
        db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS properties_idx ON properties(name)");
        db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS variables_idx ON variables(name)");
        db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS strings_idx ON strings(name)");

        db.Commit();

        // Everything is OK
        return true;

    } catch (const wxSQLite3Exception& e) {
        // Unable to use SQLite database
        return false;
    }
}

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMake::CMake(const wxFileName& path)
    : m_dirty(true)
    , m_path(path)
{
    // Nothing to do
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

void
CMake::LoadData(bool force, bool onlyCached, wxProgressDialog* progress)
{
    // Loading data again is not required
    if (!m_dirty && !force)
        return;

    // Clear old data
    m_version.clear();
    m_commands.clear();
    m_modules.clear();
    m_properties.clear();
    m_variables.clear();

    // Create SQLite database
    wxSQLite3Database db;

    // Open it
    bool dbOk = PrepareDatabase(db, wxFileName(wxStandardPaths::Get().GetUserDataDir(), "cmake.db"));

    // Load data from database
    if (!force && dbOk && LoadFromDatabase(db)) {
        return;
    }

    // Do not continue
    if (onlyCached) {
        return;
    }

    // Unable to use CMake
    if (!IsOk())
        return;

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
    LoadFromCMake(progress);

    // Database is open so we can store result into database
    if (dbOk) {
        StoreIntoDatabase(db, progress);
    }

    if (progress)
        progress->Update(progress->GetRange(), "Done");

    m_dirty = false;
}

/* ************************************************************************ */

void
CMake::LoadFromCMake(wxProgressDialog* progress)
{
    // Get cmake program path
    const wxString program = GetPath().GetFullPath();

    if (progress) progress->Pulse("Loading commands...");
    LoadList(program, "command",  m_commands, progress);

    if (progress) progress->Pulse("Loading modules...");
    LoadList(program, "module",   m_modules, progress);

    if (progress) progress->Pulse("Loading properties...");
    LoadList(program, "property", m_properties, progress);

    if (progress) progress->Pulse("Loading variables...");
    LoadList(program, "variable", m_variables, progress);
}

/* ************************************************************************ */

bool
CMake::LoadFromDatabase(wxSQLite3Database& db)
{
    // Database is closed
    if (!db.IsOpen())
        return false;

    // Strings - Version
    {
        wxSQLite3ResultSet res = db.ExecuteQuery("SELECT desc FROM strings WHERE name = 'version'");
        if (res.NextRow()) {
            m_version = res.GetAsString(0);
        }
    }

    // No data stored
    if (m_version.IsEmpty())
        return false;

    // Commands
    {
        wxSQLite3ResultSet res = db.ExecuteQuery("SELECT name, desc FROM commands");
        while (res.NextRow()) {
            m_commands[res.GetAsString(0)] = res.GetAsString(1);
        }
    }

    // Modules
    {
        wxSQLite3ResultSet res = db.ExecuteQuery("SELECT name, desc FROM modules");
        while (res.NextRow()) {
            m_modules[res.GetAsString(0)] = res.GetAsString(1);
        }
    }

    // Properties
    {
        wxSQLite3ResultSet res = db.ExecuteQuery("SELECT name, desc FROM properties");
        while (res.NextRow()) {
            m_properties[res.GetAsString(0)] = res.GetAsString(1);
        }
    }

    // Variables
    {
        wxSQLite3ResultSet res = db.ExecuteQuery("SELECT name, desc FROM variables");
        while (res.NextRow()) {
            m_variables[res.GetAsString(0)] = res.GetAsString(1);
        }
    }

    // Data is OK
    m_dirty = false;

    // Everything is loaded
    return true;
}

/* ************************************************************************ */

bool
CMake::StoreIntoDatabase(wxSQLite3Database& db, wxProgressDialog* progress) const
{
    // Database is closed
    if (!db.IsOpen())
        return false;

    // Commands
    {
        if (progress)
            progress->Pulse("Storing commands...");

        db.Begin();

        db.ExecuteUpdate("DELETE FROM commands");
        wxSQLite3Statement stmt = db.PrepareStatement("INSERT INTO commands (name, desc) VALUES(?, ?)");
        for (HelpMap::const_iterator it = m_commands.begin(), ite = m_commands.end(); it != ite; ++it) {
            stmt.Bind(1, it->first);
            stmt.Bind(2, it->second);
            stmt.ExecuteUpdate();
        }

        db.Commit();
    }

    // Modules
    {
        if (progress)
            progress->Pulse("Storing modules...");

        db.Begin();

        db.ExecuteUpdate("DELETE FROM modules");
        wxSQLite3Statement stmt = db.PrepareStatement("INSERT INTO modules (name, desc) VALUES(?, ?)");
        for (HelpMap::const_iterator it = m_modules.begin(), ite = m_modules.end(); it != ite; ++it) {
            stmt.Bind(1, it->first);
            stmt.Bind(2, it->second);
            stmt.ExecuteUpdate();
        }

        db.Commit();
    }

    // Properties
    {
        if (progress)
            progress->Pulse("Storing properties...");

        db.Begin();

        db.ExecuteUpdate("DELETE FROM properties");
        wxSQLite3Statement stmt = db.PrepareStatement("INSERT INTO properties (name, desc) VALUES(?, ?)");
        for (HelpMap::const_iterator it = m_properties.begin(), ite = m_properties.end(); it != ite; ++it) {
            stmt.Bind(1, it->first);
            stmt.Bind(2, it->second);
            stmt.ExecuteUpdate();
        }

        db.Commit();
    }

    // Variables
    {
        if (progress)
            progress->Pulse("Storing variables...");

        db.Begin();

        db.ExecuteUpdate("DELETE FROM variables");
        wxSQLite3Statement stmt = db.PrepareStatement("INSERT INTO variables (name, desc) VALUES(?, ?)");
        for (HelpMap::const_iterator it = m_variables.begin(), ite = m_variables.end(); it != ite; ++it) {
            stmt.Bind(1, it->first);
            stmt.Bind(2, it->second);
            stmt.ExecuteUpdate();
        }

        db.Commit();
    }

    // Strings - Version
    {
        if (progress)
            progress->Pulse("Storing version...");

        db.Begin();

        wxSQLite3Statement stmt = db.PrepareStatement("REPLACE INTO strings (name, desc) VALUES('version', ?)");
        stmt.Bind(1, m_version);
        stmt.ExecuteUpdate();

        db.Commit();
    }

    return true;
}

/* ************************************************************************ */
