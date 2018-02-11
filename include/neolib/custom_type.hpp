// custom_type.hpp - v1.0
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
#include <boost/any.hpp>
#include <sstream>
#include <boost/optional.hpp>
#include "reference_counted.hpp"
#include "string.hpp"
#include "type_traits.hpp"
#include "i_custom_type.hpp"

namespace neolib
{
	namespace detail
	{
		template <bool>
		struct to_string
		{
			template <typename T>
			i_string* operator()(const T& aValue)
			{
				std::ostringstream oss;
				oss << aValue;
				return new string(oss.str());
			}
		};
		template <>
		struct to_string<false>
		{
			template <typename T>
			i_string* operator()(const T& aValue)
			{
				return new string();
			}
		};
	}

	template <typename AbstractType, typename ConcreteType>
	class custom_type : public reference_counted<i_custom_type>
	{
	public:
		struct type_mismatch : std::logic_error { type_mismatch() : std::logic_error("neolib::custom_type::type_mismatch") {} };
	private:
		typedef boost::optional<ConcreteType> container_type;
	public:
		custom_type(const string& aName) :
			iName(aName) {}
		custom_type(const string& aName, const AbstractType& aInstance) :
			iName(aName), iInstance(aInstance) {}
		custom_type(const i_custom_type& aOther) :
			iName(aOther.name()), iInstance(aOther.instance_ptr() ? container_type(aOther.instance_as<AbstractType>()), container_type()) {}
		~custom_type() {}
	public:
		virtual const i_string& name() const { return iName; }
		virtual i_string& name() { return iName; }
		virtual i_string* to_string() const { if (!!iInstance) return detail::to_string<type_traits::has_saving_support<AbstractType>::value>()(*iInstance); else return new string(); }
		virtual i_custom_type* clone() const { return new custom_type(*this); }
		virtual i_custom_type& assign(const i_custom_type& aRhs)
		{
			if (aRhs.name() != name())
				throw type_mismatch();
			if (iInstance == boost::none)
				iInstance = ConcreteType(aRhs.instance_as<AbstractType>());
			else
				*iInstance = aRhs.instance_as<AbstractType>();
			return *this;
		}
		virtual bool operator==(const i_custom_type& aRhs) const
		{
			return instance_ptr() == aRhs.instance_ptr() || (instance_ptr() != 0 && aRhs.instance_ptr() != 0 && instance_as<AbstractType>() == aRhs.instance_as<AbstractType>());
		}
		virtual bool operator<(const i_custom_type& aRhs) const
		{
			return (instance_ptr() != 0 && aRhs.instance_ptr() != 0 && instance_as<AbstractType>() < aRhs.instance_as<AbstractType>()) || (instance_ptr() < aRhs.instance_ptr());
		}
	public:
		virtual const void* instance_ptr() const { return iInstance != boost::none ? static_cast<const AbstractType*>(&*iInstance) : static_cast<const AbstractType*>(0); }
		virtual void* instance_ptr() { return iInstance != boost::none ? static_cast<AbstractType*>(&*iInstance) : static_cast<AbstractType*>(0); }
	private:
		string iName;
		container_type iInstance;
	};
}
