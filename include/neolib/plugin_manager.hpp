// plugin_manager.hpp - v1.0
/*
 *  Copyright (c) 2007-present, Leigh Johnston.
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
#include <memory>
#include <boost/filesystem.hpp>
#include "observable.hpp"
#include "vector.hpp"
#include "module.hpp"
#include "reference_counted.hpp"
#include "i_plugin_manager.hpp"
#include "i_application.hpp"

namespace neolib
{
	class plugin_manager : public reference_counted<i_plugin_manager>, private observable<i_plugin_manager::i_subscriber>
	{
		// types
	private:
		typedef std::map<uuid, std::unique_ptr<module>> module_list;
		typedef vector<i_plugin*> plugin_list;
		// construction
	public:
		plugin_manager(i_application& aApplication, const std::string& aPluginFolder);
		~plugin_manager();
		// implementation
	public:
		// from i_discoverable
		virtual bool discover(const uuid& aId, void*& aObject);
		// from i_plugin_manager
		virtual bool load_plugins();
		virtual bool load_plugin(const i_string& aPluginPath);
		virtual void enable_plugin(i_plugin& aPlugin, bool aEnable);
		virtual bool plugin_enabled(const i_plugin& aPlugin) const;
		virtual void unload_plugins();
		virtual const i_vector<i_plugin*>& plugins() const;
		virtual i_plugin* find_plugin(const uuid& aId) const;
		virtual bool open_uri(const i_string& aUri);
	public:
		virtual void subscribe(i_subscriber& aObserver);
		virtual void unsubscribe(i_subscriber& aObserver);
		// implementation
		// from observable<i_plugin_manager::i_subscriber>
	private:
		virtual void notify_observer(observer_type& aObserver, notify_type aType, const void* aParameter, const void* aParameter2);
		// own
	private:
		i_plugin* create_plugin(const i_string& aPluginPath);
		// attributes
	private:
		i_application& iApplication;
		string iPluginFolder;
		module_list iModules;
		plugin_list iPlugins;
		bool iInitializing;
	};
}
