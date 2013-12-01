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
#include "CMakeBuilder.hpp"

// wxWidgets
#include <wx/dir.h>

// Codelite
#include "dirsaver.h"

// CMakePlugin
#include "CMakePlugin.hpp"
#include "CMakeOutput.hpp"
#include "CMakeProjectSettings.hpp"

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeBuilder::CMakeBuilder(CMakePlugin* plugin)
    : m_plugin(plugin)
    , m_asyncCmd(NULL)
{
    wxASSERT(m_plugin);
}

/* ************************************************************************ */

wxString
CMakeBuilder::CreateConfigureCmd(const wxString& cmake,
                                 const wxString& sourceDir,
                                 const wxString& buildDir,
                                 const wxArrayString& args)
{
    /// cd $buildDir && $cmake $args $sourceDir

    // Build cmd
    //return "cd \"" + buildDir + "\" && " + cmake + " " << wxJoin(args, ' ') << " " << sourceDir;
    return cmake + " " << wxJoin(args, ' ', '\0') << " " << sourceDir;
}

/* ************************************************************************ */

wxString
CMakeBuilder::CreateConfigureCmd(const wxString& cmake, const CMakeProjectSettings& settings)
{
    const wxString sourceDir = settings.sourceDirectory;
    const wxString buildDir = settings.buildDirectory;
    wxArrayString args;

    // Generator
    if (!settings.generator.IsEmpty())
        args.Add("-G \"" + settings.generator + "\"");

    // Build Type
    if (!settings.buildType.IsEmpty())
        args.Add("-DCMAKE_BUILD_TYPE=" + settings.buildType);

    // Copy additional arguments
    for (wxArrayString::const_iterator it = settings.arguments.begin(),
        ite = settings.arguments.end(); it != ite; ++it)
    {
        args.Add(*it);
    }

    return CreateConfigureCmd(cmake, sourceDir, buildDir, args);
}

/* ************************************************************************ */

wxString
CMakeBuilder::CreateBuildCmd(const wxString& make,
                             const wxString& buildDir,
                             const wxString& target)
{
    /// cd $buildDir && $make

    // Build cmd
    //return "@cd \"" + buildDir + "\" && " + make + " " + target;
    return make + " " + target;
}

/* ************************************************************************ */

wxString
CMakeBuilder::CreateBuildCmd(const wxString& make,
                             const CMakeProjectSettings& settings,
                             const wxString& target)
{
    return CreateBuildCmd(make, settings.buildDirectory, target);
}

/* ************************************************************************ */

void
CMakeBuilder::Configure(const wxString& cmake,
                        const wxString& sourceDir,
                        const wxString& buildDir,
                        const wxArrayString& args)
{
    // Create directory if missing
    if (!wxDir::Exists(buildDir))
        wxDir::Make(buildDir);

    {
        DirSaver dir;

        wxSetWorkingDirectory(wxGetCwd() + "/" + buildDir);

        // Run, Forrest, run!
        Run(CreateConfigureCmd(cmake, sourceDir, buildDir, args));
    }
}

/* ************************************************************************ */

void
CMakeBuilder::Build(const wxString& make, const wxString& buildDir,
                    const wxString& target)
{
    // TODO detect if configuration wasn't performed

    {
        DirSaver dir;

        wxSetWorkingDirectory(wxGetCwd() + "/" + buildDir);

        // Build project
        Run(CreateBuildCmd(make, buildDir, target));
    }
}

/* ************************************************************************ */

void
CMakeBuilder::Run(const wxString& command)
{
    // Create on-demand
    if (!m_asyncCmd)
        m_asyncCmd.reset(new AsyncExeCmd(m_plugin->GetOutput()));

    wxASSERT(m_asyncCmd);

    // Command is busy.
    if (m_asyncCmd->IsBusy())
        return;

    // Set command
    // Execution returns -1 with info (in debug) that there are
    // no child processes and only on my desktop PC with i7 (maybe
    // is too fast) - it does the wxExecute function call.
    m_asyncCmd->Execute(command, false, true);

    if (m_asyncCmd->GetProcess()) {
        m_asyncCmd->GetProcess()->Bind(wxEVT_END_PROCESS,
                                       &CMakeBuilder::OnProcessEnd, this);
    }
}

/* ************************************************************************ */

void
CMakeBuilder::OnProcessEnd(wxProcessEvent& event)
{
    m_asyncCmd->ProcessEnd(event);
    m_asyncCmd->GetProcess()->Unbind(wxEVT_END_PROCESS, &CMakeBuilder::OnProcessEnd, this);

#ifdef __WXMSW__
    // On Windows, sometimes killing the DOS Windows, does not kill the children
    m_asyncCmd->Terminate();
#endif
}

/* ************************************************************************ */
