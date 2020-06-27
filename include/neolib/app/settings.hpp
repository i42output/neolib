// settings.hpp
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
#include <fstream>
#include <set>
#include <memory>
#include <neolib/core/mutable_set.hpp>
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
    class settings : public reference_counted<i_settings>
    {
        template <typename T>
        friend class setting;
    public:
        define_declared_event(SettingsChanged, settings_changed, const i_string&)
        define_declared_event(SettingChanged, setting_changed, const i_setting&)
        define_declared_event(SettingDeleted, setting_deleted, const i_setting&)
        define_declared_event(InterestedInDirtySettings, interested_in_dirty_settings, bool&)
    private:
        typedef std::map<i_setting::id_type, ref_ptr<i_setting>> setting_list;
        typedef std::map<std::pair<string, string>, i_setting::id_type> setting_by_name_list;
    public:
        settings(const i_string& aFileName);
        settings(const i_application& aApp, const i_string& aFileName = string{ "settings.xml" });
    public:
        using i_settings::register_setting;
        i_setting::id_type register_setting(i_setting& aSetting) override;
        std::size_t count() const override;
        i_setting& get_setting(std::size_t aIndex) override;
        i_setting& find_setting(i_setting::id_type aId) override;
        i_setting& find_setting(const i_string& aSettingCategory, const i_string& aSettingName) override;
        void change_setting(i_setting& aExistingSetting, const i_setting_value& aValue, bool aApplyNow = false) override;
        void delete_setting(i_setting& aExistingSetting) override;
        void apply_changes() override;
        void discard_changes() override;
        bool dirty() const override;
    public:
        void load() override;
        void save() const override;
    public:
        static const uuid& id() { static uuid sId = neolib::make_uuid("E19B3C48-04F7-4207-B24A-2967A3523CE7"); return sId; }
    private:
        i_setting::id_type next_id() override;
        void setting_changed(i_setting& aExistingSetting) override;
    private:
        string iFileName;
        i_setting::id_type iNextSettingId;
        mutable std::unique_ptr<xml> iStore;
        setting_list iSettings;
        setting_by_name_list iSettingsByName;
    };
}
