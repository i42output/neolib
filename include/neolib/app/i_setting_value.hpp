// i_setting_value.hpp
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
#include <neolib/core/string.hpp>
#include <neolib/core/i_enum.hpp>

namespace neolib
{
    enum class setting_type : uint32_t
    {
        Boolean,
        Int8,
        Int16,
        Int32,
        Int64,
        Uint8,
        Uint16,
        Uint32,
        Uint64,
        Float32,
        Float64,
        String,
        Enum,
        Custom
    };

    template<setting_type ST> struct get_setting_type;
    template<> struct get_setting_type<setting_type::Boolean> { typedef bool type; };
    template<> struct get_setting_type<setting_type::Int8> { typedef int8_t type; };
    template<> struct get_setting_type<setting_type::Int16> { typedef int16_t type; };
    template<> struct get_setting_type<setting_type::Int32> { typedef int32_t type; };
    template<> struct get_setting_type<setting_type::Int64> { typedef int64_t type; };
    template<> struct get_setting_type<setting_type::Uint8> { typedef uint8_t type; };
    template<> struct get_setting_type<setting_type::Uint16> { typedef uint16_t type; };
    template<> struct get_setting_type<setting_type::Uint32> { typedef uint32_t type; };
    template<> struct get_setting_type<setting_type::Uint64> { typedef uint64_t type; };
    template<> struct get_setting_type<setting_type::Float32> { typedef float type; };
    template<> struct get_setting_type<setting_type::Float64> { typedef double type; };
    template<> struct get_setting_type<setting_type::String> { typedef string type; };
    template <setting_type ST>
    using setting_type_t = typename get_setting_type<ST>::type;

    template <typename T, typename = sfinae> constexpr setting_type setting_type_v = setting_type::Custom;
    template<> constexpr setting_type setting_type_v<bool> = setting_type::Boolean;
    template<> constexpr setting_type setting_type_v<int8_t> = setting_type::Int8;
    template<> constexpr setting_type setting_type_v<int16_t> = setting_type::Int16;
    template<> constexpr setting_type setting_type_v<int32_t> = setting_type::Int32;
    template<> constexpr setting_type setting_type_v<int64_t> = setting_type::Int64;
    template<> constexpr setting_type setting_type_v<uint8_t> = setting_type::Uint8;
    template<> constexpr setting_type setting_type_v<uint16_t> = setting_type::Uint16;
    template<> constexpr setting_type setting_type_v<uint32_t> = setting_type::Uint32;
    template<> constexpr setting_type setting_type_v<uint64_t> = setting_type::Uint64;
    template<> constexpr setting_type setting_type_v<float> = setting_type::Float32;
    template<> constexpr setting_type setting_type_v<double> = setting_type::Float64;
    template<> constexpr setting_type setting_type_v<string> = setting_type::String;
    template<typename T> constexpr setting_type setting_type_v<T, std::enable_if_t<std::is_enum_v<T>, sfinae>> = setting_type::Enum;

    template <typename T, typename = sfinae> struct setting_type_name {};
    
    #define define_setting_type(T) template<> struct ::neolib::setting_type_name<T> { static const ::neolib::string& name() { static ::neolib::string sTypeName = #T; return sTypeName; } };

    define_setting_type(bool)
    define_setting_type(int8_t)
    define_setting_type(int16_t)
    define_setting_type(int32_t)
    define_setting_type(int64_t)
    define_setting_type(uint8_t)
    define_setting_type(uint16_t)
    define_setting_type(uint32_t)
    define_setting_type(uint64_t)
    define_setting_type(float)
    define_setting_type(double)
    define_setting_type(string)
    template<typename T> struct setting_type_name<T, std::enable_if_t<std::is_enum_v<T>, sfinae>> { static const neolib::string& name() { static neolib::string sTypeName = "enum"; return sTypeName; } };

    template<typename T>
    const string setting_type_name_v = setting_type_name<T>::name();

    class i_setting_value
    {
    public:
        typedef i_setting_value abstract_type;
    public:
        struct not_set : std::logic_error { not_set() : std::logic_error{ "neolib::i_setting_value::not_set" } {} };
    public:
        virtual ~i_setting_value() = default;
    public:
        virtual setting_type type() const = 0;
        virtual i_string const& type_name() const = 0;
        virtual bool is_set() const = 0;
        virtual void clear() = 0;
    public:
        virtual bool operator==(const i_setting_value& aRhs) const = 0;
        virtual bool operator<(const i_setting_value& aRhs) const = 0;
    private:
        virtual void const* data() const = 0;
        virtual void* data() = 0;
    public:
        bool operator!=(const i_setting_value& aRhs) const
        {
            return !(*this == aRhs);
        }
        template <typename T>
        abstract_return_t<T const> get() const
        {
            if (type() != setting_type::Enum)
                return *static_cast<abstract_t<T> const*>(data());
            else 
            {
                if constexpr (std::is_same_v<T, i_enum>)
                    return *static_cast<i_enum const*>(data());
                else if constexpr (std::is_scalar_v<T>)
                    return static_cast<i_enum const*>(data())->value<T>();
                else
                    return *static_cast<abstract_t<T> const*>(data());
            }
        }
        template <typename T>
        void set(T const& aNewValue)
        {
            if (type() != setting_type::Enum)
                *static_cast<abstract_t<T>*>(data()) = aNewValue;
            else
            {
                if constexpr (std::is_same_v<T, i_enum>)
                    *static_cast<i_enum*>(data()) = aNewValue;
                else if constexpr (std::is_scalar_v<T>)
                    static_cast<i_enum*>(data())->set_value<T>(aNewValue);
                else
                    *static_cast<abstract_t<T>*>(data()) = aNewValue;
            }
        }
    };
}
