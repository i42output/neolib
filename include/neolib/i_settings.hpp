	// i_settings.hpp - v1.0
/*
 *  Copyright (c) 2012-present, Leigh Johnston.  All Rights Reserved.
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
#include "vector.hpp"
#include "i_reference_counted.hpp"
#include "uuid.hpp"
#include "i_string.hpp"
#include "simple_variant.hpp"
#include "i_setting.hpp"

namespace neolib
{
	class i_settings : public i_reference_counted
	{
		friend class setting;
	public:
		struct setting_already_registered : std::logic_error { setting_already_registered() : std::logic_error("neolib::i_settings::setting_already_registered") {} };
		struct setting_not_found : std::logic_error { setting_not_found() : std::logic_error("neolib::i_settings::setting_not_found") {} };
	public:
		class i_subscriber
		{
		public:
			virtual void settings_changed(const i_string& aSettingCategory) = 0;
			virtual void setting_changed(const i_setting& aSetting) = 0;
			virtual void setting_deleted(const i_setting& aSetting) = 0;
			virtual bool interested_in_dirty_settings() const { return false; }
		public:
			enum notify_type
			{
				NotifySettingsChanged,
				NotifySettingChanged,
				NotifySettingDeleted
			};
		};
	public:
		virtual i_setting::id_type register_setting(const i_string& aSettingCategory, const i_string& aSettingName, i_simple_variant::type_e aSettingType, const i_simple_variant& aDefaultValue = simple_variant(), bool aHidden = false) = 0;
		i_setting::id_type register_setting(const string& aSettingCategory, const string& aSettingName, i_simple_variant::type_e aSettingType, const simple_variant& aDefaultValue = simple_variant(), bool aHidden = false)
		{
			return register_setting(static_cast<const i_string&>(aSettingCategory), static_cast<const i_string&>(aSettingName), aSettingType, static_cast<const i_simple_variant&>(aDefaultValue), aHidden);
		}
		virtual std::size_t count() const = 0;
		virtual i_setting& get_setting(std::size_t aIndex) = 0;
		virtual i_setting& find_setting(i_setting::id_type aId) = 0;
		virtual i_setting& find_setting(const i_string& aSettingCategory, const i_string& aSettingName) = 0;
		i_setting& find_setting(const string& aSettingCategory, const string& aSettingName) { return find_setting(static_cast<const i_string&>(aSettingCategory), static_cast<const i_string&>(aSettingName)); }
		virtual void change_setting(i_setting& aExistingSetting, const i_simple_variant& aValue, bool aApplyNow = false) = 0;
		void change_setting(i_setting& aExistingSetting, const simple_variant& aValue, bool aApplyNow = false) { change_setting(aExistingSetting, static_cast<const i_simple_variant&>(aValue), aApplyNow); }
		virtual void delete_setting(i_setting& aExistingSetting) = 0;
		virtual void apply_changes() = 0;
		virtual void discard_changes() = 0;
		virtual bool dirty() const = 0;
	public:
		virtual void load() = 0;
		virtual void save() const = 0;
	public:
		virtual void subscribe(i_subscriber& aSubscriber) = 0;
		virtual void unsubscribe(i_subscriber& aSubscriber) = 0;
	public:
		static const uuid& id() { static uuid sId = neolib::make_uuid("E19B3C48-04F7-4207-B24A-2967A3523CE7"); return sId; }
	private:
		virtual void setting_changed(i_setting& aExistingSetting) = 0;
	};
}
