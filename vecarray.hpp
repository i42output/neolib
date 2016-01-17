// vecarray.hpp - v3.0.1
/*
 *  Copyright (c) 2014 Leigh Johnston.
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
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <vector>
#include "align.hpp"

namespace neolib
{
	typedef std::true_type true_type;
	typedef std::false_type false_type;
	
	struct vecarray_overflow : public std::exception
	{
		virtual const char* what() const throw() { return "neolib::vecarray_overflow"; }
	};

	struct nocheck
	{
		inline static void test(bool aValid)
		{
			(void)aValid;
			assert(aValid);
		}
	};

	template <typename Exception>
	struct check
	{
		inline static void test(bool aValid)
		{
			if (!aValid)
				throw Exception();
		}
	};

	namespace detail
	{
		template <typename T, typename IsScalar>
		struct vecarray_scalar_trait_impl
		{ 
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef IsScalar type;
			typedef const T& parameter_type; 
			inline static void construct(pointer object, parameter_type value)
			{
				new (static_cast<void*>(object)) T(value);
			}
			inline static void destruct(pointer object)
			{
				object->~T();
			}
			inline static void destruct(pointer first, pointer last)
			{
				while(first != last)
				{
					first->~T();
					++first;
				}
			}
			template <typename InputIter, typename ForwardIter>
			inline static void uninitialized_copy(InputIter first, InputIter last, ForwardIter dest)
			{
				std::uninitialized_copy(first, last, dest);
			}
			template <typename InputIter1, typename InputIter2, typename ForwardIter1, typename ForwardIter2>
			inline static void uninitialized_copy2(InputIter1 first1, InputIter1 last1, ForwardIter1 dest1, InputIter2 first2, InputIter2 last2, ForwardIter2 dest2)
			{
				std::uninitialized_copy(first1, last1, dest1);
				try
				{
					std::uninitialized_copy(first2, last2, dest2);
				}
				catch(...)
				{
					destruct(dest1, dest1 + (last1 - first1));
					throw;
				}
			}
		};
		template <typename T>
		struct vecarray_scalar_trait_impl<T, true_type>
		{ 
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef true_type type;
			typedef T parameter_type; 
			inline static void construct(pointer object, parameter_type value)
			{
				*object = value;
			}
			inline static void destruct(pointer)
			{
			}
			inline static void destruct(pointer, pointer)
			{
			}
			template <typename InputIter, typename ForwardIter>
			inline static void uninitialized_copy(InputIter first, InputIter last, ForwardIter dest)
			{
				std::uninitialized_copy(first, last, dest);
			}
			template <typename InputIter1, typename InputIter2, typename ForwardIter1, typename ForwardIter2>
			inline static void uninitialized_copy2(InputIter1 first1, InputIter1 last1, ForwardIter1 dest1, InputIter2 first2, InputIter2 last2, ForwardIter2 dest2)
			{
				std::uninitialized_copy(first1, last1, dest1);
				std::uninitialized_copy(first2, last2, dest2);
			}
		};
	}

	template<typename T, std::size_t ArraySize, std::size_t MaxVectorSize = ArraySize, typename CheckPolicy = check<vecarray_overflow>, typename Alloc = std::allocator<T> >
	class vecarray
	{
	public:
		struct iterator_invalid : std::logic_error { iterator_invalid() : std::logic_error("neolib::vecarray::iterator_invalid") {} };
	private:
		enum iterator_type_e { ArrayIterator, VectorIterator };
	public:
		typedef T value_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef Alloc allocator_type;
		typedef std::vector<value_type, allocator_type> vector_type;
		class const_iterator;
		class iterator : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference>
		{
			friend class const_iterator;
		public:
			iterator() : iType(ArrayIterator), iArrayPtr(0), iVectorIter()
			{
			}
			iterator(const iterator& aOther) : iType(aOther.iType), iArrayPtr(aOther.iArrayPtr), iVectorIter(aOther.iVectorIter)
			{
			}
			iterator(T* aPtr) : iType(ArrayIterator), iArrayPtr(aPtr), iVectorIter()
			{
			}
			iterator(typename vector_type::iterator aIter) : iType(VectorIterator), iArrayPtr(0), iVectorIter(aIter)
			{
			}
		public:
			iterator& operator++()
			{
				if (iType == ArrayIterator)
					++iArrayPtr;
				else
					++iVectorIter;
				return *this;
			}
			iterator& operator--()
			{
				if (iType == ArrayIterator)
					--iArrayPtr;
				else
					--iVectorIter;
				return *this;
			}
			iterator operator++(int) { iterator ret(*this); operator++(); return ret; }
			iterator operator--(int) { iterator ret(*this); operator--(); return ret; }
			iterator& operator+=(difference_type aDifference)
			{
				if (aDifference < 0)
					return operator-=(-aDifference);
				if (iType == ArrayIterator)
					iArrayPtr += aDifference;
				else
					iVectorIter += aDifference;
				return *this;
			}
			iterator& operator-=(difference_type aDifference)
			{
				if (aDifference < 0)
					return operator+=(-aDifference);
				if (iType == ArrayIterator)
					iArrayPtr -= aDifference;
				else
					iVectorIter -= aDifference;
				return *this;
			}
			iterator operator+(difference_type aDifference) const { iterator result(*this); result += aDifference; return result; }
			iterator operator-(difference_type aDifference) const { iterator result(*this); result -= aDifference; return result; }
			reference operator[](difference_type aDifference) const { return *((*this) + aDifference); }
			difference_type operator-(const iterator& aOther) const 
			{ 
				if (iType == ArrayIterator)
					return iArrayPtr - aOther.iArrayPtr;
				else
					return iVectorIter - aOther.iVectorIter;
			}
			reference operator*() const
			{
				if (iType == ArrayIterator)
					return *iArrayPtr;
				else
					return *iVectorIter;
			}
			pointer operator->() const { return &operator*(); }
			bool operator==(const iterator& aOther) const 
			{ 
				if (iType == ArrayIterator)
					return iArrayPtr == aOther.iArrayPtr;
				else
					return iVectorIter == aOther.iVectorIter;
			}
			bool operator!=(const iterator& aOther) const
			{
				if (iType == ArrayIterator)
					return iArrayPtr != aOther.iArrayPtr;
				else
					return iVectorIter != aOther.iVectorIter;
			}
			bool operator<(const iterator& aOther) const
			{
				if (iType == ArrayIterator)
					return iArrayPtr < aOther.iArrayPtr;
				else
					return iVectorIter < aOther.iVectorIter;
			}
		public:
			T* array_ptr() const
			{
				if (iType == ArrayIterator)
					return iArrayPtr;
				else
					throw iterator_invalid();
			}
			typename vector_type::iterator vector_iter() const
			{
				if (iType == VectorIterator)
					return iVectorIter;
				else
					throw iterator_invalid();
			}
		private:
			iterator_type_e iType;
			T* iArrayPtr;
			typename vector_type::iterator iVectorIter;
		};
		class const_iterator : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference>
		{
		public:
			const_iterator() : iType(ArrayIterator), iArrayPtr(0), iVectorIter()
			{
			}
			const_iterator(const const_iterator& aOther) : iType(aOther.iType), iArrayPtr(aOther.iArrayPtr), iVectorIter(aOther.iVectorIter)
			{
			}
			const_iterator(const typename vecarray::iterator& aOther) : iType(aOther.iType), iArrayPtr(aOther.iArrayPtr), iVectorIter(aOther.iVectorIter)
			{
			}
			const_iterator(const T* aPtr) : iType(ArrayIterator), iArrayPtr(aPtr), iVectorIter()
			{
			}
			const_iterator(typename vector_type::const_iterator aIter) : iType(VectorIterator), iArrayPtr(0), iVectorIter(aIter)
			{
			}
			const_iterator(typename vector_type::iterator aIter) : iType(VectorIterator), iArrayPtr(0), iVectorIter(aIter)
			{
			}
		public:
			const_iterator& operator++()
			{
				if (iType == ArrayIterator)
					++iArrayPtr;
				else
					++iVectorIter;
				return *this;
			}
			const_iterator& operator--()
			{
				if (iType == ArrayIterator)
					--iArrayPtr;
				else
					--iVectorIter;
				return *this;
			}
			const_iterator operator++(int) { const_iterator ret(*this); operator++(); return ret; }
			const_iterator operator--(int) { const_iterator ret(*this); operator--(); return ret; }
			const_iterator& operator+=(difference_type aDifference)
			{
				if (aDifference < 0)
					return operator-=(-aDifference);
				if (iType == ArrayIterator)
					iArrayPtr += aDifference;
				else
					iVectorIter += aDifference;
				return *this;
			}
			const_iterator& operator-=(difference_type aDifference)
			{
				if (aDifference < 0)
					return operator+=(-aDifference);
				if (iType == ArrayIterator)
					iArrayPtr -= aDifference;
				else
					iVectorIter -= aDifference;
				return *this;
			}
			const_iterator operator+(difference_type aDifference) const { const_iterator result(*this); result += aDifference; return result; }
			const_iterator operator-(difference_type aDifference) const { const_iterator result(*this); result -= aDifference; return result; }
			const_reference operator[](difference_type aDifference) const { return *((*this) + aDifference); }
			difference_type operator-(const const_iterator& aOther) const
			{
				if (iType == ArrayIterator)
					return iArrayPtr - aOther.iArrayPtr;
				else
					return iVectorIter - aOther.iVectorIter;
			}
			const_reference operator*() const
			{
				if (iType == ArrayIterator)
					return *iArrayPtr;
				else
					return *iVectorIter;
			}
			const_pointer operator->() const { return &operator*(); }
			bool operator==(const const_iterator& aOther) const
			{
				if (iType == ArrayIterator)
					return iArrayPtr == aOther.iArrayPtr;
				else
					return iVectorIter == aOther.iVectorIter;
			}
			bool operator!=(const const_iterator& aOther) const
			{
				if (iType == ArrayIterator)
					return iArrayPtr != aOther.iArrayPtr;
				else
					return iVectorIter != aOther.iVectorIter;
			}
			bool operator<(const const_iterator& aOther) const
			{
				if (iType == ArrayIterator)
					return iArrayPtr < aOther.iArrayPtr;
				else
					return iVectorIter < aOther.iVectorIter;
			}
		public:
			const T* array_ptr() const
			{
				if (iType == ArrayIterator)
					return iArrayPtr;
				else
					throw iterator_invalid();
			}
			typename vector_type::const_iterator vector_iter() const
			{
				if (iType == VectorIterator)
					return iVectorIter;
				else
					throw iterator_invalid();
			}
		private:
			iterator_type_e iType;
			const T* iArrayPtr;
			typename vector_type::const_iterator iVectorIter;
		};
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	private:
		typedef detail::vecarray_scalar_trait_impl<T, typename std::is_scalar<T>::type> scalar_trait;
	public:
		typedef typename scalar_trait::parameter_type parameter_type;
	
	private:
		union
		{
			max_align alignTo;
			char iData[sizeof(T) * ArraySize];
			char iVector[sizeof(vector_type)];
		} iAlignedBuffer;
		size_type iSize;
		static const size_type USING_VECTOR = static_cast<size_type>(-1);
	
	public:
		// construction
		vecarray() : iSize(0)
		{
		}
		vecarray(const vecarray& rhs) : iSize(0)
		{
			insert(begin(), rhs.begin(), rhs.end());
		}
		template <typename T2, std::size_t N2>
		vecarray(const vecarray<T2, N2>& rhs) : iSize(0)
		{
			insert(begin(), rhs.begin(), rhs.end());
		}
		vecarray(size_type n) : iSize(0)
		{
			insert(begin(), n, value_type());
		}
		vecarray(size_type n, parameter_type value) : iSize(0)
		{
			insert(begin(), n, value);
		}
		template<typename InputIterator>
		vecarray(InputIterator first, InputIterator last) : iSize(0)
		{
			insert(begin(), first, last);
		}
		~vecarray()
		{
			clear();
			if (using_vector())
			{
				vector().~vector_type();
			}
		}
		// traversals
		iterator begin() { return using_array() ? iterator(reinterpret_cast<pointer>(iAlignedBuffer.iData)) : iterator(vector().begin()); }
		const_iterator begin() const { return using_array() ? const_iterator(reinterpret_cast<const_pointer>(iAlignedBuffer.iData)) : const_iterator(vector().begin()); }
		iterator end() { return using_array() ? iterator(reinterpret_cast<pointer>(iAlignedBuffer.iData) + iSize) : iterator(vector().end()); }
		const_iterator end() const { return using_array() ? const_iterator(reinterpret_cast<const_pointer>(iAlignedBuffer.iData) + iSize) : const_iterator(vector().end()); }
		reverse_iterator rbegin() { return reverse_iterator(end()); }
		const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
		reverse_iterator rend() { return reverse_iterator(begin()); }
		const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
		bool empty() const { return size() == 0; }
		bool full() const { return size() == MaxVectorSize; }
		size_type size() const { return using_array() ? iSize : vector().size(); }
		size_type available() const { return MaxVectorSize - size(); }
		size_type capacity() const { return MaxVectorSize; }
		size_type max_size() const { return MaxVectorSize; }
		size_type after(size_type position) const { return position < size() ? size() - position : 0; }
		// element access
		reference operator[](size_type n) { return *(begin() + n); }
		const_reference operator[](size_type n) const { return *(begin() + n); }
		reference at(size_type n) { if (n < size()) return operator[](n); throw std::out_of_range("vecarray::at"); }
		const_reference at(size_type n) const { if (n < size()) return operator[](n); throw std::out_of_range("vecarray::at"); }
		reference front() { return *begin(); }
		reference back() { return *(begin() + (size() - 1)); }
		const_reference front() const { return *begin(); }
		const_reference back() const { return *(begin() + (size() - 1)); }
		// modifiers
		vecarray& operator=(const vecarray& rhs)
		{
			if (&rhs != this)
				assign(rhs.begin(), rhs.end());
			return *this;
		}
		template<typename T2, std::size_t N2>
		vecarray& operator=(const vecarray<T2, N2>& rhs)
		{
			if (static_cast<const void*>(&rhs) != static_cast<const void*>(this))
				assign(rhs.begin(), rhs.end());
			return *this;
		}
		template <typename InputIterator>
		void assign(InputIterator first, InputIterator last)
		{
			clear();
			insert(begin(), first, last);
		}
		void assign(size_type n, parameter_type value)
		{
			clear();
			insert(begin(), n, value);
		}
		template <class InputIterator>
		typename std::enable_if<!std::is_integral<InputIterator>::value, void>::type
		insert(const_iterator position, InputIterator first, InputIterator last)
		{
			do_insert(position, first, last);
		}
		iterator insert(const_iterator position, parameter_type value)
		{
			need(1, position);
			size_type index = position - const_iterator(vector().begin());
			insert(position, 1, value);
			return begin() + index;
		}
		void insert(const_iterator position, size_type count, const value_type& value)
		{
			need(count, position);
			if (using_array())
			{
				const_iterator next = position;
				while (count > 0)
				{
					insert(next++, &value, &value + 1);
					--count;
				}
			}
			else
			{
				vector().insert(remove_const(position.vector_iter()), count, value);
			}
		}
		iterator erase(const_iterator position)
		{ 
			if (using_array())
			{
				assert(iSize > 0);
				iterator last = end(); --last;
				std::copy(position + 1, const_iterator(end()), const_cast<pointer>(position.array_ptr()));
				scalar_trait::destruct(last.array_ptr());
				--iSize;
				return const_cast<pointer>(position.array_ptr());
			}
			else
			{
				return vector().erase(remove_const(position.vector_iter()));
			}
		}
		iterator erase(const_iterator first, const_iterator last) 
		{ 
			if (using_array())
			{
				if (last != end())
					std::copy(last, const_iterator(end()), const_cast<pointer>(first.array_ptr()));
				scalar_trait::destruct(end().array_ptr() - (last - first), end().array_ptr());
				iSize -= (last - first);
				return const_cast<pointer>(first.array_ptr());
			}
			else
			{
				return vector().erase(remove_const(first.vector_iter()), remove_const(last.vector_iter()));
			}
		}
		void clear()
		{
			erase(begin(), end());
		}
		void push_back(parameter_type value)
		{
			do_push_back(value, typename scalar_trait::type());
		}
		void pop_back()
		{
			erase(end() - 1);
		}
		void remove(parameter_type value, bool multiple = true)
		{
			for (iterator i = begin(); i != end(); )
			{
				if (*i == value)
				{
					erase(i);
					if (!multiple)
						return;
				}
				else
					++i;
			}
		}
		void resize(size_type n)
		{
			if (using_array())
			{
				if (size() > n)
					erase(begin() + n, end());
				else if (size() < n)
					insert(end(), n - size(), value_type());
			}
			else
				vector().resize(n);
		}
		void resize(size_type n, parameter_type value)
		{
			if (using_array())
			{
				if (size() > n)
					erase(begin() + n, end());
				else if (size() < n)
					insert(end(), n - size(), value);
			}
			else
				vector().resize(n, value);
		}
		template<typename T2, std::size_t ArraySize2, std::size_t MaxVectorSize2, typename CheckPolicy2, typename Alloc2>
		void swap(vecarray<T2, ArraySize2, MaxVectorSize2, CheckPolicy2, Alloc2>& rhs)
		{
			vecarray tmp = rhs;
			rhs = *this;
			*this = tmp;
		}
		// equality
		template<typename T2, std::size_t ArraySize2, std::size_t MaxVectorSize2, typename CheckPolicy2, typename Alloc2>
		bool operator==(const vecarray<T2, ArraySize2, MaxVectorSize2, CheckPolicy2, Alloc2>& rhs) const
		{
			return rhs.size() == size() && std::equal(begin(), end(), rhs.begin());
		}
		template<typename T2, std::size_t ArraySize2, std::size_t MaxVectorSize2, typename CheckPolicy2, typename Alloc2>
		bool operator!=(const vecarray<T2, ArraySize2, MaxVectorSize2, CheckPolicy2, Alloc2>& rhs) const
		{
			return !operator==(rhs);
		}

	private:
		bool using_array() const
		{
			return iSize != USING_VECTOR;
		}
		bool using_vector() const
		{
			return iSize == USING_VECTOR;
		}
		vector_type& vector()
		{
			return reinterpret_cast<vector_type&>(iAlignedBuffer.iVector);
		}
		const vector_type& vector() const
		{
			return reinterpret_cast<const vector_type&>(iAlignedBuffer.iVector);
		}
		void need(size_type aAmount)
		{
			if (using_array() && size() + aAmount > ArraySize)
				convert();
		}
		template <typename Iter>
		void need(size_type aAmount, Iter& aIter)
		{
			if (using_array() && size() + aAmount > ArraySize)
			{
				size_type index = aIter - begin();
				convert();
				aIter = begin() + index;
			}
		}
		void convert()
		{
			if (using_array())
			{
				vecarray copy = *this;
				clear();
				new (iAlignedBuffer.iVector) vector_type(copy.begin(), copy.end());
				iSize = USING_VECTOR;
			}
		}
		template <class InputIterator>
		typename std::enable_if<!std::is_same<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>::value, void>::type
		do_insert(const_iterator position, InputIterator first, InputIterator last)
		{
			difference_type n = last - first;
			CheckPolicy::test(size() + n <= MaxVectorSize);
			need(n, position);
			if (using_array())
			{
				const_iterator theEnd = end();
				difference_type t = theEnd - position;
				if (t > 0)
				{
					if (t > n)
					{
						scalar_trait::uninitialized_copy(theEnd - n, theEnd, const_cast<pointer>(theEnd.array_ptr()));
						iSize += n;
						std::copy_backward(position, theEnd - n, const_cast<pointer>(theEnd.array_ptr()));
						std::copy(first, last, const_cast<pointer>(position.array_ptr()));
					}
					else
					{
						scalar_trait::uninitialized_copy2(theEnd - t, theEnd, const_cast<pointer>((theEnd + (n - t)).array_ptr()), first + t, last, const_cast<pointer>(theEnd.array_ptr()));
						iSize += n;
						std::copy(first, first + t, const_cast<pointer>(position.array_ptr()));
					}
				}
				else
				{
					scalar_trait::uninitialized_copy(first, last, const_cast<pointer>(theEnd.array_ptr()));
					iSize += n;
				}
			}
			else
			{
				vector().insert(remove_const(position.vector_iter()), first, last);
			}
		}
		template <class InputIterator>
		typename std::enable_if<std::is_same<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>::value, void>::type
		do_insert(const_iterator position, InputIterator first, InputIterator last)
		{
			const_iterator next = position;
			while (first != last)
			{
				insert(next++, 1, *first++);
			}
			const_cast<pointer>(position);
		}
		void do_push_back(parameter_type value, false_type)
		{
			CheckPolicy::test(size() < MaxVectorSize);
			need(1);
			if (using_array())
			{
				scalar_trait::construct(end().array_ptr(), value);
				++iSize;
			}
			else
				vector().push_back(value);
		}
		void do_push_back(parameter_type value, true_type)
		{
			CheckPolicy::test(size() < MaxVectorSize);
			need(1);
			if (using_array())
			{
				*end() = value;
				++iSize;
			}
			else
				vector().push_back(value);
		}
		typename vector_type::iterator remove_const(typename vector_type::const_iterator aIter)
		{
			return vector().begin() + (aIter - typename vector_type::const_iterator(vector().begin()));
		}
	};
}
