// any_ref.hpp
/*
 *  Copyright (c) 2012-present, Leigh Johnston.
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
#include "align.hpp"

namespace neolib
{
	struct any_const_ref_bad_cast : public std::logic_error 
	{ 
		any_const_ref_bad_cast() : std::logic_error("neolib::any_const_ref_bad_cast") 
		{
		} 
	};

	class any_const_ref_holder_base
	{
		// construction
	public:
		virtual ~any_const_ref_holder_base() {}
		// operations
	public:
		template <typename T>
		operator const T&() const
		{
			if (typeid(T) != type()) 
				throw any_const_ref_bad_cast();
			return *static_cast<const T*>(ptr());
		}
		template <typename T>
		bool is() const { return typeid(T) == type(); }
		virtual any_const_ref_holder_base* clone(void* aWhere) const = 0;
		// implementation
	private:
		virtual const std::type_info& type() const = 0;
		virtual const void* ptr() const = 0;
	};

	template <typename T>
	class any_const_ref_holder : public any_const_ref_holder_base
	{
		// construction
	public:
		any_const_ref_holder(const T& aRef) : iRef(aRef) {}
		// implementation
	private:
		virtual any_const_ref_holder_base* clone(void* aWhere) const { return new (aWhere) any_const_ref_holder(iRef); }
		virtual const std::type_info& type() const { return typeid(T); }
		virtual const void* ptr() const { return &iRef; }
		// attributes
	private:
		const T& iRef;
	};

	struct any_ref_bad_cast : public std::logic_error 
	{ 
		any_ref_bad_cast() : std::logic_error("neolib::any_ref_bad_cast") 
		{
		} 
	};

	class any_ref_holder_base
	{
		// construction
	public:
		virtual ~any_ref_holder_base() {}
		// operations
	public:
		template <typename T>
		operator T&() const
		{
			if (typeid(T) != type()) 
				throw any_ref_bad_cast();
			return *static_cast<T*>(ptr());
		}
		template <typename T>
		bool is() const { return typeid(T) == type(); }
		virtual any_ref_holder_base* clone(void* aWhere) const = 0;
		virtual any_const_ref_holder_base* const_clone(void* aWhere) const = 0;
		// implementation
	private:
		virtual const std::type_info& type() const = 0;
		virtual void* ptr() const = 0;
	};

	template <typename T>
	class any_ref_holder : public any_ref_holder_base
	{
		// construction
	public:
		any_ref_holder(T& aRef) : iRef(aRef) {}
		// implementation
	private:
		virtual any_ref_holder_base* clone(void* aWhere) const { return new (aWhere) any_ref_holder(iRef); }
		virtual any_const_ref_holder_base* const_clone(void* aWhere) const { return new (aWhere) any_const_ref_holder<T>(const_cast<const T&>(iRef)); }
		virtual const std::type_info& type() const { return typeid(T); }
		virtual void* ptr() const { return &iRef; }
		// attributes
	private:
		T& iRef;
	};

	class any_ref
	{
		friend class any_const_ref;
		// construction
	public:
		any_ref() : iHolder(0) {}
		template <typename T>
		any_ref(T& aObject) : iHolder(new (iSpace.iBytes) any_ref_holder<T>(aObject)) {}
		any_ref(const any_ref& aOther) : iHolder(aOther.iHolder ? aOther.iHolder->clone(iSpace.iBytes) : 0) {}
		any_ref(any_ref& aOther) : iHolder(aOther.iHolder ? aOther.iHolder->clone(iSpace.iBytes) : 0) {}
		~any_ref() { destroy(); }
		any_ref& operator=(const any_ref& aOther) 
		{ 
			destroy(); 
			if (aOther.iHolder) 
				iHolder = aOther.iHolder->clone(iSpace.iBytes); 
			return *this;
		}
		// operations
	public:
		template <typename T>
		operator T&() const { if (empty()) throw any_ref_bad_cast(); return *iHolder; }
		template <typename T>
		bool is() const { return iHolder && iHolder->is<T>(); }
		bool something() const { return iHolder != 0; }
		bool empty() const { return !something(); }
		void reset() { destroy(); }
		// implementation
	private:
		void destroy() { if (iHolder != 0) iHolder->~any_ref_holder_base(); iHolder = 0; }
		// attributes
	private:
		union
		{
			max_align alignTo;
			char iBytes[sizeof(any_ref_holder<char>)];
		} iSpace;
		any_ref_holder_base* iHolder;
	};

	class any_const_ref
	{
		// construction
	public:
		any_const_ref() : iHolder(0) {}
		template <typename T>
		any_const_ref(const T& aObject) : iHolder(new (iSpace.iBytes) any_const_ref_holder<T>(aObject)) {}
		any_const_ref(const any_ref& aOther) : iHolder(aOther.iHolder ? aOther.iHolder->const_clone(iSpace.iBytes) : 0) {}
		any_const_ref(const any_const_ref& aOther) : iHolder(aOther.iHolder ? aOther.iHolder->clone(iSpace.iBytes) : 0) {}
		any_const_ref(any_const_ref& aOther) : iHolder(aOther.iHolder ? aOther.iHolder->clone(iSpace.iBytes) : 0) {}
		~any_const_ref() { destroy(); }
		any_const_ref& operator=(const any_const_ref& aOther) 
		{ 
			destroy(); 
			if (aOther.iHolder) 
				iHolder = aOther.iHolder->clone(iSpace.iBytes); 
			return *this;
		}
		// operations
	public:
		template <typename T>
		operator const T&() const { if (empty()) throw any_const_ref_bad_cast(); return *iHolder; }
		template <typename T>
		bool is() const { return iHolder && iHolder->is<T>(); }
		bool something() const { return iHolder != 0; }
		bool empty() const { return !something(); }
		void reset() { destroy(); }
		// implementation
	private:
		void destroy() { if (iHolder != 0) iHolder->~any_const_ref_holder_base(); iHolder = 0; }
		// attributes
	private:
		union
		{
			max_align alignTo;
			char iBytes[sizeof(any_const_ref_holder<char>)];
		} iSpace;
		any_const_ref_holder_base* iHolder;
	};
}
