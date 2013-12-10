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

// Codelite
#include "procutils.h"

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
void ParseManDesc(wxArrayString::const_iterator& line, wxStringMap_t& data)
{
    wxString name;
    wxString newName;
    wxString desc;
    bool store = false;

    // Read until another section is found
    for (++line; !line->StartsWith(".SH"); ++line) {
        // .B marks name
        if (line->StartsWith(".B ", &newName)) {
            // Remove trailing newline
            newName.Trim();
            store = true;

            if (!desc.empty()) {
                // Replace all dash escape sequences: \- => -
                desc.Replace("\\-", "-");

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

            // Append line
            desc.Append(*line);
        }
    }

    // Previous line
    --line;
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
CMake::LoadData()
{
    // Loading data again is not required
    if (!m_dirty)
        return;

    // Clear old data
    m_version.clear();
    m_commands.clear();
    m_modules.clear();
    m_properties.clear();
    m_variables.clear();
    m_copyright.clear();

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
