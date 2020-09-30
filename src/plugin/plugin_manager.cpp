// plugin_manager.cpp
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
#include <boost/dll.hpp>
#include <neolib/plugin/plugin_manager.hpp>

namespace neolib
{
    typedef void(*entry_point)(i_application&, const i_string&, i_ref_ptr<i_plugin>&);

    plugin_manager::plugin_manager(i_application& aApplication) :
        iApplication{ aApplication }
    {
        iPluginFileExtensions.push_back(string{ aApplication.info().plugin_extension() });
        std::set<std::string> folders;
        folders.insert(boost::filesystem::canonical(boost::filesystem::path{ aApplication.info().application_folder().to_std_string() }).string());
        folders.insert(boost::filesystem::canonical(boost::dll::program_location().parent_path()).string());
        for (auto const& folder : folders)
            iPluginFolders.push_back(string{ folder });
    }

    plugin_manager::~plugin_manager()
    {
        unload_plugins();
    }

    bool plugin_manager::discover(const uuid&, void*&)
    {
        return false;
    }

    const plugin_manager::plugin_file_extensions_t& plugin_manager::plugin_file_extensions() const
    {
        return iPluginFileExtensions;
    }

    plugin_manager::plugin_file_extensions_t& plugin_manager::plugin_file_extensions()
    {
        return iPluginFileExtensions;
    }

    const plugin_manager::plugin_folders_t& plugin_manager::plugin_folders() const
    {
        return iPluginFolders;
    }

    plugin_manager::plugin_folders_t& plugin_manager::plugin_folders()
    {
        return iPluginFolders;
    }

    bool plugin_manager::load_plugins()
    {
        for (auto const& folder : plugin_folders())
        {
            if (!boost::filesystem::is_directory(folder.to_std_string()))
                continue;
            for (boost::filesystem::recursive_directory_iterator i(folder.to_std_string()); i != boost::filesystem::recursive_directory_iterator(); ++i)
                if (find(plugin_file_extensions().begin(), plugin_file_extensions().end(), i->path().extension().string()) != plugin_file_extensions().end())
                    create_plugin(string(i->path().generic_string()));
        }
        bool gotSome = false;
        for (plugins_t::const_iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
            if (!(*i)->loaded())
                if ((*i)->load())
                {
                    gotSome = true;
                    PluginLoaded.trigger(**i);
                }
        return gotSome;
    }

    bool plugin_manager::load_plugin(const i_string& aPluginPath)
    {
        bool loaded = false;
        auto const& newPlugin = create_plugin(aPluginPath);
        if (newPlugin != nullptr)
            loaded = newPlugin->load();
        if (loaded)
            PluginLoaded.trigger(*newPlugin);
        return loaded;
    }

    void plugin_manager::enable_plugin(i_plugin& aPlugin, bool aEnable)
    {
        /* todo */
        (void)aPlugin;
        (void)aEnable;
    }

    bool plugin_manager::plugin_enabled(const i_plugin& aPlugin) const
    {
        /* todo */
        (void)aPlugin;
        return true;
    }

    void plugin_manager::unload_plugins()
    {
        {
            auto pluginsToUnload = std::move(iPlugins);
            for (auto const& p : pluginsToUnload)
                PluginUnloaded.trigger(*p);
        }
        for (modules_t::iterator i = iModules.begin(); i != iModules.end(); ++i)
        {
            (*i).second->unload();
            (*i).second.reset();
        }
        iModules.clear();
    }

    const plugin_manager::plugins_t& plugin_manager::plugins() const
    {
        return iPlugins;
    }
    
    const i_ref_ptr<i_plugin>& plugin_manager::find_plugin(const uuid& aId) const
    {
        for (plugins_t::const_iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
            if ((*i)->id() == aId)
                return *i;
        thread_local ref_ptr<i_plugin> nullref;
        nullref = nullptr;
        return nullref;
    }

    i_ref_ptr<i_plugin>& plugin_manager::find_plugin(const uuid& aId)
    {
        return const_cast<i_ref_ptr<i_plugin>&>(to_const(*this).find_plugin(aId));
    }

    bool plugin_manager::open_uri(const i_string& aUri)
    {
        for (plugins_t::const_iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
            if ((*i)->open_uri(aUri))
                return true;
        return false;
    }

    i_ref_ptr<i_plugin>& plugin_manager::create_plugin(const i_string& aPluginPath)
    {
        auto pm = std::make_unique<module>(aPluginPath.to_std_string());
        thread_local ref_ptr<i_plugin> tNewPlugin;
        tNewPlugin = nullptr;
        try
        {
            if (!pm->load())
                return tNewPlugin;
            entry_point entryPoint = pm->procedure<entry_point>("entry_point");
            if (entryPoint == nullptr)
                return tNewPlugin;
            entryPoint(iApplication, string(boost::filesystem::path(aPluginPath.to_std_string()).parent_path().generic_string()), tNewPlugin);
            if (tNewPlugin == nullptr)
                return tNewPlugin;
            iPlugins.push_back(tNewPlugin);
            auto& newPlugin = iPlugins.back();
            tNewPlugin = nullptr;
            iModules[newPlugin->id()] = std::move(pm);
            return newPlugin;
        }
        catch (const std::exception& e)
        {
            throw plugin_exception<std::runtime_error>(e.what());
        }
        catch (...)
        {
            throw plugin_exception<std::runtime_error>("Unknown exception");
        }
    }
}
