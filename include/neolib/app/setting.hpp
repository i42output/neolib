// setting.hpp
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
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/task/event.hpp>
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
    public:
        define_declared_event(Changing, changing)
        define_declared_event(Changed, changed)
    public:
        setting(i_settings& aManager, i_string const& aKey, T const& aDefaultValue, i_setting_constraints const& aConstraints = setting_constraints<T>{}, i_string const& aFormat = string{}) :
            iManager{ aManager }, 
            iKey{ aKey }, 
            iConstraints{ aConstraints },
            iFormat{ aFormat },
            iEnabled{ !aConstraints.initially_disabled() },
            iDefaultValue{ aDefaultValue },
            iValue{}
        {}
        setting(const self_type& aOther) : 
            base_type{ aOther },
            iManager{ aOther.iManager },
            iKey{ aOther.iKey },
            iConstraints{ aOther.iConstraints },
            iFormat{ aOther.iFormat },
            iEnabled{ !aOther.iConstraints.initially_disabled() },
            iDefaultValue{ aOther.iDefaultValue },
            iValue{ aOther.iValue }
        {
        }
        setting(const i_setting& aSetting) :
            iManager{ aSetting.manager() }, 
            iKey{ aSetting.key() }, 
            iConstraints{ aSetting.constraints() },
            iFormat{ aSetting.format() },
            iEnabled{ !aSetting.constraints().initially_disabled() },
            iDefaultValue{ aSetting.default_value() },
            iValue{ aSetting.value() }
        {}
    public:
        i_settings& manager() const override 
        { 
            return iManager; 
        }
        string const& key() const override 
        { 
            return iKey; 
        }
        setting_constraints<T> const& constraints() const override
        { 
            return iConstraints; 
        }
        string const& format() const override
        {
            return iFormat;
        }
        bool hidden() const override
        {
            return iFormat.empty();
        }
        bool is_enabled() const override
        {
            return iEnabled;
        }
        void set_enabled(bool aEnabled) override
        {
            if (iEnabled != aEnabled)
            {
                iEnabled = aEnabled;
                Changed.trigger();
                iManager.setting_changed().trigger(*this);
            }
        }
        bool is_default(bool aUnappliedNew = false) const override
        {
            if (aUnappliedNew && modified())
                return !modified_value().is_set();
            return !iValue.is_set();
        }
        bool modified() const override
        { 
            return iNewValue != std::nullopt; 
        }
        i_setting_value const& default_value() const override
        {
            return iDefaultValue;
        }
        using i_setting::value;
        i_setting_value const& value(bool aUnappliedNew = false) const override
        {
            if (aUnappliedNew && modified())
            {
                if (modified_value().is_set())
                    return modified_value();
                return iDefaultValue;
            }
            if (iValue.is_set())
                return iValue;
            return iDefaultValue;
        }
        using i_setting::modified_value;
        i_setting_value const& modified_value() const override
        {
            if (!modified())
                throw setting_not_modified();
            return *iNewValue;
        }
        using i_setting::value_as_string;
        void value_as_string(i_string& aValue, bool aUnappliedNew = false) const override
        {
            aValue = to_string(value(aUnappliedNew).get<T>());
        }
        using i_setting::set_default_value;
        void set_default_value(i_setting_value const& aDefaultValue) override
        {
            if (iDefaultValue != aDefaultValue)
            {
                iDefaultValue = aDefaultValue;
                if (is_default())
                {
                    Changed.trigger();
                    iManager.setting_changed().trigger(*this);
                }
            }
        }
        using i_setting::set_value;
        void set_value(i_setting_value const& aNewValue) override
        {
            if (iValue != aNewValue)
            {
                if (!modified() || modified_value() != aNewValue)
                {
                    iNewValue = aNewValue;
                    Changing.trigger();
                    iManager.setting_changing().trigger(*this);
                }
            }
            else if (modified())
            {
                iNewValue = std::nullopt;
                Changing.trigger();
                iManager.setting_changing().trigger(*this);
            }
        }
        void set_value_from_string(i_string const& aNewValue) override
        {
            set_value(setting_value<T>{ from_string<T>(aNewValue) });
        }
        void clear() override
        {
            if (!constraints().optional())
                throw setting_not_optional();
            iNewValue.emplace();
            Changing.trigger();
            iManager.setting_changing().trigger(*this);
        }
    private:
        bool apply_change() override
        {
            if (modified())
            {
                bool changed = (iValue != *iNewValue);
                iValue = *iNewValue;
                iNewValue = std::nullopt;
                if (changed)
                {
                    Changed.trigger();
                    iManager.setting_changed().trigger(*this);
                }
                else
                {
                    Changing.trigger();
                    iManager.setting_changing().trigger(*this);
                }
                return true;
            }
            return false;
        }
        bool discard_change() override
        {
            if (modified())
            {
                iNewValue = std::nullopt;
                Changing.trigger();
                iManager.setting_changing().trigger(*this);
                return true;
            }
            return false;
        }
    private:
        void clone(i_ref_ptr<i_setting>& aResult) const override
        {
            aResult = make_ref<setting<T>>(*this);
        }
    private:
        i_settings& iManager;
        string iKey;
        setting_constraints<T> iConstraints;
        string iFormat;
        bool iEnabled;
        setting_value_type iDefaultValue;
        setting_value_type iValue;
        std::optional<setting_value_type> iNewValue;
    };
}
