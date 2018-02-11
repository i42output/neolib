// setting.hpp - v1.0
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
#include "reference_counted.hpp"
#include "string.hpp"
#include "simple_variant.hpp"
#include "i_setting.hpp"
#include "i_settings.hpp"

namespace neolib
{
	class setting : public reference_counted<i_setting>
	{
		friend class settings;
	public:
		typedef id_type key_type;
	public:
		setting(i_settings& aManager, id_type aId, const i_string& aCategory, const i_string& aName, i_simple_variant::type_e aType, const simple_variant& aValue = simple_variant(), bool aHidden = false) :
			iManager(aManager), iId(aId), iCategory(aCategory), iName(aName), iType(aType), iValue(aValue), iHidden(aHidden) {}
		setting(const i_setting& aSetting) :
			iManager(aSetting.manager()), iId(aSetting.id()), iCategory(aSetting.category()), iName(aSetting.name()), iType(aSetting.type()), iValue(aSetting.value()), iHidden(aSetting.hidden()) {}
	public:
		virtual i_settings& manager() const { return iManager; }
		virtual const id_type id() const { return iId; }
		virtual const i_string& category() const { return iCategory; }
		virtual const i_string& name() const { return iName; }
		virtual i_simple_variant::type_e type() const { return iType; }
		virtual const i_simple_variant& value() const { return iValue; }
		virtual void set(const i_simple_variant& aNewValue);
		virtual const i_simple_variant& new_value() const { if (!iNewValue.empty()) return iNewValue; return iValue; }
		virtual bool dirty() const { return !iNewValue.empty(); }
		virtual bool hidden() const { return iHidden; }
	public:
		operator key_type() const { return key_type(iId); }
	private:
		virtual bool apply_change();
		virtual bool discard_change();
	private:
		i_settings& iManager;
		id_type iId;
		string iCategory;
		string iName;
		i_simple_variant::type_e iType;
		simple_variant iValue;
		simple_variant iNewValue;
		bool iHidden;
	};
}
