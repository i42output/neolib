// i_setting_constraints.hpp
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
#include <neolib/core/i_vector.hpp>
#include <neolib/core/i_string.hpp>
#include <neolib/app/i_setting_value.hpp>

namespace neolib
{
    class i_setting_constraints
    {
    public:
        virtual ~i_setting_constraints() = default;
    public:
        virtual bool has_minimum_value() const = 0;
        virtual bool has_maximum_value() const = 0;
        virtual bool has_allowable_values() const = 0;
        virtual bool has_step_value() const = 0;
        virtual i_setting_value const& minimum_value() const = 0;
        virtual i_setting_value const& maximum_value() const = 0;
        virtual i_vector<i_setting_value> const& allowable_values() const = 0;
        virtual i_setting_value const& step_value() const = 0;
    public:
        template <typename T>
        abstract_t<T> const& minimum_value() const
        {
            return minimum_value().get<T>();
        }
        template <typename T>
        abstract_t<T> const& maximum_value() const
        {
            return maximum_value().get<T>();
        }
        template <typename T>
        abstract_t<T> const& step_value() const
        {
            return step_value().get<T>();
        }
    };
}
