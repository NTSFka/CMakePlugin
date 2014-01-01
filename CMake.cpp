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

/* ************************************************************************ */
/* FUNCTIONS                                                                */
/* ************************************************************************ */

/**
 * @brief Parses man page block with available generators.
 *
 * @param line Current parsed line imutable iterator.
 *
 * @return A list of available generators.
 */
static
wxArrayString ParseManGenerators(wxArrayString::const_iterator& line)
{
    wxArrayString generators;

    // Read until another section is found
    for (++line; !line->StartsWith(".SH"); ++line) {
        wxString name;

        // .B marks generator name
        if (line->StartsWith(".B ", &name)) {
            // Remove trailing newline
            name.Trim();
            // Store generator name
            generators.push_back(name);
        }
    }

    // Previous line
    --line;

    return generators;
}

/* ************************************************************************ */

/**
 * @brief Parses man page block with text.
 *
 * @param line Current parsed line imutable iterator.
 *
 * @return Text.
 */
static
wxString ParseManText(wxArrayString::const_iterator& line)
{
    wxString text;

    // Read until another section is found
    for (++line; !line->StartsWith(".SH"); ++line) {
        // Ignore lines with macros

        // From: http://www.fnal.gov/docs/products/ups/ReferenceManual/html/manpages.html
        // .SH "<text>" : Section Heading; if no blanks in text, quotes are not needed.
        // .SS "<text>" : Subsection Heading; if no blanks in text, quotes are not needed.
        // .P           : Paragraph break
        // .IP "<item>" : Starts an indented paragraph where "item" is put to the left of it; if no blanks in "item", quotes are not needed.
        // .HP          : Starts a paragraph with a hanging indent; i.e. lines after the first are indented
        // .RE          : Defines an indented region
        // .B "<text>"  : Bold; if no blanks in text, quotes are not needed.
        // .I "<text>"  : Italic; this shows up as underlined on most terminals. If no blanks in text, quotes are not needed.
        // .TP <columns> : Term/paragraph format; columns specify how many columns to allocate to the term column. As an example, this input:
        // .P           : New paragraph
        // .br          : Break line
        // .nf          : Nofill (used to suppress normal line filling; used for preformatted text)
        // .fi          : Fill (used to resume normal line filling, usually after a .nf)
        // ./"          : Comment line
        if (line->StartsWith(".")) {
            const bool isMacro =
                line->StartsWith(".SS") ||
                line->StartsWith(".P") ||
                line->StartsWith(".IP") ||
                line->StartsWith(".HP") ||
                line->StartsWith(".RE") ||
                line->StartsWith(".B") ||
                line->StartsWith(".I") ||
                line->StartsWith(".TP") ||
                line->StartsWith(".br") ||
                line->StartsWith(".nf") ||
                line->StartsWith(".fi") ||
                line->StartsWith("./\"")
            ;

            if (isMacro)
                continue;
        }

        // Add text
        text.Append(*line);
    }

    // Previous line
    --line;

    // Replace all dash escape sequences: \- => -
    text.Replace("\\-", "-");

    return text;
}

/* ************************************************************************ */

/**
 * @brief Parses man page block with available data.
 *
 * @param line Current parsed line imutable iterator.
 * @param data Output data container.
 */
static
void ParseManDesc(wxArrayString::const_iterator& line, CMake::LinesMap& data)
{
    wxString name;
    wxString newName;
    wxArrayString desc;
    bool store = false;

    // Read until another section is found
    for (++line; !line->StartsWith(".SH"); ++line) {
        // .B marks name
        if (line->StartsWith(".B ", &newName)) {
            // Remove trailing newline
            newName.Trim();
            store = true;

            if (!desc.IsEmpty()) {
                // Store name and description
                data.insert(std::make_pair(name, desc));
                desc.clear();
            }

            name = newName;

        } else if (store) {

            // Skip MAN macros
            if (line->StartsWith(".")) {
                const bool isMacro =
                    line->StartsWith(".SS") ||
                    line->StartsWith(".P") ||
                    line->StartsWith(".IP") ||
                    line->StartsWith(".HP") ||
                    line->StartsWith(".RE") ||
                    line->StartsWith(".B") ||
                    line->StartsWith(".I") ||
                    line->StartsWith(".TP") ||
                    line->StartsWith(".br") ||
                    line->StartsWith(".nf") ||
                    line->StartsWith(".fi") ||
                    line->StartsWith("./\"")
                ;

                if (isMacro)
                    continue;
            }

            wxString copy = *line;
            // Replace all dash escape sequences: \- => -
            copy.Replace("\\-", "-");

            // Append line
            desc.push_back(copy);
        }
    }

    // Previous line
    --line;
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

        // Create tables
        db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS generators (name TEXT)");
        db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS commands (name TEXT, desc TEXT)");
        db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS modules (name TEXT, desc TEXT)");
        db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS properties (name TEXT, desc TEXT)");
        db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS variables (name TEXT, desc TEXT)");
        db.ExecuteUpdate("CREATE TABLE IF NOT EXISTS strings (name TEXT, desc TEXT)");

        // Create indices
        db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS generators_idx ON generators(name)");
        db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS commands_idx ON commands(name)");
        db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS modules_idx ON modules(name)");
        db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS properties_idx ON properties(name)");
        db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS variables_idx ON variables(name)");
        db.ExecuteUpdate("CREATE UNIQUE INDEX IF NOT EXISTS strings_idx ON strings(name)");

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
CMake::LoadData(bool force)
{
    // Loading data again is not required
    if (!m_dirty && !force)
        return;

    // Clear old data
    m_version.clear();
    m_generators.clear();
    m_commands.clear();
    m_modules.clear();
    m_properties.clear();
    m_variables.clear();
    m_copyright.clear();

    // Create SQLite database
    wxSQLite3Database db;

    // Open it
    bool dbOk = PrepareDatabase(db, wxFileName(wxStandardPaths::Get().GetUserDataDir(), "cmake.db"));

    // Load data from database
    if (!force && dbOk && LoadFromDatabase(db))
        return;

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

    // Parse data from CMake manual page
    ParseCMakeManPage();

    // Database is open so we can store result into database
    if (dbOk)
        StoreIntoDatabase(db);

    m_dirty = false;
}

/* ************************************************************************ */

void
CMake::ParseCMakeManPage()
{
    // Get cmake program path
    const wxString program = GetPath().GetFullPath();

    // Get CMake man page
    wxArrayString output;
    ProcUtils::SafeExecuteCommand(program + " --help-man", output);

    // Foreach lines
    for (wxArrayString::const_iterator line = output.begin();
        line != output.end(); ++line) {
        // Generators
        if (line->StartsWith(".SH GENERATORS")) {
            m_generators = ParseManGenerators(line);
        } else if (line->StartsWith(".SH COMMANDS")) {
            ParseManDesc(line, m_commands);
        } else if (line->StartsWith(".SH PROPERTIES")) {
            ParseManDesc(line, m_properties);
        } else if (line->StartsWith(".SH MODULES")) {
            ParseManDesc(line, m_modules);
        } else if (line->StartsWith(".SH VARIABLES")) {
            ParseManDesc(line, m_variables);
        } else if (line->StartsWith(".SH COPYRIGHT")) {
            m_copyright = ParseManText(line);
        }
    }
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

    // Strings - Copyright
    {
        wxSQLite3ResultSet res = db.ExecuteQuery("SELECT desc FROM strings WHERE name = 'copyright'");
        if (res.NextRow()) {
            m_copyright = res.GetAsString(0);
        }
    }

    // Generators
    {
        wxSQLite3ResultSet res = db.ExecuteQuery("SELECT name FROM generators");
        while (res.NextRow()) {
            m_generators.Add(res.GetAsString(0));
        }
    }

    // Commands
    {
        wxSQLite3ResultSet res = db.ExecuteQuery("SELECT name, desc FROM commands");
        while (res.NextRow()) {
            m_commands[res.GetAsString(0)] = wxSplit(res.GetAsString(1), '\n');
        }
    }

    // Modules
    {
        wxSQLite3ResultSet res = db.ExecuteQuery("SELECT name, desc FROM modules");
        while (res.NextRow()) {
            m_modules[res.GetAsString(0)] = wxSplit(res.GetAsString(1), '\n');
        }
    }

    // Properties
    {
        wxSQLite3ResultSet res = db.ExecuteQuery("SELECT name, desc FROM properties");
        while (res.NextRow()) {
            m_properties[res.GetAsString(0)] = wxSplit(res.GetAsString(1), '\n');
        }
    }

    // Variables
    {
        wxSQLite3ResultSet res = db.ExecuteQuery("SELECT name, desc FROM variables");
        while (res.NextRow()) {
            m_variables[res.GetAsString(0)] = wxSplit(res.GetAsString(1), '\n');
        }
    }

    // Data is OK
    m_dirty = false;

    // Everything is loaded
    return true;
}

/* ************************************************************************ */

bool
CMake::StoreIntoDatabase(wxSQLite3Database& db) const
{
    // Database is closed
    if (!db.IsOpen())
        return false;

    // Generators
    {
        wxSQLite3Statement stmt = db.PrepareStatement("REPLACE INTO generators (name) VALUES(?)");
        for (wxArrayString::const_iterator it = m_generators.begin(), ite = m_generators.end(); it != ite; ++it) {
            stmt.Bind(1, *it);
            stmt.ExecuteUpdate();
        }
    }

    // Commands
    {
        wxSQLite3Statement stmt = db.PrepareStatement("REPLACE INTO commands (name, desc) VALUES(?, ?)");
        for (LinesMap::const_iterator it = m_commands.begin(), ite = m_commands.end(); it != ite; ++it) {
            stmt.Bind(1, it->first);
            stmt.Bind(2, wxJoin(it->second, '\n'));
            stmt.ExecuteUpdate();
        }
    }

    // Modules
    {
        wxSQLite3Statement stmt = db.PrepareStatement("REPLACE INTO modules (name, desc) VALUES(?, ?)");
        for (LinesMap::const_iterator it = m_modules.begin(), ite = m_modules.end(); it != ite; ++it) {
            stmt.Bind(1, it->first);
            stmt.Bind(2, wxJoin(it->second, '\n'));
            stmt.ExecuteUpdate();
        }
    }

    // Properties
    {
        wxSQLite3Statement stmt = db.PrepareStatement("REPLACE INTO properties (name, desc) VALUES(?, ?)");
        for (LinesMap::const_iterator it = m_properties.begin(), ite = m_properties.end(); it != ite; ++it) {
            stmt.Bind(1, it->first);
            stmt.Bind(2, wxJoin(it->second, '\n'));
            stmt.ExecuteUpdate();
        }
    }

    // Variables
    {
        wxSQLite3Statement stmt = db.PrepareStatement("REPLACE INTO variables (name, desc) VALUES(?, ?)");
        for (LinesMap::const_iterator it = m_variables.begin(), ite = m_variables.end(); it != ite; ++it) {
            stmt.Bind(1, it->first);
            stmt.Bind(2, wxJoin(it->second, '\n'));
            stmt.ExecuteUpdate();
        }
    }

    // Strings - Copyright
    {
        wxSQLite3Statement stmt = db.PrepareStatement("REPLACE INTO strings (name, desc) VALUES('copyright', ?)");
        stmt.Bind(1, m_copyright);
        stmt.ExecuteUpdate();
    }

    // Strings - Version
    {
        wxSQLite3Statement stmt = db.PrepareStatement("REPLACE INTO strings (name, desc) VALUES('version', ?)");
        stmt.Bind(1, m_version);
        stmt.ExecuteUpdate();
    }

    return true;
}

/* ************************************************************************ */
