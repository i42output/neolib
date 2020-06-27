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
#include <neolib/app/settings.hpp>

namespace neolib
{
    settings::settings(const i_string& aFileName) :
        iFileName{ aFileName }, iNextSettingId{ 1 }
    {
        load();
    }

    settings::settings(const i_application& aApp, const i_string& aFileName) :
        iFileName{ aApp.info().settings_folder() + "/" + aFileName }, iNextSettingId{ 1 }
    {
        load();
    }

    i_setting::id_type settings::register_setting(i_setting& aSetting)
    {
        setting_by_name_list::iterator iterCheck = iSettingsByName.find(setting_by_name_list::key_type(aSetting.category(), aSetting.name()));
        if (iterCheck != iSettingsByName.end())
            throw setting_already_registered();
        if (iStore != nullptr)
        {
            xml::element::iterator xmlIterCategory = iStore->root().find(aSetting.category().to_std_string());
            if (xmlIterCategory != iStore->root().end())
            {
                xml::element::iterator xmlIterSetting = xmlIterCategory->find(aSetting.name().to_std_string());
                if (xmlIterSetting != xmlIterCategory->end())
                    aSetting.set_value_from_string(string{ xmlIterSetting->attribute_value("value") });
            }
        }
        iSettings.insert(std::make_pair(aSetting.id(), ref_ptr<i_setting>{aSetting}));
        iSettingsByName[std::pair<string, string>(aSetting.category(), aSetting.name())] = aSetting.id();
        return aSetting.id();
    }

    std::size_t settings::count() const
    {
        return iSettings.size();
    }

    i_setting& settings::get_setting(std::size_t aIndex)
    {
        if (aIndex >= iSettings.size())
            throw setting_not_found();
        setting_list::iterator iter = iSettings.begin();
        std::advance(iter, aIndex);
        return *iter->second;
    }

    i_setting& settings::find_setting(i_setting::id_type aId)
    {
        setting_list::iterator iter = iSettings.find(aId);
        if (iter == iSettings.end())
            throw setting_not_found();
        return *iter->second;
    }

    i_setting& settings::find_setting(const i_string& aSettingCategory, const i_string& aSettingName)
    {
        setting_by_name_list::iterator iter = iSettingsByName.find(std::pair<string, string>(aSettingCategory, aSettingName));
        if (iter == iSettingsByName.end())
            throw setting_not_found();
        return find_setting(iter->second);
    }

    void settings::change_setting(i_setting& aExistingSetting, const i_setting_value& aValue, bool aApplyNow)
    {
        aExistingSetting.set_value(aValue);
        if (aApplyNow)
        {
            aExistingSetting.apply_change();
            save();
        }
    }

    void settings::delete_setting(i_setting& aExistingSetting)
    {
        setting_list::iterator iter = iSettings.find(aExistingSetting.id());
        if (iter == iSettings.end())
            throw setting_not_found();
        SettingDeleted.trigger(*iter->second);
        iSettings.erase(iter);
        save();
    }

    void settings::apply_changes()
    {
        if (!dirty())
            return;
        std::set<string> categoriesChanged;
        for (setting_list::iterator iter = iSettings.begin(); iter != iSettings.end(); ++iter)
            if (iter->second->apply_change())
                categoriesChanged.insert(iter->second->category());
        for (std::set<string>::const_iterator iter = categoriesChanged.begin(); iter != categoriesChanged.end(); ++iter)
            SettingsChanged.trigger(*iter);
        save();
    }

    void settings::discard_changes()
    {
        for (setting_list::iterator iter = iSettings.begin(); iter != iSettings.end(); ++iter)
            iter->second->discard_change();
    }

    bool settings::dirty() const
    {
        for (setting_list::const_iterator iter = iSettings.begin(); iter != iSettings.end(); ++iter)
            if (iter->second->dirty())
                return true;
        return false;
    }

    void settings::load()
    {
        if (!iFileName.empty())
        {
            std::ifstream input(iFileName.c_str());
            if (input)
            {
                iStore.reset(new xml());
                iStore->read(input);
            }
        }
    }

    void settings::save() const
    {
        if (!iFileName.empty())
        {
            std::ofstream output(iFileName.c_str());
            iStore.reset(new xml());
            iStore->root().name() = "settings";
            for (setting_list::const_iterator i = iSettings.begin(); i != iSettings.end(); ++i)
            {
                xml::element& category = static_cast<xml::element&>(*iStore->root().find_or_append(i->second->category().to_std_string()));
                category.append(i->second->name().to_std_string()).set_attribute("value", i->second->value_as_string());
            }
            iStore->write(output);
        }
    }

    i_setting::id_type settings::next_id()
    {
        return iNextSettingId++;
    }

    void settings::setting_changed(i_setting& aExistingSetting)
    {
        setting_list::iterator iter = iSettings.find(aExistingSetting.id());
        if (iter == iSettings.end())
            throw setting_not_found();
        SettingChanged.trigger(*iter->second);
    }
}
