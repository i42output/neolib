// simple_variant.hpp - v1.0
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
#include <string>
#include <boost/lexical_cast.hpp>
#include <neolib/reference_counted.hpp>
#include <neolib/i_simple_variant.hpp>
#include <neolib/string.hpp>
#include <neolib/enum.hpp>
#include <neolib/variant.hpp>
#include <neolib/custom_type.hpp>

namespace neolib
{
    class simple_variant : public reference_counted<i_simple_variant>, public variant<bool, int64_t, double, string, ref_ptr<i_enum>, ref_ptr<i_custom_type>>
    {
        // types
    private:
        typedef neolib::variant<bool, int64_t, double, string, ref_ptr<i_enum>, ref_ptr<i_custom_type>> variant_type;

        // construction
    public:
        simple_variant() {}
        simple_variant(bool aValue) : variant_type{ aValue } {}
        simple_variant(int32_t aValue) : variant_type{ static_cast<int64_t>(aValue) } {}
        simple_variant(int64_t aValue) : variant_type{ aValue } {}
        simple_variant(double aValue) : variant_type{ aValue } {}
        simple_variant(const char* const aValue) : variant_type{ string(aValue) } {}
        simple_variant(const i_string& aValue) : variant_type{ string(aValue) } {}
        simple_variant(const ref_ptr<i_enum>& aValue) : variant_type{ aValue } {}
        simple_variant(i_enum& aValue) : variant_type{ ref_ptr<i_enum>(aValue) } {}
        simple_variant(const ref_ptr<i_custom_type>& aValue) : variant_type{ aValue } {}
        simple_variant(i_custom_type& aValue) : variant_type{ ref_ptr<i_custom_type>(aValue) } {}
        simple_variant(const simple_variant& aVariant) : variant_type{ static_cast<const variant_type&>(aVariant) }
        {
        }
        simple_variant(simple_variant&& aVariant) : variant_type{ static_cast<variant_type&&>(aVariant) }
        {
        }
        template <typename T>
        simple_variant(T&& aValue, typename std::enable_if<!std::is_same<typename std::remove_cv<typename std::remove_reference<T>::type>::type, simple_variant>::value, simple_variant>::type* = nullptr) : variant_type{ std::forward<T>(aValue) }
        {
        }
        simple_variant(const i_simple_variant& aVariant)
        {
            *this = aVariant;
        }

        // assignment
    public:
        simple_variant& operator=(const simple_variant& aVariant)
        {
            variant_type::operator=(static_cast<const variant_type&>(aVariant));
            return *this;
        }
        simple_variant& operator=(simple_variant&& aVariant)
        {
            variant_type::operator=(static_cast<variant_type&&>(aVariant));
            return *this;
        }
        template <typename T>
        typename std::enable_if<!std::is_same<typename std::remove_cv<typename std::remove_reference<T>::type>::type, simple_variant>::value, simple_variant>::type& operator=(T&& aValue)
        {
            variant_type::operator=(std::forward<T>(aValue));
            return *this;
        }
        simple_variant& operator=(const i_simple_variant& aVariant) override
        {
            switch (aVariant.type())
            {
            case simple_variant_type::Empty:
                // nothing to copy
                break;
            case simple_variant_type::Boolean:
                static_cast<variant_type&>(*this) = get<bool>(aVariant);
                break;
            case simple_variant_type::Integer:
                static_cast<variant_type&>(*this) = get<int64_t>(aVariant);
                break;
            case simple_variant_type::Real:
                static_cast<variant_type&>(*this) = get<double>(aVariant);
                break;
            case simple_variant_type::String:
                static_cast<variant_type&>(*this) = string(get<i_string>(aVariant));
                break;
            case simple_variant_type::Enum:
                if (type() != simple_variant_type::Enum)
                    static_cast<variant_type&>(*this) = ref_ptr<i_enum>(get<i_enum>(aVariant).clone());
                else
                    value_as_enum() = aVariant.value_as_enum();
                break;
            case simple_variant_type::CustomType:
                if (type() != simple_variant_type::CustomType || value_as_custom_type().name() != aVariant.value_as_custom_type().name())
                    static_cast<variant_type&>(*this) = ref_ptr<i_custom_type>(get<i_custom_type>(aVariant).clone());
                else
                    value_as_custom_type() = aVariant.value_as_custom_type();
                break;
            default:
                throw unknown_type();
            }
            return *this;
        }

        // operations
    public:
        simple_variant_type type() const override
        {
            auto result = static_cast<simple_variant_type>(variant_type::index());
            if (result < simple_variant_type::COUNT)
                return result;
            throw unknown_type();
        }
        using i_simple_variant::empty;
    public:
        const bool& value_as_boolean() const override
        {
            return static_variant_cast<const bool&>(*this);
        }
        bool& value_as_boolean() override
        {
            return static_variant_cast<bool&>(*this);
        }
        const int64_t& value_as_integer() const override
        {
            return static_variant_cast<const int64_t&>(*this);
        }
        int64_t& value_as_integer() override
        {
            return static_variant_cast<int64_t&>(*this);
        }
        const double& value_as_real() const override
        {
            return static_variant_cast<const double&>(*this);
        }
        double& value_as_real() override
        {
            return static_variant_cast<double&>(*this);
        }
        const i_string& value_as_string() const override
        {
            return static_variant_cast<const string&>(*this);
        }
        i_string& value_as_string() override
        {
            return static_variant_cast<string&>(*this);
        }
        const i_enum& value_as_enum() const override
        {
            return *static_variant_cast<const ref_ptr<i_enum>&>(*this);
        }
        i_enum& value_as_enum() override
        {
            return *static_variant_cast<ref_ptr<i_enum>&>(*this);
        }
        const i_custom_type& value_as_custom_type() const override
        {
            return *static_variant_cast<const ref_ptr<i_custom_type>&>(*this);
        }
        i_custom_type& value_as_custom_type() override
        {
            return *static_variant_cast<ref_ptr<i_custom_type>&>(*this);
        }
    };

    inline simple_variant from_string(const std::string& aValue, simple_variant_type aType)
    {
        switch (aType)
        {
        case simple_variant_type::Boolean:
            return boost::lexical_cast<bool>(aValue);
        case simple_variant_type::Integer:
            return boost::lexical_cast<int64_t>(aValue);
        case simple_variant_type::Real:
            return boost::lexical_cast<double>(aValue);
        case simple_variant_type::String:
            return neolib::string(aValue);
        case simple_variant_type::Enum:
        case simple_variant_type::CustomType:
        default:
            throw i_simple_variant::unsupported_operation("can't create from string");
        }
    }

    inline simple_variant from_string(const i_string& aValue, simple_variant_type aType)
    {
        return from_string(aValue.c_str(), aType);
    }
}
