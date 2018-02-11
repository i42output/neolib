// simple_variant.hpp - v1.0
/*
 *  Copyright (c) 2007-present, Leigh Johnston.  All Rights Reserved.
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

#include "neolib.hpp"
#include <string>
#include <boost/lexical_cast.hpp>
#include "reference_counted.hpp"
#include "i_simple_variant.hpp"
#include "string.hpp"
#include "variant.hpp"
#include "custom_type.hpp"

namespace neolib
{
	class simple_variant : public reference_counted<i_simple_variant>, public variant<bool, int64_t, double, string, auto_ref<i_custom_type>>
	{
		// types
	private:
		typedef variant<bool, int64_t, double, string, auto_ref<i_custom_type>> variant_type;

		// construction
	public:
		simple_variant() {}
		simple_variant(bool aValue) : variant_type{ aValue } {}
		simple_variant(int32_t aValue) : variant_type{ static_cast<int64_t>(aValue) } {}
		simple_variant(int64_t aValue) : variant_type{ aValue } {}
		simple_variant(double aValue) : variant_type{ aValue } {}
		simple_variant(const char* const aValue) : variant_type{ string(aValue) } {}
		simple_variant(const i_string& aValue) : variant_type{ string(aValue) } {}
		simple_variant(const auto_ref<i_custom_type>& aValue) : variant_type{ aValue } {}
		simple_variant(i_custom_type& aValue) : variant_type{ auto_ref<i_custom_type>(aValue) } {}
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
		simple_variant& operator=(const i_simple_variant& aVariant)
		{
			switch (aVariant.type())
			{
			case Empty:
				// nothing to copy
				break;
			case Boolean:
				static_cast<variant_type&>(*this) = get<bool>(aVariant);
				break;
			case Integer:
				static_cast<variant_type&>(*this) = get<int64_t>(aVariant);
				break;
			case Real:
				static_cast<variant_type&>(*this) = get<double>(aVariant);
				break;
			case String:
				static_cast<variant_type&>(*this) = string(get<i_string>(aVariant));
				break;
			case CustomType:
				if (type() != CustomType || value_as_custom_type().name() != aVariant.value_as_custom_type().name())
					static_cast<variant_type&>(*this) = auto_ref<i_custom_type>(get<i_custom_type>(aVariant).clone());
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
		virtual type_e type() const
		{
			switch (variant_type::which())
			{
			case 0:
				return Empty;
			case type_id<bool>::value:
				return Boolean;
			case type_id<int64_t>::value:
				return Integer;
			case type_id<double>::value:
				return Real;
			case type_id<string>::value:
				return String;
			case type_id<auto_ref<i_custom_type>>::value:
				return CustomType;
			default:
				throw unknown_type();
			}
		}
		using i_simple_variant::empty;
	public:
		virtual const bool& value_as_boolean() const
		{
			return static_variant_cast<const bool&>(*this);
		}
		virtual bool& value_as_boolean()
		{
			return static_variant_cast<bool&>(*this);
		}
		virtual const int64_t& value_as_integer() const
		{
			return static_variant_cast<const int64_t&>(*this);
		}
		virtual int64_t& value_as_integer()
		{
			return static_variant_cast<int64_t&>(*this);
		}
		virtual const double& value_as_real() const
		{
			return static_variant_cast<const double&>(*this);
		}
		virtual double& value_as_real()
		{
			return static_variant_cast<double&>(*this);
		}
		virtual const i_string& value_as_string() const
		{
			return static_variant_cast<const string&>(*this);
		}
		virtual i_string& value_as_string()
		{
			return static_variant_cast<string&>(*this);
		}
		virtual const i_custom_type& value_as_custom_type() const
		{
			return *static_variant_cast<const auto_ref<i_custom_type>&>(*this);
		}
		virtual i_custom_type& value_as_custom_type()
		{
			return *static_variant_cast<auto_ref<i_custom_type>&>(*this);
		}
	};

	inline simple_variant from_string(const std::string& aValue, i_simple_variant::type_e aType)
	{
		switch (aType)
		{
		case i_simple_variant::Boolean:
			return boost::lexical_cast<bool>(aValue);
		case i_simple_variant::Integer:
			return boost::lexical_cast<int64_t>(aValue);
		case i_simple_variant::Real:
			return boost::lexical_cast<double>(aValue);
		case i_simple_variant::String:
			return neolib::string(aValue);
		case i_simple_variant::CustomType:
		default:
			throw i_simple_variant::unsupported_operation("can't create from string");
		}
	}

	inline simple_variant from_string(const i_string& aValue, i_simple_variant::type_e aType)
	{
		return from_string(aValue.c_str(), aType);
	}
}
