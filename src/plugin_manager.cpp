// plugin_manager.cpp
/*
 *  Copyright (c) 2014-2016 Leigh Johnston.
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

#include "neolib.hpp"
#include "plugin_manager.hpp"

namespace neolib
{
	typedef void(*entry_point)(i_application&, const i_string&, i_plugin*&);

	plugin_manager::plugin_manager(i_application& aApplication, const std::string& aPluginFolder) :
		iApplication(aApplication), iPluginFolder(aPluginFolder)
	{
	}

	plugin_manager::~plugin_manager()
	{
		unload_plugins();
	}

	bool plugin_manager::discover(const uuid& aId, void*& aObject)
	{
		return false;
	}

	bool plugin_manager::load_plugins()
	{
		if (!boost::filesystem::is_directory(iPluginFolder.to_std_string()))
			return false;
		for (boost::filesystem::recursive_directory_iterator i(iPluginFolder.to_std_string()); i != boost::filesystem::recursive_directory_iterator(); ++i)
		{
			if (i->path().extension() != ".plg")
				continue;
			create_plugin(string(i->path().generic_string()));
		}
		for (plugin_list::const_iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
			if (!(*i)->loaded())
				if ((*i)->load())
					notify_observers(i_subscriber::NotifyPluginLoaded, **i);
		return true;
	}

	bool plugin_manager::load_plugin(const i_string& aPluginPath)
	{
		bool loaded = false;
		i_plugin* newPlugin = create_plugin(aPluginPath);
		if (newPlugin != 0)
			loaded = newPlugin->load();
		if (loaded)
			notify_observers(i_subscriber::NotifyPluginLoaded, *newPlugin);
		return loaded;
	}

	void plugin_manager::enable_plugin(i_plugin& aPlugin, bool aEnable)
	{
	}

	bool plugin_manager::plugin_enabled(const i_plugin& aPlugin) const
	{
		return true;
	}

	void plugin_manager::unload_plugins()
	{
		for (plugin_list::iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
			(*i)->release();
		for (module_list::iterator i = iModules.begin(); i != iModules.end(); ++i)
		{
			(*i).second->unload();
			notify_observers(i_subscriber::NotifyPluginUnloaded, *(*i).second);
			(*i).second.reset();
		}
		iPlugins.clear();
		iModules.clear();
	}

	const i_vector<i_plugin*>& plugin_manager::plugins() const
	{
		return iPlugins;
	}
	
	i_plugin* plugin_manager::find_plugin(const uuid& aId) const
	{
		for (plugin_list::const_iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
			if ((*i)->id() == aId)
				return (*i);
		return 0;
	}

	bool plugin_manager::open_uri(const i_string& aUri)
	{
		for (plugin_list::const_iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
			if ((*i)->open_uri(aUri))
				return true;
		return false;
	}

	void plugin_manager::subscribe(i_subscriber& aObserver)
	{
		add_observer(aObserver);
		for (plugin_list::const_iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
			if ((*i)->loaded())
				aObserver.plugin_loaded(**i);
	}

	void plugin_manager::unsubscribe(i_subscriber& aObserver)
	{
		remove_observer(aObserver);
	}

	void plugin_manager::notify_observer(observer_type& aObserver, notify_type aType, const void* aParameter, const void* aParameter2)
	{
		switch (aType)
		{
		case i_subscriber::NotifyPluginLoaded:
			aObserver.plugin_loaded(*static_cast<i_plugin*>(const_cast<void*>(aParameter)));
			break;
		case i_subscriber::NotifyPluginUnloaded:
			aObserver.plugin_unloaded(*static_cast<i_plugin*>(const_cast<void*>(aParameter)));
			break;
		}
	}

	i_plugin* plugin_manager::create_plugin(const i_string& aPluginPath)
	{
		auto pm = std::make_unique<module>(aPluginPath.to_std_string());
		i_plugin* newPlugin = 0;
		try
		{
			if (!pm->load())
				return 0;
			entry_point entryPoint = pm->procedure<entry_point>("entry_point");
			if (entryPoint == 0)
				return 0;
			entryPoint(iApplication, string(boost::filesystem::path(aPluginPath.to_std_string()).parent_path().generic_string()), newPlugin);
			if (newPlugin == 0)
				return 0;
			iPlugins.push_back(newPlugin);
			iModules[newPlugin->id()] = std::move(pm);
		}
		catch (const std::exception& e)
		{
			throw plugin_exception<std::runtime_error>(e.what());
		}
		catch (...)
		{
			throw plugin_exception<std::runtime_error>("Unknown exception");
		}
		return newPlugin;
	}
}
