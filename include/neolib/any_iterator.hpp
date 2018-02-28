// any_iterator.hpp
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

#include <stdexcept>
#include <typeinfo>

namespace neolib
{
	class any_iterator;

	class any_const_iterator
	{
		// types
	public:
		struct bad_cast : public std::logic_error { bad_cast() : std::logic_error("neolib::any_const_iterator::bad_cast") {} };
		struct is_empty : public std::logic_error { is_empty() : std::logic_error("neolib::any_const_iterator::is_empty") {} };
	private:
		class holder_base
		{
			// construction
		public:
			virtual ~holder_base() {}
			// operations
		public:
			virtual void operator++() = 0;
			virtual void operator--() = 0;
			virtual bool operator==(const any_const_iterator& aOther) const = 0;
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
			virtual holder_base* clone(any_const_iterator& aOwner) const = 0;
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
			virtual void operator++() { ++iObject; }
			virtual void operator--() { --iObject; }
			virtual bool operator==(const any_const_iterator& aOther) const 
			{
				return iObject == static_cast<const T&>(aOther); 
			}
			virtual holder_base* clone(any_const_iterator& aOwner) const { return aOwner.create(iObject); }
			virtual const std::type_info& type() const { return typeid(T); }
			virtual const void* ptr() const { return &iObject; }
			virtual void* ptr() { return &iObject; }
			// attributes
		private:
			T iObject;
		};
		// construction
	public:
		any_const_iterator() : iHolder(0) {}
		template <typename T>
		any_const_iterator(const T& aObject) : iHolder(create(aObject)) {}
		any_const_iterator(const any_const_iterator& aOther) : iHolder(aOther.iHolder ? aOther.iHolder->clone(*this) : 0) {}
		~any_const_iterator() { destroy(); }
		any_const_iterator& operator=(const any_const_iterator& aOther) 
		{ 
			destroy(); 
			if (aOther.iHolder) 
				iHolder = aOther.iHolder->clone(*this); 
			return *this;
		}
	private:
		any_const_iterator(const any_iterator&); // not allowed, use any_const_iterator_cast instead
		// operations
	public:
		any_const_iterator& operator++()
		{
			if (empty())
				throw is_empty();
			++(*iHolder);
			return *this;
		}
		any_const_iterator operator++(int)
		{
			if (empty())
				throw is_empty();
			any_const_iterator ret = *this;
			++ret;
			return ret;
		}
		any_const_iterator& operator--()
		{
			if (empty())
				throw is_empty();
			--(*iHolder);
			return *this;
		}
		any_const_iterator operator--(int)
		{
			if (empty())
				throw is_empty();
			any_const_iterator ret = *this;
			--ret;
			return ret;
		}
		friend bool operator==(const any_const_iterator& aLeft, const any_const_iterator& aRight)
		{
			if (aLeft.empty())
				throw is_empty();
			return aLeft.iHolder->operator==(aRight);
		}
		friend bool operator!=(const any_const_iterator& aLeft, const any_const_iterator& aRight)
		{
			return !(aLeft == aRight);
		}
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
		template <typename T>
		holder_base* create(const T& aData)
		{
			if (sizeof(T) <= sizeof(iSpace.iBytes))
				iHolder = new (iSpace.iBytes) holder<T>(aData);
			else
				iHolder = new holder<T>(aData);
			return iHolder;
		}
		void destroy() 
		{ 
			if (iHolder == reinterpret_cast<holder_base*>(iSpace.iBytes)) 
				iHolder->~holder_base(); 
			else 
				delete iHolder; 
			iHolder = 0; 
		}
		// attributes
	private:
		holder_base* iHolder;
		union
		{
			std::max_align_t alignTo;
			char iBytes[64];
		} iSpace;
	};

	class any_iterator
	{
		// types
	public:
		struct bad_cast : public std::logic_error { bad_cast() : std::logic_error("neolib::any_iterator::bad_cast") {} };
		struct is_empty : public std::logic_error { is_empty() : std::logic_error("neolib::any_iterator::is_empty") {} };
	private:
		class holder_base
		{
			// construction
		public:
			virtual ~holder_base() {}
			// operations
		public:
			virtual void operator++() = 0;
			virtual void operator--() = 0;
			virtual bool operator==(const any_iterator& aOther) const = 0;
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
			virtual holder_base* clone(any_iterator& aOwner) const = 0;
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
			virtual void operator++() { ++iObject; }
			virtual void operator--() { --iObject; }
			virtual bool operator==(const any_iterator& aOther) const 
			{
				return iObject == static_cast<const T&>(aOther); 
			}
			virtual holder_base* clone(any_iterator& aOwner) const { return aOwner.create(iObject); }
			virtual const std::type_info& type() const { return typeid(T); }
			virtual const void* ptr() const { return &iObject; }
			virtual void* ptr() { return &iObject; }
			// attributes
		private:
			T iObject;
		};
		// construction
	public:
		any_iterator() : iHolder(0) {}
		template <typename T>
		any_iterator(const T& aObject) : iHolder(new holder<T>(aObject)) {}
		any_iterator(const any_iterator& aOther) : iHolder(aOther.iHolder ? aOther.iHolder->clone(*this) : 0) {}
		~any_iterator() { destroy(); }
		any_iterator& operator=(const any_iterator& aOther) 
		{ 
			destroy(); 
			if (aOther.iHolder) 
				iHolder = aOther.iHolder->clone(*this); 
			return *this;
		}
		// operations
	public:
		any_iterator& operator++()
		{
			if (empty())
				throw is_empty();
			++(*iHolder);
			return *this;
		}
		any_iterator operator++(int)
		{
			if (empty())
				throw is_empty();
			any_iterator ret = *this;
			++ret;
			return ret;
		}
		any_iterator& operator--()
		{
			if (empty())
				throw is_empty();
			--(*iHolder);
			return *this;
		}
		any_iterator operator--(int)
		{
			if (empty())
				throw is_empty();
			any_iterator ret = *this;
			--ret;
			return ret;
		}
		friend bool operator==(const any_iterator& aLeft, const any_iterator& aRight)
		{
			if (aLeft.empty())
				throw is_empty();
			return aLeft.iHolder->operator==(aRight);
		}
		friend bool operator!=(const any_iterator& aLeft, const any_iterator& aRight)
		{
			return !(aLeft == aRight);
		}
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
		template <typename T>
		holder_base* create(const T& aData)
		{
			if (sizeof(T) <= sizeof(iSpace.iBytes))
				iHolder = new (iSpace.iBytes) holder<T>(aData);
			else
				iHolder = new holder<T>(aData);
			return iHolder;
		}
		void destroy() 
		{ 
			if (iHolder == reinterpret_cast<holder_base*>(iSpace.iBytes)) 
				iHolder->~holder_base(); 
			else 
				delete iHolder; 
			iHolder = 0; 
		}
		// attributes
	private:
		holder_base* iHolder;
		union
		{
			std::max_align_t alignTo;
			char iBytes[64];
		} iSpace;
	};

	template <typename Source, typename Target>
	any_const_iterator any_const_iterator_cast(any_iterator aSource)
	{
		return static_cast<Target>(static_cast<const Source&>(aSource));
	}
}

using neolib::any_const_iterator_cast;