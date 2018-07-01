// optional.hpp - v1.3
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

#include "neolib.hpp"
#include <boost/optional.hpp>
#include "reference_counted.hpp"
#include "i_optional.hpp"

namespace neolib
{
	template<typename T, typename ConcreteType = T>
	class optional : public reference_counted<i_optional<T> >, public boost::optional<ConcreteType>
	{
		// types
	public:
		typedef T abstract_type;
		typedef ConcreteType value_type;
		typedef ConcreteType* pointer;
		typedef const ConcreteType* const_pointer;
		typedef ConcreteType& reference;
		typedef const ConcreteType& const_reference;
		typedef typename i_optional<T>::not_valid not_valid;
	private:
		typedef i_optional<T> abstract_base;
		typedef boost::optional<ConcreteType> base;
		// construction
	public:
		optional() {}
		optional(const abstract_base& rhs) : base(rhs.get()) {}
		optional(const_reference value) : base(value) {}
		// state
	public:
		virtual bool valid() const
		{
			return static_cast<const base&>(*this) != boost::none;
		}
		virtual bool invalid() const
		{
			return !valid();
		}
		virtual operator bool() const 
		{ 
			return valid();
		}
		// element access
	public:
		virtual reference get()
		{
			if (valid())
				return base::get();
			throw not_valid();
		}
		virtual const_reference get() const
		{
			if (valid())
				return base::get();
			throw not_valid();
		}
		virtual reference operator*()
		{ 
			return get();
		}
		virtual const_reference operator*() const
		{ 
			return get();
		}
		virtual pointer operator->()
		{ 
			return &get(); 
		}
		virtual const_pointer operator->() const
		{ 
			return &get(); 
		}
		// modifiers
	public:
		virtual void reset()
		{ 
			static_cast<base&>(*this) = boost::none;
		}
		virtual optional& operator=(const boost::none_t&)
		{
			static_cast<base&>(*this) = boost::none;
			return *this;
		}
		virtual optional& operator=(const abstract_base& rhs)
		{ 
			*this = rhs.get();
			return *this;
		}
		virtual optional& operator=(const abstract_type& value) 
		{
			static_cast<base&>(*this) = value;
			return *this;
		}
		void swap(optional& rhs)
		{
			base::swap(rhs);
		}
	};

	template <typename T, typename ConcreteType>
	inline bool operator<(const optional<T, ConcreteType>& lhs, const optional<T, ConcreteType>& rhs)
	{
		if (lhs.valid() != rhs.valid())
			return lhs.valid() < rhs.valid();
		if (!lhs.valid())
			return false;
		return lhs.get() < rhs.get();
	}

	template <typename T, typename ConcreteType>
	inline bool operator==(const optional<T, ConcreteType>& lhs, const optional<T, ConcreteType>& rhs)
	{
		if (lhs.valid() != rhs.valid())
			return false;
		if (lhs.valid())
			return *lhs == *rhs;
		else
			return true;
	}

	template <typename T, typename ConcreteType>
	inline bool operator!=(const optional<T, ConcreteType>& lhs, const optional<T, ConcreteType>& rhs)
	{
		return !operator==(lhs, rhs);
	}
}
