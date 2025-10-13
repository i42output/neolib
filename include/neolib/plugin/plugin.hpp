/*
  plugin.hpp

  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.

  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <neolib/neolib.hpp>
#include <neolib/app/i_application.hpp>
#include <neolib/app/module.hpp>
#include <neolib/app/version.hpp>
#include <neolib/plugin/i_plugin.hpp>

namespace neolib
{
    template <typename Interface = i_plugin>
    class plugin : public reference_counted<Interface>
    {
    public:
        using abstract_type = Interface;
    public:
        i_module_services& module_services() const final
        {
            return neolib::module_services();
        }
        bool loaded() const final
        {
            return iLoaded;
        }
        bool initialized() const final
        {
            return iInitialized;
        }
        bool enabled() const final
        {
            return iEnabled;
        }
        bool load() override
        {
            iLoaded = true;
            return true;
        }
        bool initialize() override
        {
            iInitialized = true;
            return true;
        }
        void enable(bool aEnabled) override
        {
            iEnabled = aEnabled;
        }
        bool unload() override
        {
            iLoaded = false;
            return true;
        }
        bool open_uri(const i_string& aUri) override
        {
            return false;
        }
    private:
        bool iLoaded = false;
        bool iInitialized = false;
        bool iEnabled = true;
    };

    template <typename T, typename Interface = i_plugin>
    class simple_plugin : public plugin<Interface>
    {
        using base_type = plugin<Interface>;
    public:
        using abstract_type = Interface;
        using value_type = T;
        using abstract_value_type = typename value_type::abstract_type;
    public:
        simple_plugin(
            i_application& aApplication,
            uuid const& aId = value_type::plugin_id(),
            std::string const& aName = value_type::plugin_name(),
            std::string const& aDescription = value_type::plugin_description(),
            i_version const& aVersion = value_type::plugin_version(),
            std::string const& aCopyright = value_type::plugin_copyright()) :
            iApplication{ aApplication },
            iId{ aId },
            iName{ aName },
            iDescription{ aDescription },
            iVersion{ aVersion },
            iCopyright{ aCopyright }
        {
        }
    public:
        bool discover(uuid const& aId, void*& aObject) override
        {
            if (aId == abstract_value_type::iid())
            {
                if (!iContents)
                    iContents = neolib::make_ref<value_type>(iApplication, "file:///" + boost::dll::this_line_location().string());
                aObject = &*iContents;
                return true;
            }
            return false;
        }
    public:
        const uuid& id() const final
        {
            return iId;
        }
        const i_string& name() const final
        {
            return iName;
        }
        const i_string& description() const final
        {
            return iDescription;
        }
        const i_version& version() const final
        {
            return iVersion;
        }
        const i_string& copyright() const final
        {
            return iCopyright;
        }
        bool unload() override
        {
            iContents.reset();
            return base_type::unload();
        }
    private:
        i_application& iApplication;
        uuid iId;
        string iName;
        string iDescription;
        neolib::version iVersion;
        string iCopyright;
        ref_ptr<value_type> iContents;
    };
}