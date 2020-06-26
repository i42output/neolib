// custom_type.hpp
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
#include <any>
#include <sstream>
#include <optional>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/type_traits.hpp>
#include <neolib/core/i_custom_type.hpp>

namespace neolib
{
    namespace detail
    {
        template <typename T>
        string to_string(const T& aValue)
        {
            std::ostringstream oss;
            oss << aValue;
            return oss.str();
        }

        template <typename T>
        T from_string(const i_string& aValueAsString)
        {
            T result;
            std::ostringstream oss{ aValueAsString.to_std_string() };
            oss >> result;
            return result;
        }
    }

    template <typename T>
    class custom_type : public reference_counted<i_custom_type>
    {
    public:
        struct type_mismatch : std::logic_error { type_mismatch() : std::logic_error("neolib::custom_type::type_mismatch") {} };
    private:
        typedef T value_type;
        typedef abstract_t<value_type> abstract_value_type;
        typedef std::optional<value_type> container_type;
    public:
        custom_type(const string& aName) :
            iName{ aName } {}
        custom_type(const string& aName, const string& aValue) :
            iName{ aName }, iInstance{ detail::from_string<T>(aValue) } {}
        custom_type(const string& aName, const abstract_value_type& aValue) :
            iName{ aName }, iInstance{ aValue } {}
        custom_type(const i_custom_type& aOther) :
            iName{ aOther.name() }, iInstance{ aOther.instance_ptr() ? container_type{aOther.instance_as<abstract_value_type>()} : container_type{} } {}
        ~custom_type() {}
    public:
        using i_custom_type::name;
        using i_custom_type::to_string;
        virtual void name(i_string& aName) const
        { 
            aName = iName;
        }
        virtual void to_string(i_string& aString) const
        {
            if (!!iInstance)
                aString = detail::to_string(*iInstance);
            else
                aString.clear();
        }
        virtual i_custom_type* clone() const 
        { 
            return new custom_type{ *this }; 
        }
        virtual i_custom_type& assign(const i_custom_type& aRhs)
        {
            if (aRhs.name() != name())
                throw type_mismatch();
            if (iInstance == std::nullopt)
                iInstance = value_type{ aRhs.instance_as<abstract_value_type>() };
            else
                *iInstance = aRhs.instance_as<abstract_value_type>();
            return *this;
        }
        virtual bool operator==(const i_custom_type& aRhs) const
        {
            return instance_ptr() == aRhs.instance_ptr() || (instance_ptr() != nullptr && aRhs.instance_ptr() != nullptr && instance_as<abstract_value_type>() == aRhs.instance_as<abstract_value_type>());
        }
        virtual bool operator<(const i_custom_type& aRhs) const
        {
            return (instance_ptr() != nullptr && aRhs.instance_ptr() != nullptr && instance_as<abstract_value_type>() < aRhs.instance_as<abstract_value_type>()) || (instance_ptr() < aRhs.instance_ptr());
        }
    public:
        virtual const void* instance_ptr() const { return iInstance != std::nullopt ? static_cast<const abstract_value_type*>(&*iInstance) : static_cast<const abstract_value_type*>(nullptr); }
        virtual void* instance_ptr() { return iInstance != std::nullopt ? static_cast<abstract_value_type*>(&*iInstance) : static_cast<abstract_value_type*>(nullptr); }
    private:
        string iName;
        container_type iInstance;
    };

    template <typename T>
    const std::string custom_type_name_v = T::type_name;

    template <typename T>
    custom_type<T> make_custom_type()
    {
        return custom_type<T>{ custom_type_name_v<T> };
    }

    template <typename T>
    custom_type<T> make_custom_type(const abstract_t<T>& aValue)
    {
        return custom_type<T>{ custom_type_name_v<T>, aValue };
    }
}
