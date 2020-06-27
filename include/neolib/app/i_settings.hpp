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
#include <neolib/core/i_reference_counted.hpp>
#include <neolib/core/uuid.hpp>
#include <neolib/core/i_string.hpp>
#include <neolib/plugin/simple_variant.hpp>
#include <neolib/plugin/i_plugin_event.hpp>
#include <neolib/app/setting.hpp>
#include <neolib/app/setting_constraints.hpp>

namespace neolib
{
    class i_settings : public i_reference_counted
    {
        template <typename T>
        friend class setting;
    public:
        declare_event(settings_changed, const i_string&)
        declare_event(setting_changed, const i_setting&)
        declare_event(setting_deleted, const i_setting&)
        declare_event(interested_in_dirty_settings, bool&)
    public:
        struct setting_already_registered : std::logic_error { setting_already_registered() : std::logic_error("i_settings::setting_already_registered") {} };
        struct category_not_found : std::logic_error { category_not_found() : std::logic_error("i_settings::category_not_found") {} };
        struct group_not_found : std::logic_error { group_not_found() : std::logic_error("i_settings::group_not_found") {} };
        struct setting_not_found : std::logic_error { setting_not_found() : std::logic_error("i_settings::setting_not_found") {} };
    public:
        virtual void register_category(i_string const& aCategorySubkey, i_string const& aCategoryTitle = string{}) = 0;
        virtual void register_group(i_string const& aGroupSubkey, i_string const& aGroupTitle = string{}) = 0;
        virtual void register_setting(i_setting& aSetting) = 0;
        virtual std::size_t category_count() const = 0;
        virtual i_string const& category(std::size_t aCategoryIndex) const = 0;
        virtual i_string const& category_title(i_string const& aCategorySubkey) const = 0;
        virtual std::size_t group_count(i_string const& aCategorySubkey) const = 0;
        virtual i_string const& group(i_string const& aCategorySubkey, std::size_t aGroupIndex) const = 0;
        virtual i_string const& group_title(i_string const& aGroupSubkey) const = 0;
        virtual std::size_t setting_count() const = 0;
        virtual i_setting& setting(std::size_t aSettingIndex) = 0;
        virtual i_setting& setting(i_string const& aKey) = 0;
        virtual void change_setting(i_setting& aExistingSetting, i_setting_value const& aValue, bool aApplyNow = true) = 0;
        virtual void delete_setting(i_setting& aExistingSetting) = 0;
        virtual void apply_changes() = 0;
        virtual void discard_changes() = 0;
        virtual bool dirty() const = 0;
    public:
        virtual void load() = 0;
        virtual void save() const = 0;
    public:
        static uuid const& id() { static uuid sId = neolib::make_uuid("E19B3C48-04F7-4207-B24A-2967A3523CE7"); return sId; }
    private:
        virtual void setting_changed(i_setting& aExistingSetting) = 0;
        // helpers
    public:
        template <typename T>
        i_setting& register_setting(string const& aKey, setting_constraints<T> const& aSettingConstraints, std::optional<T> const& aDefaultValue = {}, string const& aFormat = {})
        {
            auto newSetting = make_ref<setting<T>>(*this, aKey, aSettingConstraints, aFormat);
            register_setting(*newSetting);
            if (!newSetting->value().is_set() && aDefaultValue)
                change_setting(*newSetting, *aDefaultValue, false);
            return *newSetting;
        }
        template <typename T>
        i_setting& register_setting(string const& aKey, std::optional<T> const& aDefaultValue = {}, string const& aFormat = {})
        {
            return register_setting(aKey, {}, aDefaultValue, aFormat);
        }
        template <typename T>
        void change_setting(i_setting& aExistingSetting, const T& aValue, bool aApplyNow = true)
        {
            change_setting(aExistingSetting, static_cast<i_setting_value const&>(setting_value{ aValue }), aApplyNow);
        }
    };
}
