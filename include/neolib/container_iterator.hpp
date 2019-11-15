// iterator.hpp - v1.0
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
#include "reference_counted.hpp"
#include "i_iterator.hpp"

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
            typename AbstractIterator = i_iterator<abstract_t<T>, typename ContainerIterator::iterator_category, std::ptrdiff_t, abstract_t<T>*, abstract_t<T>&>,
            typename AbstractConstIterator = i_const_iterator<abstract_t<T>, typename ContainerConstIterator::iterator_category, std::ptrdiff_t, const abstract_t<T>*, const abstract_t<T>&>,
            typename ConcreteIteratorType = void>
        class iterator;
            
        template <typename T, typename ContainerIterator, typename ContainerConstIterator, typename AbstractIterator, typename AbstractConstIterator, typename ConcreteIteratorType>
        class iterator : public reference_counted<AbstractIterator, false>
        {
        public:
            typedef T value_type;
            typedef abstract_t<T> abstract_value_type;
            typedef ContainerIterator container_iterator;
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
            abstract_base_iterator& operator++() override { ++iContainerIterator; return *this; }
            abstract_base_iterator& operator--() override { --iContainerIterator; return *this; }
            reference operator*() const override { return to_abstract_type(*iContainerIterator); }
            pointer operator->() const override { return &(**this); }
            bool operator==(const abstract_base_iterator& aOther) const override { return iContainerIterator == static_cast<const iterator&>(aOther).iContainerIterator; }
            bool operator!=(const abstract_base_iterator& aOther) const override { return iContainerIterator != static_cast<const iterator&>(aOther).iContainerIterator; }
        public:
            abstract_base_iterator* clone(void* memory) const override { return new (memory) concrete_iterator_type(iContainerIterator); }
            abstract_const_base_iterator* const_clone(void* memory) const override;
        private:
            abstract_base_iterator* do_post_increment(void* memory) override { return new (memory) concrete_iterator_type(iContainerIterator++); }
            abstract_base_iterator* do_post_decrement(void* memory) override { return new (memory) concrete_iterator_type(iContainerIterator++); }
        protected:
            container_iterator iContainerIterator;
        };

        template <typename T, typename ContainerIterator, typename ContainerConstIterator,
            typename AbstractIterator = i_random_access_iterator<abstract_t<T>, typename ContainerIterator::iterator_category, std::ptrdiff_t, abstract_t<T>*, abstract_t<T>&>,
            typename AbstractConstIterator = i_random_access_const_iterator<abstract_t<T>, typename ContainerConstIterator::iterator_category, std::ptrdiff_t, const abstract_t<T>*, const abstract_t<T>&> >
        class random_access_iterator;
            
        template <typename T, typename ContainerIterator, typename ContainerConstIterator, typename AbstractIterator, typename AbstractConstIterator >
        class random_access_iterator : public iterator<T, ContainerIterator, ContainerConstIterator, AbstractIterator, AbstractConstIterator, random_access_iterator<T, ContainerIterator, ContainerConstIterator, AbstractIterator, AbstractConstIterator> >
        {
            typedef iterator<T, ContainerIterator, ContainerConstIterator, AbstractIterator, AbstractConstIterator, random_access_iterator<T, ContainerIterator, ContainerConstIterator, AbstractIterator, AbstractConstIterator>> base_type;
        public:
            using typename base_type::value_type;
            using typename base_type::abstract_value_type;
            using typename base_type::container_iterator;
            using typename base_type::container_const_iterator;
            using typename base_type::difference_type;
            using typename base_type::pointer;
            using typename base_type::reference;
            using typename base_type::abstract_const_iterator;
            using typename base_type::abstract_iterator;
        public:
            random_access_iterator() : base_type{} {}
            random_access_iterator(container_iterator aContainerIterator) : base_type{ aContainerIterator } {}
            random_access_iterator(const random_access_iterator& aOther) : base_type{ aOther } {}
            random_access_iterator& operator=(const random_access_iterator& aOther) { base_type::operator=(aOther); return *this; }
        public:
            abstract_iterator& operator+=(difference_type aDifference) override { base_type::iContainerIterator += aDifference; return *this; }
            abstract_iterator& operator-=(difference_type aDifference) override { base_type::iContainerIterator -= aDifference; return *this; }
            reference operator[](difference_type aDifference) const override { return base_type::iContainerIterator[aDifference]; }
            difference_type operator-(const abstract_iterator& aOther) const override { return base_type::iContainerIterator - static_cast<const random_access_iterator&>(aOther).iContainerIterator; }
            bool operator<(const abstract_iterator& aOther) const override { return base_type::iContainerIterator < static_cast<const random_access_iterator&>(aOther).iContainerIterator; }
            bool operator<=(const abstract_iterator& aOther) const override { return base_type::iContainerIterator <= static_cast<const random_access_iterator&>(aOther).iContainerIterator; }
            bool operator>(const abstract_iterator& aOther) const override { return base_type::iContainerIterator > static_cast<const random_access_iterator&>(aOther).iContainerIterator; }
            bool operator>=(const abstract_iterator& aOther) const override { return base_type::iContainerIterator >= static_cast<const random_access_iterator&>(aOther).iContainerIterator; }
        private:
            abstract_iterator* do_add(void* memory, difference_type aDifference) const override { return new (memory) random_access_iterator(base_type::iContainerIterator + aDifference); }
            abstract_iterator* do_subtract(void* memory, difference_type aDifference) const override { return new (memory) random_access_iterator(base_type::iContainerIterator - aDifference); }
        };

        template <typename T, typename ContainerIterator,
            typename AbstractIterator = i_const_iterator<abstract_t<T>, typename ContainerIterator::iterator_category, std::ptrdiff_t, const abstract_t<T>*, const abstract_t<T>&>,
            typename ConcreteIteratorType = void>
        class const_iterator;
            
        template <typename T, typename ContainerIterator, typename AbstractIterator, typename ConcreteIteratorType>
        class const_iterator : public reference_counted<AbstractIterator, false>
        {
        public:
            typedef const T value_type;
            typedef const abstract_t<T> abstract_value_type;
            typedef ContainerIterator container_iterator;
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
            abstract_base_iterator& operator++() override { ++iContainerIterator; return *this; }
            abstract_base_iterator& operator--() override { --iContainerIterator; return *this; }
            reference operator*() const override { return to_abstract_type(*iContainerIterator); }
            pointer operator->() const override { return &(**this); }
            bool operator==(const abstract_base_iterator& aOther) const override { return iContainerIterator == static_cast<const const_iterator&>(aOther).iContainerIterator; }
            bool operator!=(const abstract_base_iterator& aOther) const override { return iContainerIterator != static_cast<const const_iterator&>(aOther).iContainerIterator; }
        public:
            abstract_base_iterator* clone(void* memory) const override { return new (memory) concrete_iterator_type(iContainerIterator); }
        private:
            abstract_base_iterator* do_post_increment(void* memory) override { return new (memory) concrete_iterator_type(iContainerIterator++); }
            abstract_base_iterator* do_post_decrement(void* memory) override { return new (memory) concrete_iterator_type(iContainerIterator++); }
        protected:
            container_iterator iContainerIterator;
        };

        template <typename T, typename ContainerIterator,
            typename AbstractIterator = i_random_access_const_iterator<abstract_t<T>, typename ContainerIterator::iterator_category, std::ptrdiff_t, const abstract_t<T>*, const abstract_t<T>&> >
        class random_access_const_iterator;
            
        template <typename T, typename ContainerIterator, typename AbstractIterator>
        class random_access_const_iterator : public const_iterator<T, ContainerIterator, AbstractIterator, random_access_const_iterator<T, ContainerIterator, AbstractIterator> >
        {
            typedef const_iterator<T, ContainerIterator, AbstractIterator, random_access_const_iterator<T, ContainerIterator, AbstractIterator>> base_type;
        public:
            using typename base_type::value_type;
            using typename base_type::abstract_value_type;
            using typename base_type::container_iterator;
            using typename base_type::difference_type;
            using typename base_type::pointer;
            using typename base_type::reference;
            using typename base_type::abstract_iterator;
        public:
            random_access_const_iterator() {}
            random_access_const_iterator(container_iterator aContainerIterator) : base_type(aContainerIterator) {}
            random_access_const_iterator(const random_access_const_iterator& aOther) : base_type(aOther.iContainerIterator) {}
            template <typename ContainerIterator2>
            random_access_const_iterator(const random_access_iterator<T, ContainerIterator2, ContainerIterator>& aOther) : base_type(aOther.iContainerIterator) {}
            random_access_const_iterator& operator=(const random_access_const_iterator& aOther) { base_type::operator=(aOther); return *this; }
        public:
            operator container_iterator() const { return base_type::iContainerIterator; }
        public:
            abstract_iterator& operator+=(difference_type aDifference) override { base_type::iContainerIterator += aDifference; return *this; }
            abstract_iterator& operator-=(difference_type aDifference) override { base_type::iContainerIterator -= aDifference; return *this; }
            reference operator[](difference_type aDifference) const override { return base_type::iContainerIterator[aDifference]; }
            difference_type operator-(const abstract_iterator& aOther) const override { return base_type::iContainerIterator - static_cast<const random_access_const_iterator&>(aOther).iContainerIterator; }
            bool operator<(const abstract_iterator& aOther) const override { return base_type::iContainerIterator < static_cast<const random_access_const_iterator&>(aOther).iContainerIterator; }
            bool operator<=(const abstract_iterator& aOther) const override { return base_type::iContainerIterator <= static_cast<const random_access_const_iterator&>(aOther).iContainerIterator; }
            bool operator>(const abstract_iterator& aOther) const override { return base_type::iContainerIterator > static_cast<const random_access_const_iterator&>(aOther).iContainerIterator; }
            bool operator>=(const abstract_iterator& aOther) const override { return base_type::iContainerIterator >= static_cast<const random_access_const_iterator&>(aOther).iContainerIterator; }
        private:
            abstract_iterator* do_add(void* memory, difference_type aDifference) const override { return new (memory) random_access_const_iterator(base_type::iContainerIterator + aDifference); }
            abstract_iterator* do_subtract(void* memory, difference_type aDifference) const override { return new (memory) random_access_const_iterator(base_type::iContainerIterator - aDifference); }
        };

        template <typename T, typename ContainerIterator, typename ContainerConstIterator, typename AbstractIterator, typename AbstractConstIterator, typename ConcreteIteratorType>
        inline typename AbstractConstIterator::base_iterator* iterator<T, ContainerIterator, ContainerConstIterator, AbstractIterator, AbstractConstIterator, ConcreteIteratorType>::const_clone(void* memory) const
        {
            return new (memory) typename const_iterator<T, ContainerConstIterator>::concrete_iterator_type(iContainerIterator);
        }

    }
}
