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
#include "CMake.hpp"

// C++
#include <map>

// wxWidgets
#include <wx/regex.h>

// Codelite
#include "procutils.h"

/* ************************************************************************ */
/* FUNCTIONS                                                                */
/* ************************************************************************ */

/**
 * @brief Join all strings from array into one beautifull string.
 *
 * @param arr Input array.
 *
 * @return Joined string.
 */
static
wxString Join(const wxArrayString& arr)
{
    if (arr.IsEmpty())
        return wxEmptyString;

    wxString res;

    // pre-allocate memory using the estimation of the average length of the
    // strings in the given array: this is very imprecise, of course, but
    // better than nothing
    res.reserve(arr.size() * (arr.front().length() + arr.Last().length()) / 2);

    // Find padding
    const size_t pos = arr[0].find_first_not_of(' ');

    // escaping is disabled:
    for (size_t i = 0u; i < arr.size(); ++i)
    {
        wxString str = arr[i];

        // Remove whitespaces
        str.Trim();

        // Find local padding
        size_t localPos = str.find_first_not_of(' ');
        // Use minimal padding
        localPos = std::min(localPos, pos);

        // Remove first 4 characters (that should be spaces)
        if (localPos != wxString::npos && str.length() > localPos)
            str = str.substr(localPos);

        res += str + '\n';
    }

    res.Shrink(); // release extra memory if we allocated too much
    return res;
}

/* ************************************************************************ */

/**
 * @brief Removes that ugly whitespaces around strings in given array.
 *
 * @param arr Input array.
 *
 * @return Trimmed array.
 */
static
wxArrayString Trim(const wxArrayString& arr)
{
    wxArrayString res;
    res.reserve(arr.size());

    for (auto item : arr)
    {
        item.Trim();
        res.push_back(std::move(item));
    }

    return res;
}

/* ************************************************************************ */

/**
 * @brief Helper function for parsing and caching CMake help output.
 *
 * @param cache   Mutable cache variable where data are stored.
 * @param name    Key value.
 * @param command Command that will be executed for fetching help data.
 *
 * @return Help data.
 */
static
wxString CacheHelp(std::map<wxString, wxString>& cache,
                   const wxString& name,
                   const wxString& command)
{
    // Try to find in cache
    auto it = cache.find(name);

    // Found in cache
    if (it != cache.end())
        return it->second;

    // Call command
    wxArrayString output;
    ProcUtils::SafeExecuteCommand(command, output);

    if (output.IsEmpty())
        return "(ERROR)";

    // The first line is CMake version
    output.RemoveAt(0);
    // The second line is the command name
    output.RemoveAt(0);

    return Join(output);
}

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
    for (++line; !line->StartsWith(".SH"); ++line)
    {
        wxString name;

        // .B marks generator name
        if (line->StartsWith(".B ", &name))
        {
            // Remove trailing newline
            name.Trim();
            // Store generator name
            generators.push_back(name);
        }
    }

    return generators;
}

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMake::CMake(const wxFileName& path)
    : m_path(path)
{
    LoadData();
}

/* ************************************************************************ */

wxArrayString
CMake::GetVersions() noexcept
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

    return {sizeof(VERSIONS) / sizeof(VERSIONS[0]), VERSIONS};
}

/* ************************************************************************ */

bool
CMake::IsOk() const noexcept
{
    wxArrayString output;
    ProcUtils::SafeExecuteCommand(GetPath().GetFullPath() + " -h", output);

    // SafeExecuteCommand doesn't return status code so the only way
    // to test the success is the output emptiness.
    return !output.empty();
}

/* ************************************************************************ */

wxString
CMake::GetModuleHelp(const wxString& name) const noexcept
{
    static std::map<wxString, wxString> cache;

    // Get module help
    return CacheHelp(cache, name, GetPath().GetFullPath() + " --help-module \"" + name + "\"");
}

/* ************************************************************************ */

wxString
CMake::GetCommandHelp(const wxString& name) const noexcept
{
    static std::map<wxString, wxString> cache;

    // Get command help
    return CacheHelp(cache, name, GetPath().GetFullPath() + " --help-command \"" + name + "\"");
}

/* ************************************************************************ */

wxString
CMake::GetPropertyHelp(const wxString& name) const noexcept
{
    static std::map<wxString, wxString> cache;

    // Get property help
    return CacheHelp(cache, name, GetPath().GetFullPath() + " --help-property \"" + name + "\"");
}

/* ************************************************************************ */

wxString
CMake::GetVariableHelp(const wxString& name) const noexcept
{
    static std::map<wxString, wxString> cache;

    // Get variable help
    return CacheHelp(cache, name, GetPath().GetFullPath() + " --help-variable \"" + name + "\"");
}

/* ************************************************************************ */

void
CMake::LoadData() noexcept
{
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
        if (!output.IsEmpty())
        {
            const wxString& versionLine = output[0];
            wxRegEx expression("cmake version ([0-9\\.]+)");

            if (expression.IsValid() && expression.Matches(versionLine))
            {
                m_version = expression.GetMatch(versionLine, 1);
            }
        }
    }

    // Commands
    {
        wxArrayString output;
        ProcUtils::SafeExecuteCommand(program + " --help-command-list", output);

        // First line is CMake version
        output.RemoveAt(0);

        m_commands = Trim(output);
    }

    // Modules
    {
        wxArrayString output;
        ProcUtils::SafeExecuteCommand(program + " --help-module-list", output);

        // First line is CMake version
        output.RemoveAt(0);

        m_modules = Trim(output);
    }

    // Properties
    {
        wxArrayString output;
        ProcUtils::SafeExecuteCommand(program + " --help-property-list", output);

        // First line is CMake version
        output.RemoveAt(0);

        m_properties = Trim(output);
    }

    // Variables
    {
        wxArrayString output;
        ProcUtils::SafeExecuteCommand(program + " --help-variable-list", output);

        // First line is CMake version
        output.RemoveAt(0);

        m_variables = Trim(output);
    }

    // Copyright
    {
        wxArrayString output;
        ProcUtils::SafeExecuteCommand(program + " --copyright", output);

        // First line is CMake version
        output.RemoveAt(0);

        m_copyright = Join(output);
    }

    // Parse data from CMake manual page
    ParseCMakeManPage();

#if DEPRECATED
    // Generators
    {
        // TODO improve

        m_generators.clear();

        wxArrayString output;
        ProcUtils::SafeExecuteCommand(program + " --help", output);

        // Find line with "Generators"
        for (size_t i = 0; i < output.size(); ++i)
        {
            wxString line = output[i];
            // Remove whitespaces
            line.Trim().Trim(false);

            // Generators found
            if (line != "Generators")
                continue;

            // One with Generators, one empty line and one with
            // some bla bla bla...
            i += 3;

            // Foreach remaining lines
            for (; i < output.size(); ++i)
            {
                line = output[i];

                // Try to find separator
                size_t pos = line.find('=');

                // Not found. Is on the next line.
                if (pos == wxString::npos)
                {
                    ++i;
                    line += output[i];
                    pos = line.find('=');
                }

                // Ends with dot.
                if (line.Last() != '.')
                {
                    ++i;
                    line += " " + output[i];
                }

                // Get only generator name
                wxString generator = line.substr(0, pos);
                generator.Trim().Trim(false);

                // Store generator name
                if (!generator.IsEmpty())
                    m_generators.push_back(generator);
            }

            break;
        }
    }
#endif
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
        line != output.end(); ++line)
    {
        // Generators
        if (line->StartsWith(".SH GENERATORS"))
        {
            m_generators = ParseManGenerators(line);
        }
    }
}

/* ************************************************************************ */
