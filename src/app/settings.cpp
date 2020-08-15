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

#include <neolib/neolib.hpp>
#include <neolib/core/vecarray.hpp>
#include <neolib/app/settings.hpp>

namespace neolib
{
    settings::settings(const i_string& aFileName) :
        iFileName{ aFileName }
    {
        load();
    }

    settings::settings(const i_application& aApp, const i_string& aFileName) :
        iFileName{ aApp.info().settings_folder() + "/" + aFileName }
    {
        load();
    }

    void settings::register_category(i_string const& aCategorySubkey, i_string const& aCategoryTitle)
    {
        iCategoryTitles[aCategorySubkey] = aCategoryTitle;
    }

    void settings::register_group(i_string const& aGroupSubkey, i_string const& aGroupTitle)
    {
        auto const& key = aGroupSubkey.to_std_string();
        auto const& category = key.substr(0, key.find('.'));
        iGroupTitles[string{ category }][aGroupSubkey] = aGroupTitle;
    }

    void settings::register_setting(i_setting& aSetting)
    {
        if (iSettings.find(aSetting.key()) != iSettings.end())
            throw setting_already_registered();
        iSettingsOrdered.push_back(ref_ptr<i_setting>{ aSetting });
        iSettings[aSetting.key()] = iSettingsOrdered.back();
        auto const key = aSetting.key().to_std_string();
        thread_local std::vector<std::string> keyBits;
        keyBits.clear();
        keyBits = tokens(key, "."s);
        if (iStore != nullptr)
        {
            std::optional<xml::element::iterator> node;
            for (auto const& keyBit : keyBits)
            {
                if (!node)
                    node = iStore->root().find_maybe(keyBit);
                else
                    node = (**node).find_maybe(keyBit);
                if (!node)
                    break;
            }
            if (node)
            {
                aSetting.set_value_from_string(string{ (**node).attribute_value("value") });
                aSetting.apply_change();
            }
        }
    }

    settings::category_titles const& settings::all_categories() const
    {
        return iCategoryTitles;
    }

    i_string const& settings::category_title(i_string const& aCategorySubkey) const
    {
        category_titles::const_iterator iter = iCategoryTitles.find(aCategorySubkey);
        if (iter == iCategoryTitles.end())
            throw category_not_found();
        return iter->second();
    }

    settings::group_titles const& settings::all_groups() const
    {
        return iGroupTitles;
    }

    i_string const& settings::group_title(i_string const& aGroupSubkey) const
    {
        auto const& key = aGroupSubkey.to_std_string();
        auto const& category = key.substr(0, key.find('.'));
        group_titles::const_iterator iter = iGroupTitles.find(string{ category });
        if (iter == iGroupTitles.end())
            throw group_not_found();
        auto existing = iter->second().find(aGroupSubkey);
        if (existing == iter->second().end())
            throw group_not_found();
        return existing->second();
    }

    settings::setting_list const& settings::all_settings() const
    {
        return iSettings;
    }

    settings::setting_ordered_list const& settings::all_settings_ordered() const
    {
        return iSettingsOrdered;
    }

    i_setting const& settings::setting(i_string const& aKey) const
    {
        auto existing = iSettings.find(aKey);
        if (existing == iSettings.end())
            throw setting_not_found();
        return *existing->second();
    }

    i_setting& settings::setting(i_string const& aKey)
    {
        auto existing = iSettings.find(aKey);
        if (existing == iSettings.end())
            throw setting_not_found();
        return *existing->second();
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
        setting_list::iterator existing = iSettings.find(aExistingSetting.key());
        if (existing == iSettings.end())
            throw setting_not_found();
        SettingDeleted.trigger(*existing->second());
        iSettings.erase(existing);
        auto existingOrdered = std::find_if(iSettingsOrdered.begin(), iSettingsOrdered.end(), [&](auto const& s) { return &*s == &aExistingSetting; });
        iSettingsOrdered.erase(existingOrdered);
        save();
    }

    void settings::apply_changes()
    {
        if (!modified())
            return;
        std::set<string> categoriesChanged;
        for (auto& setting : iSettings)
        {
            auto const& key = setting.first().to_std_string();
            if (setting.second()->apply_change())
                categoriesChanged.insert(key.substr(0, key.find('.')));
        }
        for (auto const& category : categoriesChanged)
            SettingsChanged.trigger(category);
        save();
    }

    void settings::discard_changes()
    {
        for (auto const& setting : iSettings)
            setting.second()->discard_change();
    }

    bool settings::modified() const
    {
        for (auto const& setting: iSettings)
            if (setting.second()->modified())
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
            for (auto const& setting : iSettings)
            {
                if (setting.second()->is_default())
                    continue;
                auto const& key = setting.first().to_std_string();
                thread_local std::vector<std::string> keyBits;
                keyBits.clear();
                keyBits = tokens(key, "."s);
                std::optional<xml::element::iterator> node;
                for (auto const& keyBit : keyBits)
                {
                    if (!node)
                        node = iStore->root().find_or_append(keyBit);
                    else
                        node = (**node).find_or_append(keyBit);
                }
                (**node).set_attribute("type", setting.second()->value().type_name().to_std_string());
                (**node).set_attribute("value", setting.second()->value_as_string());
            }
            iStore->write(output);
        }
    }
}
