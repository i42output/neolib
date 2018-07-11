// settings.hpp - v1.0
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

#include "neolib.hpp"
#include <fstream>
#include <set>
#include <memory>
#include "mutable_set.hpp"
#include "vector.hpp"
#include "xml.hpp"
#include "observable.hpp"
#include "reference_counted.hpp"
#include "string.hpp"
#include "i_custom_type_factory.hpp"
#include "setting.hpp"
#include "i_settings.hpp"

namespace neolib
{
	class settings : public reference_counted<i_settings>, private observable<i_settings::i_subscriber>
	{
		friend class setting;
	private:
		typedef mutable_set<setting> setting_list;
		typedef std::map<std::pair<string, string>, i_setting::id_type> setting_by_name_list;
	public:
		settings(const i_string& aFileName = string("settings.xml"), auto_ref<i_custom_type_factory> aCustomSettingTypeFactory = auto_ref<i_custom_type_factory>()) :
			iFileName(aFileName), iNextSettingId(1), iCustomSettingTypeFactory(aCustomSettingTypeFactory)
		{
			load();
		}
	public:
		virtual i_setting::id_type register_setting(const i_string& aSettingCategory, const i_string& aSettingName, simple_variant_type aSettingType, const i_simple_variant& aDefaultValue = simple_variant(), bool aHidden = false)
		{
			return do_register_setting(aSettingCategory, aSettingName, aSettingType, aDefaultValue, aHidden);
		}
		virtual std::size_t count() const
		{
			return iSettings.size();
		}
		virtual i_setting& get_setting(std::size_t aIndex)
		{
			if (aIndex >= iSettings.size())
				throw setting_not_found();
			setting_list::iterator iter = iSettings.begin();
			std::advance(iter, aIndex);
			return *iter;
		}
		virtual i_setting& find_setting(i_setting::id_type aId)
		{
			setting_list::iterator iter = iSettings.find(setting::key_type(aId));
			if (iter == iSettings.end())
				throw setting_not_found();
			return *iter;
		}
		virtual i_setting& find_setting(const i_string& aSettingCategory, const i_string& aSettingName)
		{
			setting_by_name_list::iterator iter = iSettingsByName.find(std::pair<string, string>(aSettingCategory, aSettingName));
			if (iter == iSettingsByName.end())
				throw setting_not_found();
			return find_setting(iter->second);
		}
		virtual void change_setting(i_setting& aExistingSetting, const i_simple_variant& aValue, bool aApplyNow = false)
		{
			aExistingSetting.set(aValue);
			if (aApplyNow)
			{
				aExistingSetting.apply_change();
				save();
			}
		}
		virtual void delete_setting(i_setting& aExistingSetting)
		{
			setting_list::iterator iter = iSettings.find(setting::key_type(aExistingSetting.id()));
			if (iter == iSettings.end())
				throw setting_not_found();
			notify_observers(i_subscriber::NotifySettingDeleted, *iter);
			iSettings.erase(iter);
			save();
		}
		virtual void apply_changes()
		{
			if (!dirty())
				return;
			std::set<string> categoriesChanged;
			for (setting_list::iterator iter = iSettings.begin(); iter != iSettings.end(); ++iter)
				if (iter->apply_change())
					categoriesChanged.insert(iter->category());
			for (std::set<string>::const_iterator iter = categoriesChanged.begin(); iter != categoriesChanged.end(); ++iter)
				notify_observers(i_subscriber::NotifySettingsChanged, *iter);
			save();
		}
		virtual void discard_changes()
		{
			for (setting_list::iterator iter = iSettings.begin(); iter != iSettings.end(); ++iter)
				iter->discard_change();
		}
		virtual bool dirty() const
		{
			for (setting_list::const_iterator iter = iSettings.begin(); iter != iSettings.end(); ++iter)
				if (iter->dirty())
					return true;
			return false;
		}
	public:
		virtual void load()
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
		virtual void save() const
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
						xml::element& name = category.append(i->name().c_str());
						name.set_attribute("type", i->value().value_as_custom_type().name().c_str());
						name.set_attribute("value", i->value().value_as_custom_type().to_std_string());
					}
				}
				iStore->write(output);
			}
		}
	public:
		virtual void subscribe(i_subscriber& aSubscriber)
		{
			observable<i_subscriber>::add_observer(aSubscriber);
		}
		virtual void unsubscribe(i_subscriber& aSubscriber)
		{
			observable<i_subscriber>::remove_observer(aSubscriber);
		}
	public:
		static const uuid& id() { static uuid sId = neolib::make_uuid("E19B3C48-04F7-4207-B24A-2967A3523CE7"); return sId; }

	protected:
		i_setting::id_type do_register_setting(const string& aSettingCategory, const string& aSettingName, simple_variant_type aSettingType, const simple_variant& aDefaultValue = simple_variant(), bool aHidden = false)
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
							currentValue = simple_variant(auto_ref<i_custom_type>(iCustomSettingTypeFactory->create(valueType, valueData)));
						}
					}
				}
			}
			setting_list::iterator iter = iSettings.insert(setting(*this, iNextSettingId++, aSettingCategory, aSettingName, aSettingType, currentValue, aHidden));
			iSettingsByName[std::pair<string, string>(aSettingCategory, aSettingName)] = iter->id();
			return iter->id();
		}

	private:
		virtual void setting_changed(i_setting& aExistingSetting)
		{
			setting_list::iterator iter = iSettings.find(setting::key_type(aExistingSetting.id()));
			if (iter == iSettings.end())
				throw setting_not_found();
			notify_observers(i_subscriber::NotifySettingChanged, *iter);
		}
	private:
		virtual void notify_observer(i_subscriber& aObserver, notify_type aType, const void* aParameter, const void* aParameter2)
		{
			switch (aType)
			{
			case i_subscriber::NotifySettingsChanged:
				aObserver.settings_changed(*static_cast<const i_string*>(aParameter));
				break;
			case i_subscriber::NotifySettingChanged:
				if (!static_cast<const i_setting*>(aParameter)->dirty() || aObserver.interested_in_dirty_settings())
					aObserver.setting_changed(*static_cast<const i_setting*>(aParameter));
				break;
			case i_subscriber::NotifySettingDeleted:
				aObserver.setting_deleted(*static_cast<const i_setting*>(aParameter));
				break;
			}
		}

	private:
		string iFileName;
		i_setting::id_type iNextSettingId;
		auto_ref<i_custom_type_factory> iCustomSettingTypeFactory;
		mutable std::unique_ptr<xml> iStore;
		setting_list iSettings;
		setting_by_name_list iSettingsByName;
	};
}
