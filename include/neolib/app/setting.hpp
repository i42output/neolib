// setting.hpp
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
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/app/i_setting.hpp>
#include <neolib/app/i_settings.hpp>
#include <neolib/app/setting_value.hpp>
#include <neolib/app/setting_constraints.hpp>

namespace neolib
{
    template <typename T>
    class setting : public reference_counted<i_setting>
    {
        friend class settings;
        typedef setting<T> self_type;
        typedef reference_counted<i_setting> base_type;
    public:
        typedef T value_type;
        typedef setting_value<T> setting_value_type;
        typedef id_type key_type;
    public:
        setting(i_settings& aManager, const i_string& aCategory, const i_string& aName, const i_setting_constraints& aConstraints = setting_constraints<T>{}, const T& aValue = {}, bool aHidden = false) :
            iManager{ aManager }, 
            iId{}, 
            iCategory{ aCategory }, 
            iName{ aName }, 
            iConstraints{ aConstraints }, 
            iValue{ aValue }, 
            iHidden{ aHidden } 
        {}
        setting(const self_type& aOther) : 
            base_type{ aOther },
            iManager{ aOther.iManager },
            iId{ aOther.iId },
            iCategory{ aOther.iCategory },
            iName{ aOther.iName },
            iConstraints{ aOther.iConstraints },
            iValue{ aOther.iValue },
            iHidden{ aOther.iHidden }
        {
        }
        setting(const i_setting& aSetting) :
            iManager{ aSetting.manager() }, 
            iId{ aSetting.id() }, 
            iCategory{ aSetting.category() }, 
            iName{ aSetting.name() }, 
            iConstraints{ aSetting.constraints() }, 
            iValue{ aSetting.value() }, 
            iHidden{ aSetting.hidden() } 
        {}
    public:
        i_settings& manager() const override 
        { 
            return iManager; 
        }
        id_type id() const override 
        { 
            return iId; 
        }
        string const& category() const override 
        { 
            return iCategory; 
        }
        string const& name() const override 
        { 
            return iName; 
        }
        setting_constraints<T> const& constraints() const override
        { 
            return iConstraints; 
        }
        bool dirty() const override 
        { 
            return !iNewValue.empty(); 
        }
        bool hidden() const override 
        { 
            return iHidden; 
        }
        i_setting_value const& value() const override
        {
            return iValue;
        }
        i_setting_value const& new_value() const override
        {
            return iNewValue;
        }
        void value_as_string(i_string& aValue) const override
        {
            aValue = to_string(iValue);
        }
        void set_value(i_setting_value& aNewValue) override
        {
            if (iValue != aNewValue)
            {
                if (!iValue.is_set())
                    iValue = aNewValue;
                else if (iNewValue != aNewValue)
                {
                    iNewValue = aNewValue;
                    iManager.setting_changed(*this);
                }
            }
            else if (iNewValue.is_set())
            {
                iNewValue.clear();
                iManager.setting_changed(*this);
            }
        }
        void set_value_from_string(i_string const& aNewValue) override
        {
            set_value(setting_value<T>{ from_string<T>(aNewValue) });
        }
        void clear() override
        {
            set_value(setting_value<T>{});
        }
    private:
        void set_id(id_type aId) override
        {
            iId = aId;
        }
        bool apply_change() override
        {
            if (iNewValue.is_set())
            {
                iValue = iNewValue;
                iNewValue = none;
                iManager.setting_changed(*this);
                return true;
            }
            return false;
        }
        bool discard_change() override
        {
            if (iNewValue.is_set())
            {
                iNewValue = none;
                iManager.setting_changed(*this);
                return true;
            }
            return false;
        }
    private:
        void clone(i_ref_ptr<i_setting>& aResult) const override
        {
            aResult = new setting<T>{ *this };
        }
    private:
        i_settings& iManager;
        id_type iId;
        string iCategory;
        string iName;
        setting_constraints<T> iConstraints;
        setting_value_type iValue;
        setting_value_type iNewValue;
        bool iHidden;
    };
}
