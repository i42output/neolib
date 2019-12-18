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
#include <neolib/reference_counted.hpp>
#include <neolib/string.hpp>
#include <neolib/map.hpp>

namespace neolib
{
    template <typename Enum>
    using enum_enumerators_t = multimap<std::underlying_type_t<Enum>, string>;
    template <typename Enum>
    const enum_enumerators_t<Enum> enum_enumerators_v;

    #define declare_enum_string( enumName, enumEnumerator ) { static_cast<std::underlying_type_t<enumName>>(enumName::enumEnumerator), neolib::string{ #enumEnumerator } },

    struct bad_enum_value : std::runtime_error { bad_enum_value(const std::string& aString) : std::runtime_error{ "neolib: bad enum value '" + aString + "'" } {} };

    template <typename Enum>
    inline std::string enum_to_hex(Enum aEnumValue)
    {
        std::ostringstream oss;
        oss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(Enum) * 2u) << static_cast<std::underlying_type_t<Enum>>(aEnumValue) << "u";
        return oss.str();
    }

    template <typename Enum, typename StringT = string>
    inline StringT enum_to_string(Enum aEnumerator, bool aMustEnumerate = false)
    {
        auto const e = enum_enumerators_v<Enum>.find(static_cast<std::underlying_type_t<Enum>>(aEnumerator));
        if (e != enum_enumerators_v<Enum>.end())
        {
            if constexpr (std::is_same_v<StringT, std::string>)
                return e->second().to_std_string();
            else
                return e->second();
        }
        else if (!aMustEnumerate)
            return enum_to_hex(aEnumerator);
        else
            throw bad_enum_value(enum_to_hex(aEnumerator));
    }

    struct bad_enum_string : std::runtime_error { bad_enum_string(const std::string& aString) : std::runtime_error{ "neolib: bad enum string '" + aString + "'" } {} };

    template <typename Enum>
    inline Enum string_to_enum(const neolib::i_string& aEnumerator)
    {
        // todo: use bimap.
        for (auto const& e : enum_enumerators_v<Enum>)
            if (e.second() == aEnumerator)
                return static_cast<Enum>(e.first());
        throw bad_enum_string(aEnumerator.to_std_string());
    }
    template <typename Enum>
    inline Enum string_to_enum(const std::string& aEnumerator)
    {
        return string_to_enum<Enum>(neolib::string{ aEnumerator });
    }

    template <typename Enum>
    inline std::optional<Enum> try_string_to_enum(const neolib::i_string& aEnumerator)
    {
        // todo: use bimap.
        for (auto const& e : enum_enumerators_v<Enum>)
            if (e.second() == aEnumerator)
                return static_cast<Enum>(e.first());
        return {};
    }
    template <typename Enum>
    inline std::optional<Enum> try_string_to_enum(const std::string& aEnumerator)
    {
        return try_string_to_enum<Enum>(neolib::string{ aEnumerator });
    }

    template <typename UnderlyingType>
    class i_basic_enum : public i_reference_counted
    {
        typedef i_basic_enum<UnderlyingType> self_type;
        // exceptions
    public:
        struct bad_enum_string : std::logic_error { bad_enum_string() : std::logic_error("neolib::i_basic_enum::bad_enum_string") {} };
        // types
    public:
        typedef self_type abstract_type;
        typedef UnderlyingType underlying_type;
        typedef i_multimap<underlying_type, i_string> enumerators_t;
        // construction/assignment
    public:
        ref_ptr<self_type> clone() const
        {
            return do_clone();
        }
        self_type& operator=(const self_type& aRhs)
        {
            return do_assign(aRhs);
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
        virtual underlying_type value() const = 0;
        virtual underlying_type set_value(underlying_type aValue) = 0;
        virtual underlying_type set_value(const i_string& aValue) = 0;
        underlying_type& set_value(const std::string aValue)
        {
            return set_value(string{ aValue });
        }
        // meta
    public:
        virtual void to_string(i_string& aString) const = 0;
        std::string to_string() const { string s; to_string(s); return s.to_std_string(); }
        virtual const enumerators_t& enumerators() const = 0;
        // implementation
    private:
        virtual self_type* do_clone() const = 0;
        virtual self_type& do_assign(const self_type& aRhs) = 0;
        // helpers
    public:
        template <typename Enum>
        Enum value() const
        {
            return static_cast<Enum>(value());
        }
        template <typename Enum>
        Enum set_value(Enum aValue)
        {
            set_value(static_cast<underlying_type>(aValue));
        }
    };

    template <typename T>
    using i_enum_t = i_basic_enum<std::underlying_type_t<T>>;

    template <typename Enum, typename StringT = string>
    inline StringT enum_to_string(const i_enum_t<Enum>& aEnumerator, bool aMustEnumerate = false)
    {
        return enum_to_string(aEnumerator.value<Enum>(), aMustEnumerate);
    }

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator==(const i_basic_enum<std::underlying_type_t<Enum>>& lhs, Enum rhs)
    {
        return lhs.template value<Enum>() == rhs;
    }

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator==(Enum lhs, const i_basic_enum<std::underlying_type_t<Enum>>& rhs)
    {
        return lhs == rhs.template value<Enum>();
    }

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator!=(const i_basic_enum<std::underlying_type_t<Enum>>& lhs, Enum rhs)
    {
        return lhs.template value<Enum>() != rhs;
    }

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator!=(Enum lhs, const i_basic_enum<std::underlying_type_t<Enum>>& rhs)
    {
        return lhs != rhs.template value<Enum>();
    }

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator<(const i_basic_enum<std::underlying_type_t<Enum>>& lhs, Enum rhs)
    {
        return lhs.template value<Enum>() < rhs;
    }

    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>, bool> operator<(Enum lhs, const i_basic_enum<std::underlying_type_t<Enum>>& rhs)
    {
        return lhs < rhs.template value<Enum>();
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
