// i_enum.hpp
/*
 *  Copyright (c) 2019 Leigh Johnston.
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
#include <neolib/i_reference_counted.hpp>
#include <neolib/i_string.hpp>
#include <neolib/i_map.hpp>

namespace neolib
{
    template <typename UnderlyingType>
    class i_basic_enum : public i_reference_counted
    {
        typedef i_basic_enum<UnderlyingType> self_type;
        // exceptions
    public:
        struct bad_enum_string : std::logic_error { bad_enum_string() : std::logic_error("neolib::i_basic_enum::bad_enum_string") {} };
        // types
    public:
        typedef UnderlyingType underlying_type;
        typedef i_map<underlying_type, i_string> enumerators_t;
        // construction/assignment
    public:
        virtual self_type* clone() const = 0;
        virtual self_type& assign(const self_type& aRhs) = 0;
        self_type& operator=(const self_type& aRhs)
        {
            assign(aRhs);
            return *this;
        }
        // comparison
    public:
        bool operator==(const self_type& aRhs) const
        {
            return value() == aRhs.value();
        }
        bool operator!=(const self_type& aRhs) const
        {
            return !(*this == aRhs);
        }
        bool operator<(const self_type& aRhs) const
        {
            return value() < aRhs.value();
        }
        // state
    public:
        virtual const underlying_type& value() const = 0;
        virtual underlying_type& value() = 0;
        virtual underlying_type& set_value(const i_string& aValue) = 0;
        underlying_type& set_value(const std::string aValue)
        {
            return set_value(string{ aValue });
        }
        // meta
    public:
        virtual void to_string(i_string& aString) const = 0;
        std::string to_string() const { string s; to_string(s); return s.to_std_string(); }
        virtual const enumerators_t& enumerators() const = 0;
        // helpers
    public:
        template <typename Enum>
        const Enum& value() const
        {
            return static_cast<const Enum&>(value());
        }
        template <typename Enum>
        Enum& value()
        {
            return static_cast<Enum&>(value());
        }
    };

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator==(const i_basic_enum<std::underlying_type_t<Enum>>& lhs, Enum rhs)
    {
        return lhs.value<Enum>() == rhs;
    }

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator==(Enum lhs, const i_basic_enum<std::underlying_type_t<Enum>>& rhs)
    {
        return lhs == rhs.value<Enum>();
    }

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator!=(const i_basic_enum<std::underlying_type_t<Enum>>& lhs, Enum rhs)
    {
        return lhs.value<Enum>() != rhs;
    }

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator!=(Enum lhs, const i_basic_enum<std::underlying_type_t<Enum>>& rhs)
    {
        return lhs != rhs.value<Enum>();
    }

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator<(const i_basic_enum<std::underlying_type_t<Enum>>& lhs, Enum rhs)
    {
        return lhs.value<Enum>() < rhs;
    }

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator<(Enum lhs, const i_basic_enum<std::underlying_type_t<Enum>>& rhs)
    {
        return lhs < rhs.value<Enum>();
    }

    typedef i_basic_enum<uint8_t> i_enum_u8;
    typedef i_basic_enum<uint16_t> i_enum_u16;
    typedef i_basic_enum<uint32_t> i_enum_u32;
    typedef i_basic_enum<uint64_t> i_enum_u64;

    typedef i_basic_enum<int8_t> i_enum_i8;
    typedef i_basic_enum<int16_t> i_enum_i16;
    typedef i_basic_enum<int32_t> i_enum_i32;
    typedef i_basic_enum<int64_t> i_enum_i64;

    typedef i_enum_i32 i_enum;
}
