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
#include <neolib/app/version.hpp>
#include <neolib/plugin/i_plugin.hpp>

namespace neolib
{
    template <typename T>
    class plugin : public neolib::reference_counted<neolib::i_plugin>
    {
    public:
        typedef T value_type;
        typedef typename value_type::abstract_type abstract_value_type;
    public:
        plugin(
            neolib::i_application& aApplication,
            const neolib::uuid& aId = value_type::plugin_id(),
            std::string const& aName = value_type::plugin_name(),
            std::string const& aDescription = value_type::plugin_description(),
            const neolib::version& aVersion = value_type::plugin_version(),
            std::string const& aCopyright = value_type::plugin_copyright()) :
            iApplication{ aApplication },
            iId{ aId },
            iName{ aName },
            iDescription{ aDescription },
            iVersion{ aVersion },
            iCopyright{ aCopyright },
            iLoaded{ false }
        {
        }
    public:
        bool discover(const neolib::uuid& aId, void*& aObject) override
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
        const neolib::uuid& id() const override
        {
            return iId;
        }
        const neolib::i_string& name() const override
        {
            return iName;
        }
        const neolib::i_string& description() const override
        {
            return iDescription;
        }
        const neolib::i_version& version() const override
        {
            return iVersion;
        }
        const neolib::i_string& copyright() const override
        {
            return iCopyright;
        }
        bool load() override
        {
            iLoaded = true;
            return true;
        }
        bool unload() override
        {
            iContents.reset();
            iLoaded = false;
            return true;
        }
        bool loaded() const override
        {
            return iLoaded;
        }
        bool open_uri(const neolib::i_string& aUri) override
        {
            return false;
        }
    private:
        neolib::i_application& iApplication;
        neolib::uuid iId;
        neolib::string iName;
        neolib::string iDescription;
        neolib::version iVersion;
        neolib::string iCopyright;
        bool iLoaded;
        ref_ptr<value_type> iContents;
    };
}