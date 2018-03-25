// any.hpp
/*
 *  Copyright (c) 2007-present, Leigh Johnston.
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
#include <stdexcept>
#include <typeinfo>

namespace neolib
{
	class any
	{
		// types
	public:
		struct bad_cast : public std::logic_error { bad_cast() : std::logic_error("neolib::any::bad_cast") {} };
	private:
		class holder_base
		{
			// construction
		public:
			virtual ~holder_base() {}
			// operations
		public:
			template <typename T>
			operator const T&() const
			{
				if (typeid(T) != type()) 
					throw bad_cast();
				return *static_cast<const T*>(ptr());
			}
			template <typename T>
			operator T&()
			{
				if (typeid(T) != type()) 
					throw bad_cast();
				return *static_cast<T*>(ptr());
			}
			template <typename T>
			bool is() const { return typeid(T) == type(); }
			virtual holder_base* clone() const = 0;
			// implementation
		private:
			virtual const std::type_info& type() const = 0;
			virtual const void* ptr() const = 0;
			virtual void* ptr() = 0;
		};
		template <typename T>
		class holder : public holder_base
		{
			// construction
		public:
			holder(const T& aObject) : iObject(aObject) {}
			// implementation
		private:
			virtual holder_base* clone() const { return new holder(iObject); }
			virtual const std::type_info& type() const { return typeid(T); }
			virtual const void* ptr() const { return &iObject; }
			virtual void* ptr() { return &iObject; }
			// attributes
		private:
			T iObject;
		};
		// construction
	public:
		any() : iHolder(0) {}
		template <typename T>
		any(const T& aObject) : iHolder(new holder<T>(aObject)) {}
		any(const any& aOther) : iHolder(aOther.iHolder ? aOther.iHolder->clone() : 0) {}
		~any() { destroy(); }
		any& operator=(const any& aOther) 
		{ 
			destroy(); 
			if (aOther.iHolder) 
				iHolder = aOther.iHolder->clone(); 
			return *this;
		}
		// operations
	public:
		template <typename T>
		operator const T&() const { if (empty()) throw bad_cast(); return *iHolder; }
		template <typename T>
		operator T&() { if (empty()) throw bad_cast(); return *iHolder; }
		template <typename T>
		bool is() const { return iHolder && iHolder->is<T>(); }
		bool something() const { return iHolder != 0; }
		bool empty() const { return !something(); }
		void reset() { destroy(); }
		// implementation
	private:
		void destroy() { delete iHolder; iHolder = 0; }
		// attributes
	private:
		holder_base* iHolder;
	};
}

#include "any_ref.hpp"
#include "any_predicate.hpp"
#include "any_iterator.hpp"