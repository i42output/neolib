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
    settings::settings(const i_string& aFileName, ref_ptr<i_custom_type_factory> aCustomSettingTypeFactory) :
        iFileName{ aFileName }, iNextSettingId{ 1 }, iCustomSettingTypeFactory{ aCustomSettingTypeFactory }
    {
        load();
    }

    settings::settings(const i_application& aApp, const i_string& aFileName, ref_ptr<i_custom_type_factory> aCustomSettingTypeFactory) :
        iFileName{ aApp.info().settings_folder() + "/" + aFileName }, iNextSettingId{ 1 }, iCustomSettingTypeFactory{ aCustomSettingTypeFactory }
    {
        load();
    }

    i_setting::id_type settings::register_setting(const i_string& aSettingCategory, const i_string& aSettingName, simple_variant_type aSettingType, const i_simple_variant& aDefaultValue, const i_setting_constraints& aSettingConstraints, bool aHidden)
    {
        return do_register_setting(aSettingCategory, aSettingName, aSettingType, aDefaultValue, aSettingConstraints, aHidden);
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
        return *iter;
    }

    i_setting& settings::find_setting(i_setting::id_type aId)
    {
        setting_list::iterator iter = iSettings.find(setting::key_type(aId));
        if (iter == iSettings.end())
            throw setting_not_found();
        return *iter;
    }

    i_setting& settings::find_setting(const i_string& aSettingCategory, const i_string& aSettingName)
    {
        setting_by_name_list::iterator iter = iSettingsByName.find(std::pair<string, string>(aSettingCategory, aSettingName));
        if (iter == iSettingsByName.end())
            throw setting_not_found();
        return find_setting(iter->second);
    }

    void settings::change_setting(i_setting& aExistingSetting, const i_simple_variant& aValue, bool aApplyNow)
    {
        aExistingSetting.set(aValue);
        if (aApplyNow)
        {
            aExistingSetting.apply_change();
            save();
        }
    }

    void settings::delete_setting(i_setting& aExistingSetting)
    {
        setting_list::iterator iter = iSettings.find(setting::key_type(aExistingSetting.id()));
        if (iter == iSettings.end())
            throw setting_not_found();
        SettingDeleted.trigger(*iter);
        iSettings.erase(iter);
        save();
    }

    void settings::apply_changes()
    {
        if (!dirty())
            return;
        std::set<string> categoriesChanged;
        for (setting_list::iterator iter = iSettings.begin(); iter != iSettings.end(); ++iter)
            if (iter->apply_change())
                categoriesChanged.insert(iter->category());
        for (std::set<string>::const_iterator iter = categoriesChanged.begin(); iter != categoriesChanged.end(); ++iter)
            SettingsChanged.trigger(*iter);
        save();
    }

    void settings::discard_changes()
    {
        for (setting_list::iterator iter = iSettings.begin(); iter != iSettings.end(); ++iter)
            iter->discard_change();
    }

    bool settings::dirty() const
    {
        for (setting_list::const_iterator iter = iSettings.begin(); iter != iSettings.end(); ++iter)
            if (iter->dirty())
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
                xml::element& category = static_cast<xml::element&>(*iStore->root().find_or_append(i->category().c_str()));
                if (i->type() != simple_variant_type::CustomType)
                    category.append(i->name().c_str()).set_attribute("value", to_string(i->value()).c_str());
                else
                {
                    auto const& customType = *i->value().get<i_ref_ptr<i_custom_type>>();
                    xml::element& name = category.append(i->name().c_str());
                    name.set_attribute("type", customType.name().c_str());
                    name.set_attribute("value", customType.to_string());
                }
            }
            iStore->write(output);
        }
    }

    i_setting::id_type settings::do_register_setting(const string& aSettingCategory, const string& aSettingName, simple_variant_type aSettingType, const simple_variant& aDefaultValue, const i_setting_constraints& aSettingConstraints, bool aHidden)
    {
        setting_by_name_list::iterator iterCheck = iSettingsByName.find(setting_by_name_list::key_type(aSettingCategory, aSettingName));
        if (iterCheck != iSettingsByName.end())
            throw setting_already_registered();
        simple_variant currentValue = aDefaultValue;
        if (iStore != nullptr)
        {
            xml::element::iterator xmlIterCategory = iStore->root().find(aSettingCategory.c_str());
            if (xmlIterCategory != iStore->root().end())
            {
                xml::element::iterator xmlIterSetting = xmlIterCategory->find(aSettingName.c_str());
                if (xmlIterSetting != xmlIterCategory->end())
                {
                    if (aSettingType != simple_variant_type::CustomType)
                        currentValue = from_string(xmlIterSetting->attribute_value("value"), aSettingType);
                    else
                    {
                        string valueType = xmlIterSetting->attribute_value("type");
                        string valueData = xmlIterSetting->attribute_value("value");
                        currentValue = simple_variant(ref_ptr<i_custom_type>(iCustomSettingTypeFactory->create(valueType, valueData)));
                    }
                }
            }
        }
        setting_list::iterator iter = iSettings.insert(setting{ *this, iNextSettingId++, aSettingCategory, aSettingName, aSettingType, aSettingConstraints, currentValue, aHidden });
        iSettingsByName[std::pair<string, string>(aSettingCategory, aSettingName)] = iter->id();
        return iter->id();
    }

    void settings::setting_changed(i_setting& aExistingSetting)
    {
        setting_list::iterator iter = iSettings.find(setting::key_type(aExistingSetting.id()));
        if (iter == iSettings.end())
            throw setting_not_found();
        SettingChanged.trigger(*iter);
    }
}
