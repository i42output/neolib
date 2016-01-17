// plugin_manager.hpp - v1.0
/*
 *  Copyright (c) 2014 Leigh Johnston.
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
#include <map>
#include <boost/filesystem.hpp>
#include "observable.hpp"
#include "vector.hpp"
#include "module.hpp"
#include "reference_counted.hpp"
#include "i_plugin_manager.hpp"
#include "i_application.hpp"

namespace neolib
{
	typedef void(*entry_point)(i_application&, const i_string&, i_plugin*&);

	class plugin_manager : public reference_counted<i_plugin_manager>, private observable<i_plugin_manager::i_subscriber>
	{
		// types
	private:
		typedef std::map<uuid, module*> module_list;
		typedef vector<i_plugin*> plugin_list;
		// construction
	public:
		plugin_manager(i_application& aApplication, const std::string& aPluginFolder) :
			iApplication(aApplication), iPluginFolder(aPluginFolder)
		{
		}
		~plugin_manager()
		{
			unload_plugins();
		}
		// implementation
	public:
		// from i_discoverable
		virtual bool discover(const uuid& aId, void*& aObject)
		{
			return false;
		}
		// from i_plugin_manager
		virtual bool load_plugins()
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
		virtual bool load_plugin(const i_string& aPluginPath)
		{
			bool loaded = false;
			i_plugin* newPlugin = create_plugin(aPluginPath);
			if (newPlugin != 0)
				loaded = newPlugin->load();
			if (loaded)
				notify_observers(i_subscriber::NotifyPluginLoaded, *newPlugin);
			return loaded;
		}
		virtual void enable_plugin(i_plugin& aPlugin, bool aEnable)
		{
		}
		virtual bool plugin_enabled(const i_plugin& aPlugin) const
		{
			return true;
		}
		virtual void unload_plugins()
		{
			for (plugin_list::const_iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
				(*i)->release();
			for (module_list::const_iterator i = iModules.begin(); i != iModules.end(); ++i)
			{
				(*i).second->unload();
				notify_observers(i_subscriber::NotifyPluginUnloaded, *(*i).second);
				delete (*i).second;
			}
			iPlugins.clear();
			iModules.clear();
		}
		virtual const i_vector<i_plugin*>& plugins() const
		{
			return iPlugins;
		}
		virtual i_plugin* find_plugin(const uuid& aId) const
		{
			for (plugin_list::const_iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
				if ((*i)->id() == aId)
					return (*i);
			return 0;
		}
		virtual bool open_uri(const i_string& aUri)
		{
			for (plugin_list::const_iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
				if ((*i)->open_uri(aUri))
					return true;
			return false;
		}
	public:
		virtual void subscribe(i_subscriber& aObserver)
		{
			add_observer(aObserver);
			for (plugin_list::const_iterator i = iPlugins.begin(); i != iPlugins.end(); ++i)
				if ((*i)->loaded())
					aObserver.plugin_loaded(**i);
		}
		virtual void unsubscribe(i_subscriber& aObserver)
		{
			remove_observer(aObserver);
		}
		// implementation
		// from observable<i_plugin_manager::i_subscriber>
	private:
		virtual void notify_observer(observer_type& aObserver, notify_type aType, const void* aParameter, const void* aParameter2)
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
		// own
	private:
		i_plugin* create_plugin(const i_string& aPluginPath)
		{
			std::unique_ptr<module> pm(new module(aPluginPath.to_std_string()));
			i_plugin* newPlugin = 0;
			try
			{
				if (!pm->load())
					return 0;
				entry_point entryPoint = static_cast<entry_point>(pm->procedure("entry_point"));
				if (entryPoint == 0)
					return 0;
				entryPoint(iApplication, string(boost::filesystem::path(aPluginPath.to_std_string()).parent_path().generic_string()), newPlugin);
				if (newPlugin == 0)
					return 0;
				iPlugins.push_back(newPlugin);
				iModules[newPlugin->id()] = pm.release();
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

		// attributes
	private:
		i_application& iApplication;
		string iPluginFolder;
		module_list iModules;
		plugin_list iPlugins;
		bool iInitializing;
	};
}
