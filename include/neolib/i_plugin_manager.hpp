// i_plugin_manager.hpp - v1.0
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
#include <neolib/uuid.hpp>
#include <neolib/i_vector.hpp>
#include <neolib/string.hpp>
#include <neolib/i_discoverable.hpp>
#include <neolib/i_plugin_event.hpp>
#include <neolib/i_plugin.hpp>

namespace neolib
{
    class i_plugin_manager : public i_discoverable
    {
        // events
    public:
        declare_event(plugin_loaded, i_plugin&)
        declare_event(plugin_unloaded, i_plugin&)
        // types
    public:
        typedef i_vector<i_string> plugin_file_extensions_t;
        typedef i_vector<i_string> plugin_folders_t;
        typedef i_vector<i_ref_ptr<i_plugin>> plugins_t;
        // exceptions
    public:
        template <typename Base>
        struct plugin_exception : Base { plugin_exception(const char* aMessage) : Base(aMessage) {} };
        // operations
    public:
        virtual const plugin_file_extensions_t& plugin_file_extensions() const = 0;
        virtual plugin_file_extensions_t& plugin_file_extensions() = 0;
        virtual const plugin_folders_t& plugin_folders() const = 0;
        virtual plugin_folders_t& plugin_folders() = 0;
        virtual bool load_plugins() = 0;
        virtual bool load_plugin(const i_string& aPluginPath) = 0;
        virtual void enable_plugin(i_plugin& aPlugin, bool aEnable) = 0;
        virtual bool plugin_enabled(const i_plugin& aPlugin) const = 0;
        virtual void unload_plugins() = 0;
        virtual const plugins_t& plugins() const = 0;
        virtual const i_ref_ptr<i_plugin>& find_plugin(const uuid& aId) const = 0;
        virtual i_ref_ptr<i_plugin>& find_plugin(const uuid& aId) = 0;
        virtual bool open_uri(const i_string& aUri) = 0;
    };
}
