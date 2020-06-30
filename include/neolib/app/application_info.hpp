// application_info.hpp
/*
 *  Copyright (c) 2007 Leigh Johnston.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of Leigh Johnston nor the names of any
 *       other contributors to this software may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <neolib/neolib.hpp>
#include <boost/filesystem.hpp>
#include <neolib/core/string_ci.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/string.hpp>
#include <neolib/app/version.hpp>
#include <neolib/app/i_application_info.hpp>

namespace neolib
{
    std::string settings_folder(const std::string& aApplicationName, const std::string& aCompanyName);

    class program_arguments : public i_program_arguments
    {
    public:
        program_arguments(int argc, char* argv[]) :
            iArgc{ argc }, iArgv{ argv }
        {
            for (auto arg = 0; arg < argc; ++arg)
                iArguments.push_back(neolib::string{ argv[arg] });
        }
        program_arguments(const i_program_arguments& aOther) :
            iArgc{ aOther.argc() }, iArgv{ aOther.argv() }, iArguments{ aOther.as_vector() }
        {
        }
    public:
        int argc() const override
        {
            return iArgc;
        }
        char** argv() const override
        {
            return iArgv;
        }
        const vector<string>& as_vector() const override
        {
            return iArguments;
        }
    private:
        int iArgc;
        char** iArgv;
        vector<string> iArguments;
    };

    class NEOLIB_EXPORT application_info : public i_application_info
    {
    public:
        application_info(
            int argc, char* argv[],
            const std::string& aName = {},
            const std::string& aCompany = {},
            const neolib::version& aVersion = neolib::version{},
            const std::string& aCopyright = {},
            const std::string& aApplicationFolder = std::string{},
            const std::string& aSettingsFolder = std::string{},
            const std::string& aDataFolder = std::string{},
            const std::string& aPluginExtension = ".plg") :
            application_info
            {
                program_arguments{ argc, argv },
                aName,
                aCompany,
                aVersion,
                aCopyright,
                aApplicationFolder,
                aSettingsFolder,
                aDataFolder,
                aPluginExtension
            } {}
        application_info(
            const program_arguments& aArguments,
            const std::string& aName = {},
            const std::string& aCompany = {},
            const neolib::version& aVersion = neolib::version{},
            const std::string& aCopyright = {},
            const std::string& aApplicationFolder = std::string{},
            const std::string& aSettingsFolder = std::string{},
            const std::string& aDataFolder = std::string{},
            const std::string& aPluginExtension = ".plg") :
            iArguments{ aArguments },
            iName{ aName },
            iCompany{ aCompany },
            iVersion{ aVersion },
            iCopyright{ aCopyright },
            iApplicationFolder{ aApplicationFolder },
            iSettingsFolder{ aSettingsFolder },
            iDataFolder{ aDataFolder },
            iPluginExtension{ aPluginExtension },
            iRemovable{ false }
        {
            if (std::find(std::next(iArguments.as_vector().container().begin()), iArguments.as_vector().container().end(), neolib::ci_string("/pocket")) != iArguments.as_vector().container().end() ||
                std::find(std::next(iArguments.as_vector().container().begin()), iArguments.as_vector().container().end(), neolib::ci_string("-pocket")) != iArguments.as_vector().container().end() ||
                std::find(std::next(iArguments.as_vector().container().begin()), iArguments.as_vector().container().end(), neolib::ci_string("/removable")) != iArguments.as_vector().container().end() ||
                std::find(std::next(iArguments.as_vector().container().begin()), iArguments.as_vector().container().end(), neolib::ci_string("-removable")) != iArguments.as_vector().container().end())
            {
                iRemovable = true;
            }
        }
        application_info(const i_application_info& aOther) :
            iArguments{ aOther.arguments() },
            iName{ aOther.name() },
            iCompany{ aOther.company() },
            iVersion{ aOther.version() },
            iCopyright{ aOther.copyright() },
            iApplicationFolder{ aOther.application_folder() },
            iSettingsFolder{ aOther.settings_folder() },
            iDataFolder{ aOther.data_folder() },
            iPluginExtension{ aOther.plugin_extension() },
            iRemovable{ aOther.removable() }
        {
        }

    public:
        const program_arguments& arguments() const override 
        { 
            return iArguments; 
        }
        const i_string& name() const override 
        { 
            return iName; 
        }
        const i_string& company() const override 
        { 
            return iCompany; 
        }
        const i_version& version() const override 
        { 
            return iVersion; 
        }
        const i_string& copyright() const override 
        { 
            return iCopyright; 
        }
        const i_string& application_folder(bool aUseDefault = true) const override
        { 
            if (iApplicationFolder.empty() && aUseDefault)
            {
                if (iDefaultApplicationFolder.empty())
                    iDefaultApplicationFolder = boost::filesystem::current_path().generic_string();
                return iDefaultApplicationFolder;
            }
            return iApplicationFolder;
        }
        const i_string& settings_folder(bool aUseDefault = true) const override
        { 
            if (iSettingsFolder.empty() && aUseDefault)
            {
                if (iDefaultSettingsFolder.empty())
                    iDefaultSettingsFolder = neolib::settings_folder(name(), company());
                return iDefaultSettingsFolder;
            }
            return iSettingsFolder;
        }
        const i_string& data_folder(bool aUseDefault = true) const override
        { 
            if (iDataFolder.empty() && aUseDefault)
            {
                if (iDefaultDataFolder.empty())
                    iDefaultDataFolder = settings_folder();
                return iDefaultDataFolder;
            }
            return iDataFolder;
        }
        const i_string& plugin_extension() const override 
        { 
            return iPluginExtension; 
        }
        bool removable() const override
        {
            return iRemovable;
        }

    private:
        program_arguments iArguments;
        string iName;
        string iCompany;
        neolib::version iVersion;
        string iCopyright;
        mutable string iDefaultApplicationFolder;
        mutable string iDefaultSettingsFolder;
        mutable string iDefaultDataFolder;
        string iApplicationFolder;
        string iSettingsFolder;
        string iDataFolder;
        string iPluginExtension;
        bool iRemovable;
    };
}
