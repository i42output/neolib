// setting_constraints.hpp
/*
 *  Copyright (c) 2020 Leigh Johnston.
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
#include <neolib/core/vector.hpp>
#include <neolib/plugin/simple_variant.hpp>
#include <neolib/app/i_setting_constraints.hpp>

namespace neolib
{
    class setting_constraints : public i_setting_constraints
    {
    public:
        setting_constraints(
            const i_simple_variant& aMinimumValue = simple_variant{},
            const i_simple_variant& aMaximumValue = simple_variant{},
            const i_vector<i_simple_variant>& aAllowableValues = vector<simple_variant>{},
            const i_simple_variant& aStepValue = simple_variant{},
            const i_string& aFormatString = string{}) :
            iMinimumValue{ aMinimumValue },
            iMaximumValue{ aMaximumValue },
            iAllowableValues{ aAllowableValues },
            iStepValue{ aStepValue },
            iFormatString{ aFormatString }
        {}
        setting_constraints(const i_setting_constraints& aOther) :
            iMinimumValue{ aOther.minimum_value() },
            iMaximumValue{ aOther.maximum_value() },
            iAllowableValues{ aOther.allowable_values() },
            iStepValue{ aOther.step_value() },
            iFormatString{ aOther.format_string() }
        {}
    public:
        bool has_minimum_value() const override
        {
            return !iMinimumValue.empty();
        }
        bool has_maximum_value() const override
        {
            return !iMaximumValue.empty();
        }
        bool has_allowable_values() const override
        {
            return !iAllowableValues.empty();
        }
        bool has_step_value() const override
        {
            return !iStepValue.empty();
        }
        bool has_format_string() const override
        {
            return !iFormatString.empty();
        }
        const simple_variant& minimum_value() const override
        {
            return iMinimumValue;
        }
        const simple_variant& maximum_value() const override
        {
            return iMaximumValue;
        }
        const vector<simple_variant>& allowable_values() const override
        {
            return iAllowableValues;
        }
        const simple_variant& step_value() const override
        {
            return iStepValue;
        }
        const string& format_string() const override
        {
            return iFormatString;
        }
    private:
        simple_variant iMinimumValue;
        simple_variant iMaximumValue;
        vector<simple_variant> iAllowableValues;
        simple_variant iStepValue;
        string iFormatString;
    };
}
