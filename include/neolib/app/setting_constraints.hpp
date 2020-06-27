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
#include <neolib/app/setting_value.hpp>
#include <neolib/app/i_setting_constraints.hpp>

namespace neolib
{
    template <typename T>
    class setting_constraints : public i_setting_constraints
    {
    public:
        typedef T value_type;
        typedef setting_value<value_type> setting_value_type;
    public:
        setting_constraints(
            const setting_value_type& aMinimumValue = setting_value_type{},
            const setting_value_type& aMaximumValue = setting_value_type{},
            const i_vector<i_setting_value>& aAllowableValues = vector<setting_value_type>{},
            const setting_value_type& aStepValue = setting_value_type{}) :
            iMinimumValue{ aMinimumValue },
            iMaximumValue{ aMaximumValue },
            iAllowableValues{ aAllowableValues },
            iStepValue{ aStepValue }
        {}
        setting_constraints(const i_setting_constraints& aOther) :
            iMinimumValue{ aOther.minimum_value() },
            iMaximumValue{ aOther.maximum_value() },
            iAllowableValues{ aOther.allowable_values() },
            iStepValue{ aOther.step_value() }
        {}
    public:
        bool has_minimum_value() const override
        {
            return iMinimumValue.is_set();
        }
        bool has_maximum_value() const override
        {
            return iMaximumValue.is_set();
        }
        bool has_allowable_values() const override
        {
            return !!iAllowableValues;
        }
        bool has_step_value() const override
        {
            return iStepValue.is_set();
        }
        setting_value_type const& minimum_value() const override
        {
            return iMinimumValue;
        }
        setting_value_type const& maximum_value() const override
        {
            return iMaximumValue;
        }
        vector<setting_value_type> const& allowable_values() const override
        {
            return *iAllowableValues;
        }
        setting_value_type const& step_value() const override
        {
            return iStepValue;
        }
    private:
        setting_value_type iMinimumValue;
        setting_value_type iMaximumValue;
        std::optional<vector<setting_value_type>> iAllowableValues;
        setting_value_type iStepValue;
    };
}
