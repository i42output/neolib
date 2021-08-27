// i_settings.hpp
/*
 *  Copyright (c) 2007, 2020 Leigh Johnston.
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
#include <neolib/core/vector.hpp>
#include <neolib/core/map.hpp>
#include <neolib/core/i_reference_counted.hpp>
#include <neolib/core/uuid.hpp>
#include <neolib/core/i_string.hpp>
#include <neolib/plugin/simple_variant.hpp>
#include <neolib/plugin/i_plugin_event.hpp>
#include <neolib/app/i_setting.hpp>
#include <neolib/app/setting_constraints.hpp>

namespace neolib
{
    template <typename T>
    class setting;

    template <typename T>
    struct as_setting
    {
        typedef T type;
    };

    template <typename T>
    using as_setting_t = typename as_setting<T>::type;

    class i_settings : public i_reference_counted
    {
    public:
        declare_event(setting_changing, const i_setting&)
        declare_event(setting_changed, const i_setting&)
        declare_event(setting_deleted, const i_setting&)
        declare_event(settings_changed, const i_string&)
    public:
        struct setting_already_registered : std::logic_error { setting_already_registered() : std::logic_error("i_settings::setting_already_registered") {} };
        struct category_not_found : std::logic_error { category_not_found() : std::logic_error("i_settings::category_not_found") {} };
        struct group_not_found : std::logic_error { group_not_found() : std::logic_error("i_settings::group_not_found") {} };
        struct setting_not_found : std::logic_error { setting_not_found() : std::logic_error("i_settings::setting_not_found") {} };
    public:
        virtual void register_category(i_string const& aCategorySubkey, i_string const& aCategoryTitle = string{}) = 0;
        virtual void register_group(i_string const& aGroupSubkey, i_string const& aGroupTitle = string{}) = 0;
        virtual void register_setting(i_setting& aSetting) = 0;
        virtual i_map<i_string, i_string> const& all_categories() const = 0;
        virtual i_string const& category_title(i_string const& aCategorySubkey) const = 0;
        virtual i_map<i_string, i_map<i_string, i_string>> const& all_groups() const = 0;
        virtual i_string const& group_title(i_string const& aGroupSubkey) const = 0;
        virtual i_map<i_string, i_ref_ptr<i_setting>> const& all_settings() const = 0;
        virtual i_vector<i_ref_ptr<i_setting>> const& all_settings_ordered() const = 0;
        virtual i_setting const& setting(i_string const& aKey) const = 0;
        virtual i_setting& setting(i_string const& aKey) = 0;
        virtual void change_setting(i_setting& aExistingSetting, i_setting_value const& aValue, bool aApplyNow = true) = 0;
        virtual void delete_setting(i_setting& aExistingSetting) = 0;
        virtual void apply_changes() = 0;
        virtual void discard_changes() = 0;
        virtual bool modified() const = 0;
    public:
        virtual void register_friendly_text(i_setting const& aSetting, i_string const& aText, i_string const& aFriendlyText) = 0;
        virtual i_string const& friendly_text(i_setting const& aSetting, i_string const& aText) const = 0;
    public:
        virtual void load() = 0;
        virtual void save() const = 0;
    public:
        virtual void changing_setting(i_setting const& aSetting) = 0;
        virtual void changed_setting(i_setting const& aSetting) = 0;
    public:
        static uuid const& iid() { static uuid sId = neolib::make_uuid("E19B3C48-04F7-4207-B24A-2967A3523CE7"); return sId; }
        // helpers
    public:
        void register_category(string const& aCategorySubkey, string const& aCategoryTitle = string{})
        {
            register_category(static_cast<i_string const&>(aCategorySubkey), static_cast<i_string const&>(aCategoryTitle));
        }
        void register_group(string const& aGroupSubkey, string const& aGroupTitle = string{})
        {
            register_group(static_cast<i_string const&>(aGroupSubkey), static_cast<i_string const&>(aGroupTitle));
        }
        template <typename T>
        i_setting& register_setting(string const& aKey, T const& aDefaultValue, setting_constraints<as_setting_t<T>> const& aSettingConstraints, string const& aFormat = {})
        {
            auto newSetting = make_ref<neolib::setting<as_setting_t<T>>>(*this, aKey, aDefaultValue, aSettingConstraints, aFormat);
            register_setting(*newSetting);
            return *newSetting;
        }
        template <typename T>
        i_setting& register_setting(string const& aKey, T const& aDefaultValue, string const& aFormat = {})
        {
            return register_setting(aKey, aDefaultValue, {}, aFormat);
        }
        template <typename T>
        void change_setting(i_setting& aExistingSetting, T const& aValue, bool aApplyNow = true)
        {
            change_setting(aExistingSetting, static_cast<i_setting_value const&>(setting_value<as_setting_t<T>>{ aValue }), aApplyNow);
        }
    };
}
