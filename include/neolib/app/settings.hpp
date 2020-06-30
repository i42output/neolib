// settings.hpp
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
#include <fstream>
#include <set>
#include <memory>
#include <neolib/core/map.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/i_custom_type_factory.hpp>
#include <neolib/file/xml.hpp>
#include <neolib/plugin/plugin_event.hpp>
#include <neolib/app/i_application.hpp>
#include <neolib/app/setting.hpp>
#include <neolib/app/i_settings.hpp>

namespace neolib
{
    class NEOLIB_EXPORT settings : public reference_counted<i_settings>
    {
        template <typename T>
        friend class setting;
    public:
        define_declared_event(SettingChanging, setting_changing, const i_setting&)
        define_declared_event(SettingChanged, setting_changed, const i_setting&)
        define_declared_event(SettingDeleted, setting_deleted, const i_setting&)
        define_declared_event(SettingsChanged, settings_changed, const i_string&)
    private:
        typedef map<string, string> category_titles;
        typedef map<string, map<string, string>> group_titles;
        typedef map<string, ref_ptr<i_setting>> setting_list;
    public:
        settings(const i_string& aFileName);
        settings(const i_application& aApp, const i_string& aFileName = string{ "settings.xml" });
    public:
        using i_settings::register_category;
        using i_settings::register_group;
        using i_settings::register_setting;
        void register_category(i_string const& aCategorySubkey, i_string const& aCategoryTitle = string{}) override;
        void register_group(i_string const& aGroupSubkey, i_string const& aGroupTitle = string{}) override;
        void register_setting(i_setting& aSetting) override;
        category_titles const& all_categories() const override;
        i_string const& category_title(i_string const& aCategorySubkey) const override;
        group_titles const& all_groups() const override;
        i_string const& group_title(i_string const& aGroupSubkey) const override;
        setting_list const& all_settings() const override;
        i_setting const& setting(i_string const& aKey) const override;
        i_setting& setting(i_string const& aKey) override;
        void change_setting(i_setting& aExistingSetting, const i_setting_value& aValue, bool aApplyNow = true) override;
        void delete_setting(i_setting& aExistingSetting) override;
        void apply_changes() override;
        void discard_changes() override;
        bool modified() const override;
    public:
        void load() override;
        void save() const override;
    public:
        static const uuid& id() { static uuid sId = neolib::make_uuid("E19B3C48-04F7-4207-B24A-2967A3523CE7"); return sId; }
    private:
        void setting_updated(i_setting& aExistingSetting) override;
    private:
        string iFileName;
        mutable std::unique_ptr<xml> iStore;
        category_titles iCategoryTitles;
        group_titles iGroupTitles;
        setting_list iSettings;
    };
}
