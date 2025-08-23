// i_enum.hpp
/*
 *  Copyright (c) 2019, 2020 Leigh Johnston.
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
#include <neolib/core/map.hpp>
#include <neolib/core/i_enum.hpp>

namespace neolib
{
    template <typename Enum>
    class basic_enum : public reference_counted<i_basic_enum<std::underlying_type_t<Enum>>>
    {
        typedef reference_counted<i_basic_enum<std::underlying_type_t<Enum>>> base_type;
        // exceptions
    public:
        using typename base_type::bad_enum_string;
        // types
    public:
        typedef i_basic_enum<std::underlying_type_t<Enum>> abstract_type;
        typedef Enum enum_type;
        typedef std::underlying_type_t<Enum> underlying_type;
        // construction/assignment
    public:
        basic_enum() :
            iValue{}
        {
        }
        basic_enum(enum_type aValue) :
            iValue{ static_cast<underlying_type>(aValue) }
        {
        }
        basic_enum(const abstract_type& aOther) :
            iValue{ aOther.value() }
        {
        }
        // state
    public:
        underlying_type value() const final
        {
            return iValue;
        }
        void set_value(underlying_type aValue) final
        {
            iValue = aValue;
        }
        underlying_type set_value(const i_string& aValue) final
        {
            for (auto const& e : enumerators())
                if (e.second() == aValue)
                {
                    iValue = e.first();
                    return value();
                }
            throw bad_enum_string();
        }
        underlying_type const* data() const final
        {
            return &iValue;
        }
        underlying_type* data() final
        {
            return &iValue;
        }
        // meta
    public:
        void to_string(i_string& aString) const final
        {
            aString = enumerators().find(value())->second();
        }
        const typename base_type::enumerators_t& enumerators() const final
        {
            return enum_enumerators<enum_type>();
        }
        // implementation
    private:
        abstract_type* do_clone() const final
        {
            return new basic_enum{ *this };
        }
        abstract_type& do_assign(const abstract_type& aRhs) final
        {
            iValue = aRhs.value();
            return *this;
        }
        // state
    private:
        underlying_type iValue;
    };

    template <typename Enum>
    using enum_t = basic_enum<Enum>;
}


