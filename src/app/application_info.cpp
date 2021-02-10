// application_info.cpp
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

#include <neolib/neolib.hpp>
#include <boost/filesystem.hpp>
#include <neolib/app/application_info.hpp>
#include <neolib/file/file.hpp>

namespace neolib
{
    std::string settings_folder(const std::string& aApplicationName, const std::string& aCompanyName)
    {
        if (aApplicationName.empty())
            throw unknown_application_name();
#ifdef _WIN32
        std::string settingsFolder = user_settings_directory();
        if (!aCompanyName.empty())
            settingsFolder += ("/" + aCompanyName);
        settingsFolder += ("/" + aApplicationName);
#else
        std::string settingsFolder = user_settings_directory() + "/." + aApplicationName;
#endif
        create_path(settingsFolder);
        return settingsFolder;
    }

    program_arguments::program_arguments() :
        iArgc{ 0 }, iArgv{ nullptr }
    {
    }

    program_arguments::program_arguments(int argc, char* argv[]) :
        iArgc{ argc }, iArgv{ argv }
    {
        for (auto arg = 0; arg < argc; ++arg)
            iArguments.push_back(neolib::string{ argv[arg] });
    }

    program_arguments::program_arguments(const i_program_arguments& aOther) :
        iArgc{ aOther.argc() }, iArgv{ aOther.argv() }, iArguments{ aOther.as_vector() }
    {
    }

    int program_arguments::argc() const
    {
        return iArgc;
    }

    char** program_arguments::argv() const
    {
        return iArgv;
    }

    const vector<string>& program_arguments::as_vector() const
    {
        return iArguments;
    }

    application_info::application_info(
        const std::string& aName,
        const std::string& aCompany,
        const neolib::version& aVersion,
        const std::string& aCopyright,
        const std::string& aApplicationFolder,
        const std::string& aSettingsFolder,
        const std::string& aDataFolder,
        const std::string& aPluginExtension) :
        application_info
        {
            program_arguments{},
            aName,
            aCompany,
            aVersion,
            aCopyright,
            aApplicationFolder,
            aSettingsFolder,
            aDataFolder,
            aPluginExtension
        } {}
    
    application_info::application_info(
        int argc, char* argv[],
        const std::string& aName,
        const std::string& aCompany,
        const neolib::version& aVersion,
        const std::string& aCopyright,
        const std::string& aApplicationFolder,
        const std::string& aSettingsFolder,
        const std::string& aDataFolder,
        const std::string& aPluginExtension) :
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
    
    application_info::application_info(
        const program_arguments& aArguments,
        const std::string& aName,
        const std::string& aCompany,
        const neolib::version& aVersion,
        const std::string& aCopyright,
        const std::string& aApplicationFolder,
        const std::string& aSettingsFolder,
        const std::string& aDataFolder,
        const std::string& aPluginExtension) :
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
        if (!arguments().as_vector().empty())
            if (std::find(std::next(arguments().as_vector().container().begin()), arguments().as_vector().container().end(), neolib::ci_string("/pocket")) != arguments().as_vector().container().end() ||
                std::find(std::next(arguments().as_vector().container().begin()), arguments().as_vector().container().end(), neolib::ci_string("-pocket")) != arguments().as_vector().container().end() ||
                std::find(std::next(arguments().as_vector().container().begin()), arguments().as_vector().container().end(), neolib::ci_string("/removable")) != arguments().as_vector().container().end() ||
                std::find(std::next(arguments().as_vector().container().begin()), arguments().as_vector().container().end(), neolib::ci_string("-removable")) != arguments().as_vector().container().end())
            {
                iRemovable = true;
            }
    }

    application_info::application_info(const i_application_info& aOther) :
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

    const program_arguments& application_info::arguments() const
    {
        return iArguments;
    }

    const i_string& application_info::name() const
    {
        return iName;
    }

    const i_string& application_info::company() const
    {
        return iCompany;
    }

    const i_version& application_info::version() const
    {
        return iVersion;
    }

    const i_string& application_info::copyright() const
    {
        return iCopyright;
    }

    const i_string& application_info::application_folder(bool aUseDefault) const
    {
        if (iApplicationFolder.empty() && aUseDefault)
        {
            if (iDefaultApplicationFolder.empty())
                iDefaultApplicationFolder = boost::filesystem::current_path().generic_string();
            return iDefaultApplicationFolder;
        }
        return iApplicationFolder;
    }

    const i_string& application_info::settings_folder(bool aUseDefault) const
    {
        if (iSettingsFolder.empty() && aUseDefault)
        {
            if (iDefaultSettingsFolder.empty())
                iDefaultSettingsFolder = neolib::settings_folder(name(), company());
            return iDefaultSettingsFolder;
        }
        return iSettingsFolder;
    }

    const i_string& application_info::data_folder(bool aUseDefault) const
    {
        if (iDataFolder.empty() && aUseDefault)
        {
            if (iDefaultDataFolder.empty())
                iDefaultDataFolder = settings_folder();
            return iDefaultDataFolder;
        }
        return iDataFolder;
    }

    const i_string& application_info::plugin_extension() const
    {
        return iPluginExtension;
    }

    bool application_info::removable() const
    {
        return iRemovable;
    }
}
