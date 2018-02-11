// i_plugin_manager.hpp - v1.0
/*
 *  Copyright (c) 2007-present, Leigh Johnston.  All Rights Reserved.
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
#include "uuid.hpp"
#include "i_vector.hpp"
#include "string.hpp"
#include "i_discoverable.hpp"
#include "i_plugin.hpp"

namespace neolib
{
	class i_plugin_manager : public i_discoverable
	{
		// types
	public:
		typedef i_vector<i_plugin*> plugin_list;
		class i_subscriber
		{
		public:
			virtual void plugin_loaded(i_plugin& aPlugin) = 0;
			virtual void plugin_unloaded(i_plugin& aPlugin) = 0;
		public:
			enum notify_type
			{
				NotifyPluginLoaded,
				NotifyPluginUnloaded
			};
		};
		// exceptions
	public:
		template <typename Base>
		struct plugin_exception : Base { plugin_exception(const char* aMessage) : Base(aMessage) {} };
		// operations
	public:
		virtual bool load_plugins() = 0;
		virtual bool load_plugin(const i_string& aPluginPath) = 0;
		virtual void enable_plugin(i_plugin& aPlugin, bool aEnable) = 0;
		virtual bool plugin_enabled(const i_plugin& aPlugin) const = 0;
		virtual void unload_plugins() = 0;
		virtual const plugin_list& plugins() const = 0;
		virtual i_plugin* find_plugin(const uuid& aId) const = 0;
		virtual bool open_uri(const i_string& aUri) = 0;
	public:
		virtual void subscribe(i_subscriber& aObserver) = 0;
		virtual void unsubscribe(i_subscriber& aObserver) = 0;
	};
}
