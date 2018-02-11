// iterator.hpp - v1.0
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
#include "reference_counted.hpp"
#include "i_iterator.hpp"
#include "container_helper.hpp"

namespace neolib
{
	namespace container
	{
		namespace detail
		{
			template <typename ConcreteIteratorType, typename FallBack>
			struct select_concrete_iterator_type
			{
				typedef ConcreteIteratorType type;
			};

			template <typename FallBack>
			struct select_concrete_iterator_type<void, FallBack>
			{
				typedef FallBack type;
			};
		}

		template <typename T, typename ContainerIterator, typename ContainerConstIterator,
			typename AbstractIterator = i_iterator<T, typename ContainerIterator::iterator_category, std::ptrdiff_t, T*, T&>,
			typename AbstractConstIterator = i_const_iterator<T, typename ContainerConstIterator::iterator_category, std::ptrdiff_t, const T*, const T&>,
			typename ConcreteIteratorType = void>
		class iterator : public reference_counted<AbstractIterator>
		{
		public:
			typedef T value_type;
			typedef ContainerIterator container_iterator;
			typedef typename container_iterator::value_type concrete_value_type;
			typedef ContainerConstIterator container_const_iterator;
			typedef AbstractIterator abstract_iterator;
			typedef AbstractConstIterator abstract_const_iterator;
			typedef typename container_iterator::difference_type difference_type;
			typedef typename abstract_iterator::pointer pointer;
			typedef typename abstract_iterator::reference reference;
			typedef typename abstract_iterator::base_iterator abstract_base_iterator;
			typedef typename abstract_const_iterator::base_iterator abstract_const_base_iterator;
			typedef typename detail::select_concrete_iterator_type<ConcreteIteratorType, iterator>::type concrete_iterator_type;
		public:
			iterator() {}
			iterator(container_iterator aContainerIterator) : iContainerIterator(aContainerIterator) {}
			iterator(const iterator& aOther) : iContainerIterator(aOther.iContainerIterator) {}
			iterator& operator=(const iterator& aOther) { iContainerIterator = aOther.iContainerIterator; return *this; }
		public:
			operator container_iterator() const { return iContainerIterator; }
		public:
			virtual abstract_base_iterator& operator++() { ++iContainerIterator; return *this; }
			virtual abstract_base_iterator& operator--() { --iContainerIterator; return *this; }
			virtual reference operator*() const { return helper::converter<value_type, concrete_value_type>::to_abstract_type(*iContainerIterator); }
			virtual pointer operator->() const { return &operator*(); }
			virtual bool operator==(const abstract_base_iterator& aOther) const { return iContainerIterator == static_cast<const iterator&>(aOther).iContainerIterator; }
			virtual bool operator!=(const abstract_base_iterator& aOther) const { return iContainerIterator != static_cast<const iterator&>(aOther).iContainerIterator; }
		public:
			virtual abstract_base_iterator* clone() const { return new concrete_iterator_type(iContainerIterator); }
			virtual abstract_const_base_iterator* const_clone() const;
		private:
			virtual abstract_base_iterator* do_post_increment() { return new concrete_iterator_type(iContainerIterator++); }
			virtual abstract_base_iterator* do_post_decrement() { return new concrete_iterator_type(iContainerIterator++); }
		protected:
			container_iterator iContainerIterator;
		};

		template <typename T, typename ContainerIterator, typename ContainerConstIterator,
			typename AbstractIterator = i_random_access_iterator<T, typename ContainerIterator::iterator_category, std::ptrdiff_t, T*, T&>,
			typename AbstractConstIterator = i_random_access_const_iterator<T, typename ContainerConstIterator::iterator_category, std::ptrdiff_t, const T*, const T&> >
		class random_access_iterator;
			
		template <typename T, typename ContainerIterator, typename ContainerConstIterator, typename AbstractIterator, typename AbstractConstIterator >
		class random_access_iterator : public iterator<T, ContainerIterator, ContainerConstIterator, AbstractIterator, AbstractConstIterator, random_access_iterator<T, ContainerIterator, ContainerConstIterator, AbstractIterator, AbstractConstIterator> >
		{
		private:
			typedef iterator<T, ContainerIterator, ContainerConstIterator, AbstractIterator, AbstractConstIterator, random_access_iterator<T, ContainerIterator, ContainerConstIterator, AbstractIterator, AbstractConstIterator> > base;
		public:
			typedef typename base::value_type value_type;
			typedef typename base::container_iterator container_iterator;
			typedef typename base::container_const_iterator container_const_iterator;
			typedef typename base::difference_type difference_type;
			typedef typename base::pointer pointer;
			typedef typename base::reference reference;
			typedef typename base::abstract_const_iterator abstract_const_iterator;
			typedef typename base::abstract_iterator abstract_iterator;
		public:
			random_access_iterator() : base() {}
			random_access_iterator(container_iterator aContainerIterator) : base(aContainerIterator) {}
			random_access_iterator(const random_access_iterator& aOther) : base(aOther) {}
			random_access_iterator& operator=(const random_access_iterator& aOther) { base::operator=(aOther); return *this; }
		public:
			virtual abstract_iterator& operator+=(difference_type aDifference) { iContainerIterator += aDifference; return *this; }
			virtual abstract_iterator& operator-=(difference_type aDifference) { iContainerIterator -= aDifference; return *this; }
			virtual reference operator[](difference_type aDifference) const { return iContainerIterator[aDifference]; }
			virtual difference_type operator-(const abstract_iterator& aOther) const { return iContainerIterator - static_cast<const random_access_iterator&>(aOther).iContainerIterator; }
			virtual bool operator<(const abstract_iterator& aOther) const { return iContainerIterator < static_cast<const random_access_iterator&>(aOther).iContainerIterator; }
		private:
			virtual abstract_iterator* do_add(difference_type aDifference) const { return new random_access_iterator(iContainerIterator + aDifference); }
			virtual abstract_iterator* do_subtract(difference_type aDifference) const { return new random_access_iterator(iContainerIterator - aDifference); }
		};

		template <typename T, typename ContainerIterator,
			typename AbstractIterator = i_const_iterator<T, typename ContainerIterator::iterator_category, std::ptrdiff_t, const T*, const T&>,
			typename ConcreteIteratorType = void>
		class const_iterator : public reference_counted<AbstractIterator>
		{
		public:
			typedef const T value_type;
			typedef ContainerIterator container_iterator;
			typedef typename const container_iterator::value_type concrete_value_type;
			typedef AbstractIterator abstract_iterator;
			typedef typename container_iterator::difference_type difference_type;
			typedef typename abstract_iterator::pointer pointer;
			typedef typename abstract_iterator::reference reference;
			typedef typename abstract_iterator::base_iterator abstract_base_iterator;
			typedef typename detail::select_concrete_iterator_type<ConcreteIteratorType, const_iterator>::type concrete_iterator_type;
		public:
			const_iterator() {}
			const_iterator(container_iterator aContainerIterator) : iContainerIterator(aContainerIterator) {}
			const_iterator(const const_iterator& aOther) : iContainerIterator(aOther.iContainerIterator) {}
			template <typename ContainerIterator2>
			const_iterator(const iterator<T, ContainerIterator2, ContainerIterator>& aOther) : iContainerIterator(aOther.iContainerIterator) {}
			const_iterator& operator=(const const_iterator& aOther) { iContainerIterator = aOther.iContainerIterator; return *this; }
		public:
			operator container_iterator() const { return iContainerIterator; }
		public:
			virtual abstract_base_iterator& operator++() { ++iContainerIterator; return *this; }
			virtual abstract_base_iterator& operator--() { --iContainerIterator; return *this; }
			virtual reference operator*() const { return helper::converter<value_type, concrete_value_type>::to_abstract_type(*iContainerIterator); }
			virtual pointer operator->() const { return &operator*(); }
			virtual bool operator==(const abstract_base_iterator& aOther) const { return iContainerIterator == static_cast<const const_iterator&>(aOther).iContainerIterator; }
			virtual bool operator!=(const abstract_base_iterator& aOther) const { return iContainerIterator != static_cast<const const_iterator&>(aOther).iContainerIterator; }
		public:
			virtual abstract_base_iterator* clone() const { return new concrete_iterator_type(iContainerIterator); }
		private:
			virtual abstract_base_iterator* do_post_increment() { return new concrete_iterator_type(iContainerIterator++); }
			virtual abstract_base_iterator* do_post_decrement() { return new concrete_iterator_type(iContainerIterator++); }
		protected:
			container_iterator iContainerIterator;
		};

		template <typename T, typename ContainerIterator,
			typename AbstractIterator = i_random_access_const_iterator<T, typename ContainerIterator::iterator_category, std::ptrdiff_t, const T*, const T&> >
		class random_access_const_iterator;
			
		template <typename T, typename ContainerIterator, typename AbstractIterator>
		class random_access_const_iterator : public const_iterator<T, ContainerIterator, AbstractIterator, random_access_const_iterator<T, ContainerIterator, AbstractIterator> >
		{
		private:
			typedef const_iterator<T, ContainerIterator, AbstractIterator, random_access_const_iterator<T, ContainerIterator, AbstractIterator> > base;
		public:
			typedef typename base::value_type value_type;
			typedef typename base::container_iterator container_iterator;
			typedef typename base::difference_type difference_type;
			typedef typename base::pointer pointer;
			typedef typename base::reference reference;
			typedef typename base::abstract_iterator abstract_iterator;
		public:
			random_access_const_iterator() {}
			random_access_const_iterator(container_iterator aContainerIterator) : base(aContainerIterator) {}
			random_access_const_iterator(const random_access_const_iterator& aOther) : base(aOther.iContainerIterator) {}
			template <typename ContainerIterator2>
			random_access_const_iterator(const random_access_iterator<T, ContainerIterator2, ContainerIterator>& aOther) : base(aOther.iContainerIterator) {}
			random_access_const_iterator& operator=(const random_access_const_iterator& aOther) { base::operator=(aOther); return *this; }
		public:
			operator container_iterator() const { return iContainerIterator; }
		public:
			virtual abstract_iterator& operator+=(difference_type aDifference) { iContainerIterator += aDifference; return *this; }
			virtual abstract_iterator& operator-=(difference_type aDifference) { iContainerIterator -= aDifference; return *this; }
			virtual reference operator[](difference_type aDifference) const { return iContainerIterator[aDifference]; }
			virtual difference_type operator-(const abstract_iterator& aOther) const { return iContainerIterator - static_cast<const random_access_const_iterator&>(aOther).iContainerIterator; }
			virtual bool operator<(const abstract_iterator& aOther) const { return iContainerIterator < static_cast<const random_access_const_iterator&>(aOther).iContainerIterator; }
		private:
			virtual abstract_iterator* do_add(difference_type aDifference) const { return new random_access_const_iterator(iContainerIterator + aDifference); }
			virtual abstract_iterator* do_subtract(difference_type aDifference) const { return new random_access_const_iterator(iContainerIterator - aDifference); }
		};

		template <typename T, typename ContainerIterator, typename ContainerConstIterator, typename AbstractIterator, typename AbstractConstIterator, typename ConcreteIteratorType>
		inline typename AbstractConstIterator::base_iterator* iterator<T, ContainerIterator, ContainerConstIterator, AbstractIterator, AbstractConstIterator, ConcreteIteratorType>::const_clone() const
		{
			return new typename const_iterator<T, ContainerConstIterator>::concrete_iterator_type(iContainerIterator);
		}

	}
}
