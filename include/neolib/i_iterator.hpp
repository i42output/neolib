// i_iterator.hpp - v1.0
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
#include <iterator>
#include "i_reference_counted.hpp"

namespace neolib
{
	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	class iterator;
	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	class i_const_iterator;

	template <typename T, typename Category = std::bidirectional_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = T*, typename Reference = T&>
	class i_iterator : public std::iterator<Category, T, Difference, Pointer, Reference>, public i_reference_counted
	{
	public:
		typedef i_iterator<T, Category, Difference, Pointer, Reference> base_iterator;
		typedef T value_type;
		typedef Difference difference_type;
		typedef Pointer pointer;
		typedef Reference reference;
		typedef Category iterator_category;
		typedef neolib::iterator<T, Category, Difference, Pointer, Reference> iterator_wrapper;
	public:
		virtual i_iterator& operator++() = 0;
		virtual i_iterator& operator--() = 0;
		iterator_wrapper operator++(int);
		iterator_wrapper operator--(int);
		virtual reference operator*() const = 0;
		virtual pointer operator->() const = 0;
		virtual bool operator==(const i_iterator& aOther) const = 0;
		virtual bool operator!=(const i_iterator& aOther) const = 0;
	public:
		virtual i_iterator* clone() const = 0;
		virtual i_const_iterator<T, Category, Difference, const T*, const T&>* const_clone() const = 0;
	private:
		virtual i_iterator* do_post_increment() = 0;
		virtual i_iterator* do_post_decrement() = 0;
	};

	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	class random_access_iterator;
	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	class i_random_access_const_iterator;
	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	class random_access_const_iterator;

	template <typename T, typename Category = std::random_access_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = T*, typename Reference = T&>
	class i_random_access_iterator : public i_iterator<T, Category, Difference, Pointer, Reference>
	{
	public:
		typedef i_iterator<T, Category, Difference, Pointer, Reference> base_iterator;
		typedef typename base_iterator::value_type value_type;
		typedef typename base_iterator::difference_type difference_type;
		typedef typename base_iterator::pointer pointer;
		typedef typename base_iterator::reference reference;
		typedef Category iterator_category;
		typedef neolib::random_access_iterator<T, Category, Difference, Pointer, Reference> iterator_wrapper;
	public:
		virtual i_random_access_iterator& operator+=(difference_type aDifference) = 0;
		virtual i_random_access_iterator& operator-=(difference_type aDifference) = 0;
		iterator_wrapper operator+(difference_type aDifference) const;
		iterator_wrapper operator-(difference_type aDifference) const;
		virtual reference operator[](difference_type aDifference) const = 0;
		virtual difference_type operator-(const i_random_access_iterator& aOther) const = 0;
		virtual bool operator<(const i_random_access_iterator& aOther) const = 0;
	private:
		virtual i_random_access_iterator* do_add(difference_type aDifference) const = 0;
		virtual i_random_access_iterator* do_subtract(difference_type aDifference) const = 0;
	};

	template <typename T, typename Category = std::bidirectional_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = T*, typename Reference = T&>
	class iterator
	{
	public:
		typedef T value_type;
		typedef Difference difference_type;
		typedef Pointer pointer;
		typedef Reference reference;
		typedef Category iterator_category;
		typedef i_iterator<T, Category, Difference, Pointer, Reference> abstract_iterator;
	public:
		iterator() :
			iWrappedIterator(0)
		{
		}
		iterator(abstract_iterator* aWrappedIterator) :
			iWrappedIterator(aWrappedIterator)
		{
			if (iWrappedIterator != 0)
				iWrappedIterator->add_ref();
		}
		iterator(const iterator& aOther) :
			iWrappedIterator(aOther.clone())
		{
			if (iWrappedIterator != 0)
				iWrappedIterator->add_ref();
		}
		~iterator()
		{
			if (iWrappedIterator != 0)
				iWrappedIterator->release();
		}
		iterator& operator=(const iterator& aOther)
		{
			iterator temp(aOther);
			if (iWrappedIterator != 0)
				iWrappedIterator->release();
			iWrappedIterator = temp.iWrappedIterator;
			if (iWrappedIterator != 0)
				iWrappedIterator->add_ref();
			return *this;
		}
		operator abstract_iterator&()
		{
			return *wrapped_iterator();
		}
	public:
		abstract_iterator& operator++() { return ++(*iWrappedIterator); }
		abstract_iterator& operator--() { return --(*iWrappedIterator); }
		iterator operator++(int) { return (*iWrappedIterator)++; }
		iterator operator--(int) { return (*iWrappedIterator)--; }
		reference operator*() const { return (*iWrappedIterator).operator*(); }
		pointer operator->() const { return (*iWrappedIterator).operator->(); }
		bool operator==(const iterator& aOther) const { return iWrappedIterator != 0 && aOther.wrapped_iterator() != 0 && *iWrappedIterator == *aOther.wrapped_iterator(); }
		bool operator!=(const iterator& aOther) const { return !(*this == aOther); }
	public:
		abstract_iterator* wrapped_iterator() const { return iWrappedIterator; }
		abstract_iterator* clone() const
		{
			if (wrapped_iterator() != 0)
				return wrapped_iterator()->clone();
			else
				return 0;
		}
	protected:
		abstract_iterator* iWrappedIterator;
	};

	template <typename T, typename Category = std::random_access_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = T*, typename Reference = T&>
	class random_access_iterator : public iterator<T, Category, Difference, Pointer, Reference>
	{
	private:
		typedef iterator<T, Category, Difference, Pointer, Reference> base;
	public:
		typedef typename base::value_type value_type;
		typedef typename base::difference_type difference_type;
		typedef typename base::pointer pointer;
		typedef typename base::reference reference;
		typedef typename base::iterator_category iterator_category;
		typedef i_random_access_iterator<T, Category, Difference, Pointer, Reference> abstract_iterator;
		typedef random_access_const_iterator<T, Category, Difference, Pointer, Reference> const_iterator;
	public:
		random_access_iterator() :
			base()
		{
		}
		random_access_iterator(abstract_iterator* aWrappedIterator) :
			base(aWrappedIterator)
		{
		}
		random_access_iterator(const random_access_iterator& aOther) :
			base(aOther)
		{
		}
		~random_access_iterator()
		{
		}
		random_access_iterator& operator=(const random_access_iterator& aOther)
		{
			base::operator=(aOther);
			return *this;
		}
		operator abstract_iterator&()
		{
			return *wrapped_iterator();
		}
	public:
		abstract_iterator& operator+=(difference_type aDifference) { return (*wrapped_iterator()) += aDifference; }
		abstract_iterator& operator-=(difference_type aDifference) { return (*wrapped_iterator()) += aDifference; }
		random_access_iterator operator+(difference_type aDifference) const { return (*wrapped_iterator()) + aDifference; }
		random_access_iterator operator-(difference_type aDifference) const { return (*wrapped_iterator()) - aDifference; }
		reference operator[](difference_type aDifference) const { return (*wrapped_iterator())[aDifference]; }
		difference_type operator-(const random_access_iterator& aOther) const { return (*wrapped_iterator()) - (*aOther.wrapped_iterator()); }
		bool operator<(const random_access_iterator& aOther) const { return wrapped_iterator() != 0 && aOther.wrapped_iterator() != 0 && (*wrapped_iterator()) < *aOther.wrapped_iterator(); }
	public:
		abstract_iterator* wrapped_iterator() const { return static_cast<abstract_iterator*>(base::wrapped_iterator()); }
	};

	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	class const_iterator;

	template <typename T, typename Category = std::bidirectional_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = const T*, typename Reference = const T&>
	class i_const_iterator : public std::iterator<Category, T, Difference, Pointer, Reference>, public i_reference_counted
	{
	public:
		typedef i_const_iterator<T, Category, Difference, Pointer, Reference> base_iterator;
		typedef T value_type;
		typedef Difference difference_type;
		typedef Pointer pointer;
		typedef Reference reference;
		typedef Category iterator_category;
		typedef neolib::const_iterator<T, Category, Difference, Pointer, Reference> iterator_wrapper;
	public:
		virtual i_const_iterator& operator++() = 0;
		virtual i_const_iterator& operator--() = 0;
		iterator_wrapper operator++(int);
		iterator_wrapper operator--(int);
		virtual reference operator*() const = 0;
		virtual pointer operator->() const = 0;
		virtual bool operator==(const i_const_iterator& aOther) const = 0;
		virtual bool operator!=(const i_const_iterator& aOther) const = 0;
	public:
		virtual i_const_iterator* clone() const = 0;
	private:
		virtual i_const_iterator* do_post_increment() = 0;
		virtual i_const_iterator* do_post_decrement() = 0;
	};

	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	class random_access_const_iterator;

	template <typename T, typename Category = std::random_access_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = const T*, typename Reference = const T&>
	class i_random_access_const_iterator : public i_const_iterator<T, Category, Difference, Pointer, Reference>
	{
	public:
		typedef i_const_iterator<T, Category, Difference, Pointer, Reference> base_iterator;
		typedef typename base_iterator::value_type value_type;
		typedef typename base_iterator::difference_type difference_type;
		typedef typename base_iterator::pointer pointer;
		typedef typename base_iterator::reference reference;
		typedef typename base_iterator::iterator_category iterator_category;
		typedef neolib::random_access_const_iterator<T, Category, Difference, Pointer, Reference> iterator_wrapper;
	public:
		virtual i_random_access_const_iterator& operator+=(difference_type aDifference) = 0;
		virtual i_random_access_const_iterator& operator-=(difference_type aDifference) = 0;
		iterator_wrapper operator+(difference_type aDifference) const;
		iterator_wrapper operator-(difference_type aDifference) const;
		virtual reference operator[](difference_type aDifference) const = 0;
		virtual difference_type operator-(const i_random_access_const_iterator& aOther) const = 0;
		virtual bool operator<(const i_random_access_const_iterator& aOther) const = 0;
	private:
		virtual i_random_access_const_iterator* do_add(difference_type aDifference) const = 0;
		virtual i_random_access_const_iterator* do_subtract(difference_type aDifference) const = 0;
	};

	template <typename T, typename Category = std::bidirectional_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = const T*, typename Reference = const T&>
	class const_iterator
	{
	public:
		typedef T value_type;
		typedef Difference difference_type;
		typedef Pointer pointer;
		typedef Reference reference;
		typedef Category iterator_category;
		typedef i_const_iterator<T, Category, Difference, Pointer, Reference> abstract_iterator;
	public:
		const_iterator() :
			iWrappedIterator(0)
		{
		}
		const_iterator(abstract_iterator* aWrappedIterator) :
			iWrappedIterator(aWrappedIterator)
		{
			if (iWrappedIterator != 0)
				iWrappedIterator->add_ref();
		}
		const_iterator(const const_iterator& aOther) :
			iWrappedIterator(aOther.clone())
		{
			if (iWrappedIterator != 0)
				iWrappedIterator->add_ref();
		}
		const_iterator(const i_iterator<T, Category, difference_type, T*, T&>& aOther) :
			iWrappedIterator(0)
		{
			(iWrappedIterator = aOther.const_clone())->add_ref();
		}
		const_iterator(const iterator<T, Category, difference_type, T*, T&>& aOther) :
			iWrappedIterator(0)
		{
			if (aOther.wrapped_iterator() != 0)
				(iWrappedIterator = aOther.wrapped_iterator()->const_clone())->add_ref();
		}
		~const_iterator()
		{
			if (iWrappedIterator != 0)
				iWrappedIterator->release();
		}
		const_iterator& operator=(const const_iterator& aOther)
		{
			const_iterator temp(aOther);
			if (iWrappedIterator != 0)
				iWrappedIterator->release();
			iWrappedIterator = temp.iWrappedIterator;
			if (iWrappedIterator != 0)
				iWrappedIterator->add_ref();
			return *this;
		}
		const_iterator& operator=(const iterator<T, Category, difference_type, T*, T&>& aOther)
		{
			if (iWrappedIterator != 0)
				iWrappedIterator->release();
			if (aOther.wrapped_iterator() != 0)
				(iWrappedIterator = aOther.wrapped_iterator()->const_clone())->add_ref();
			else
				iWrappedIterator = 0;
			return *this;
		}
		operator abstract_iterator&()
		{
			return *wrapped_iterator();
		}
	public:
		abstract_iterator& operator++() { return ++(*iWrappedIterator); }
		abstract_iterator& operator--() { return --(*iWrappedIterator); }
		const_iterator operator++(int) { return (*iWrappedIterator)++; }
		const_iterator operator--(int) { return (*iWrappedIterator)--; }
		reference operator*() const { return (*iWrappedIterator).operator*(); }
		pointer operator->() const { return (*iWrappedIterator).operator->(); }
		bool operator==(const const_iterator& aOther) const { return iWrappedIterator != 0 && aOther.wrapped_iterator() != 0 && *iWrappedIterator == *aOther.wrapped_iterator(); }
		bool operator!=(const const_iterator& aOther) const { return !(*this == aOther); }
	public:
		abstract_iterator* wrapped_iterator() const { return iWrappedIterator; }
		abstract_iterator* clone() const
		{
			if (wrapped_iterator() != 0)
				return wrapped_iterator()->clone();
			else
				return 0;
		}
	protected:
		abstract_iterator* iWrappedIterator;
	};

	template <typename T, typename Category = std::random_access_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = const T*, typename Reference = const T&>
	class random_access_const_iterator : public const_iterator<T, Category, Difference, Pointer, Reference>
	{
	private:
		typedef const_iterator<T, Category, Difference, Pointer, Reference> base;
	public:
		typedef typename base::value_type value_type;
		typedef typename base::difference_type difference_type;
		typedef typename base::pointer pointer;
		typedef typename base::reference reference;
		typedef typename base::iterator_category iterator_category;
		typedef i_random_access_const_iterator<T, Category, Difference, Pointer, Reference> abstract_iterator;
	public:
		random_access_const_iterator() :
			base()
		{
		}
		random_access_const_iterator(abstract_iterator* aWrappedIterator) :
			base(aWrappedIterator)
		{
		}
		random_access_const_iterator(const random_access_const_iterator& aOther) :
			base(aOther)
		{
		}
		random_access_const_iterator(const i_random_access_iterator<T, Category, difference_type, T*, T&>& aOther) :
			base(aOther)
		{
		}
		random_access_const_iterator(const iterator<T, Category, difference_type, T*, T&>& aOther) :
			base(aOther)
		{
		}
		~random_access_const_iterator()
		{
		}
		random_access_const_iterator& operator=(const random_access_const_iterator& aOther)
		{
			base::operator=(aOther);
			return *this;
		}
		random_access_const_iterator& operator=(const i_random_access_iterator<T, Category, difference_type, T*, T&>& aOther)
		{
			base::operator=(aOther);
			return *this;
		}
		random_access_const_iterator& operator=(const iterator<T, Category, difference_type, T*, T&>& aOther)
		{
			base::operator=(aOther);
			return *this;
		}
		operator abstract_iterator&() 
		{ 
			return *wrapped_iterator(); 
		}
	public:
		abstract_iterator& operator+=(difference_type aDifference) { return (*wrapped_iterator()) += aDifference; }
		abstract_iterator& operator-=(difference_type aDifference) { return (*wrapped_iterator()) += aDifference; }
		random_access_const_iterator operator+(difference_type aDifference) const { return (*wrapped_iterator()) + aDifference; }
		random_access_const_iterator operator-(difference_type aDifference) const { return (*wrapped_iterator()) - aDifference; }
		reference operator[](difference_type aDifference) const { return (*wrapped_iterator())[aDifference]; }
		difference_type operator-(const random_access_const_iterator& aOther) const { return (*wrapped_iterator()) - (*aOther.wrapped_iterator()); }
		bool operator<(const random_access_const_iterator& aOther) const { return wrapped_iterator() != 0 && aOther.wrapped_iterator() != 0 && (*wrapped_iterator()) < *aOther.wrapped_iterator(); }
	public:
		abstract_iterator* wrapped_iterator() const { return static_cast<abstract_iterator*>(base::wrapped_iterator()); }
	};

	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	inline const_iterator<T, Category, Difference, Pointer, Reference> i_const_iterator<T, Category, Difference, Pointer, Reference>::operator++(int) { return do_post_increment(); }
	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	inline const_iterator<T, Category, Difference, Pointer, Reference> i_const_iterator<T, Category, Difference, Pointer, Reference>::operator--(int) { return do_post_decrement(); }
	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	inline random_access_const_iterator<T, Category, Difference, Pointer, Reference> i_random_access_const_iterator<T, Category, Difference, Pointer, Reference>::operator+(difference_type aDifference) const { return do_add(aDifference); }
	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	inline random_access_const_iterator<T, Category, Difference, Pointer, Reference> i_random_access_const_iterator<T, Category, Difference, Pointer, Reference>::operator-(difference_type aDifference) const { return do_subtract(aDifference); }

	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	inline iterator<T, Category, Difference, Pointer, Reference> i_iterator<T, Category, Difference, Pointer, Reference>::operator++(int) { return do_post_increment(); }
	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	inline iterator<T, Category, Difference, Pointer, Reference> i_iterator<T, Category, Difference, Pointer, Reference>::operator--(int) { return do_post_decrement(); }
	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	inline random_access_iterator<T, Category, Difference, Pointer, Reference> i_random_access_iterator<T, Category, Difference, Pointer, Reference>::operator+(difference_type aDifference) const { return do_add(aDifference); }
	template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
	inline random_access_iterator<T, Category, Difference, Pointer, Reference> i_random_access_iterator<T, Category, Difference, Pointer, Reference>::operator-(difference_type aDifference) const { return do_subtract(aDifference); }
}
