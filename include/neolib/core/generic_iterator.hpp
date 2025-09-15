// generic_iterator.hpp
/*
 *  Copyright (c) 2007, 2025 Leigh Johnston.
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
#include <cstddef>
#include <type_traits>
#include <iterator>

namespace neolib
{
    class generic_iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
    public:
        struct wrong_iterator_type : std::logic_error { wrong_iterator_type() : std::logic_error("neolib::generic_iterator::wrong_iterator_type") {} };
    protected:
        class i_wrapper
        {
        public:
            virtual ~i_wrapper() = default;
        public:
            virtual i_wrapper* clone() const = 0;
            virtual i_wrapper* clone(void* aStorage) const = 0;
        public:
            virtual i_wrapper& operator++() = 0;
            virtual i_wrapper& operator--() = 0;
            virtual i_wrapper& operator+=(difference_type aDelta) = 0;
            virtual i_wrapper& operator-=(difference_type aDelta) = 0;
            virtual bool operator==(const i_wrapper& aOther) const = 0;
            virtual bool operator!=(const i_wrapper& aOther) const = 0;
        };
        template <typename Iterator>
        class wrapper : public i_wrapper
        {
        public:
            using value_type = typename std::iterator_traits<Iterator>::value_type;
            using pointer = typename std::iterator_traits<Iterator>::pointer;
            using reference = typename std::iterator_traits<Iterator>::reference;
        public:
            wrapper(Iterator aIterator) : iIterator(aIterator)
            {
            }
        public:
            i_wrapper* clone() const override
            {
                return new wrapper(*this);
            }
            i_wrapper* clone(void* aStorage) const override
            {
                return new (aStorage)wrapper(*this);
            }
        public:
            Iterator get() const
            {
                return iIterator;
            }
        public:
            reference operator*() const
            {
                return *iIterator;
            }
            pointer operator->() const
            {
                return &*iIterator;
            }
        public:
            wrapper& operator++() override
            {
                ++iIterator;
                return *this;
            }
            wrapper& operator--() override
            {
                --iIterator;
                return *this;
            }
            wrapper& operator+=(difference_type aDelta) override
            {
                std::advance(iIterator, aDelta);
                return *this;
            }
            wrapper& operator-=(difference_type aDelta) override
            {
                std::advance(iIterator, -aDelta);
                return *this;
            }
            bool operator==(const i_wrapper& aOther) const override
            {
                return iIterator == dynamic_cast<const wrapper&>(aOther).iIterator;
            }
            bool operator!=(const i_wrapper& aOther) const override
            {
                return !(*this == aOther);
            }
        private:
            Iterator iIterator;
        };
    public:
        generic_iterator() :
            iIteratorStorage{},
            iWrappedIterator{ nullptr }, 
            iInPlace{ false }
        {
        }
        generic_iterator(const generic_iterator& aOther) :
            iIteratorStorage{},
            iWrappedIterator{ nullptr },
            iInPlace{ aOther.iInPlace }
        {
            if (aOther.iWrappedIterator != nullptr)
                iWrappedIterator = aOther.iInPlace ? aOther.iWrappedIterator->clone(iIteratorStorage) : aOther.iWrappedIterator->clone();
        }
        virtual ~generic_iterator()
        {
            if (iInPlace)
                iWrappedIterator->~i_wrapper();
            else
                delete iWrappedIterator;
        }
    public:
        generic_iterator& operator++()
        {
            (*iWrappedIterator).operator++();
            return *this;
        }
        generic_iterator& operator--()
        {
            (*iWrappedIterator).operator--();
            return *this;
        }
        generic_iterator operator++(int)
        {
            generic_iterator old = *this;
            (*iWrappedIterator).operator++();
            return old;
        }
        generic_iterator operator--(int)
        {
            generic_iterator old = *this;
            (*iWrappedIterator).operator--();
            return old;
        }
        generic_iterator& operator+=(difference_type aDelta)
        {
            (*iWrappedIterator).operator+=(aDelta);
            return *this;
        }
        generic_iterator& operator-=(difference_type aDelta)
        {
            (*iWrappedIterator).operator-=(aDelta);
            return *this;
        }
        generic_iterator operator+(difference_type aDelta)
        {
            generic_iterator result = *this;
            (*result.iWrappedIterator).operator+=(aDelta);
            return result;
        }
        generic_iterator operator-(difference_type aDelta)
        {
            generic_iterator result = *this;
            (*result.iWrappedIterator).operator-=(aDelta);
            return result;
        }
        bool operator==(const generic_iterator& aOther) const
        {
            return (*iWrappedIterator).operator==(*aOther.iWrappedIterator);
        }
        bool operator!=(const generic_iterator& aOther) const
        {
            return (*iWrappedIterator).operator!=(*aOther.iWrappedIterator);
        }
    public:
        template <typename Iterator>
        bool is_one_of() const
        {
            return dynamic_cast<const wrapper<Iterator>*>(&wrapped_iterator()) != nullptr;
        }
        template <typename Iterator1, typename Iterator2, typename... Rest>
        bool is_one_of() const
        {
            return dynamic_cast<const wrapper<Iterator1>*>(&wrapped_iterator()) != nullptr || is_one_of<Iterator2, Rest...>();
        }
        template <typename Iterator>
        Iterator get() const
        {
            if (is_one_of<Iterator>())
            {
                return dynamic_cast<const wrapper<Iterator>&>(wrapped_iterator()).get();
            }
            else
            {
                throw wrong_iterator_type();
            }
        }
        template <typename Iterator, typename NextIterator, typename... Rest>
        Iterator get() const
        {
            if (is_one_of<NextIterator>())
            {
                return dynamic_cast<const wrapper<NextIterator>&>(wrapped_iterator()).get();
            }
            else
            {
                return get<Iterator, Rest...>();
            }
        }
    protected:
        template <typename Wrapper>
        void construct(const Wrapper& aWrapper)
        {
            construct2(aWrapper, sizeof(Wrapper) <= sizeof(iIteratorStorage));
        }
        template <typename Wrapper>
        void construct2(const Wrapper& aWrapper, bool aInPlace)
        {
            if (aInPlace)
            {
                iWrappedIterator = new(iIteratorStorage) Wrapper{ aWrapper };
                iInPlace = true;
            }
            else
            {
                iWrappedIterator = new Wrapper{ aWrapper };
                iInPlace = false;
            }
        }
        i_wrapper& wrapped_iterator()
        {
            return *iWrappedIterator;
        }
        const i_wrapper& wrapped_iterator() const
        {
            return *iWrappedIterator;
        }
    private:
        alignas(std::max_align_t) std::byte iIteratorStorage[sizeof(void*) * 4];
        i_wrapper* iWrappedIterator;
        bool iInPlace;
    };

    template <typename Iterator>
    class specialized_generic_iterator : public generic_iterator
    {
    public:
        typedef typename std::iterator_traits<Iterator>::value_type value_type;
        typedef typename std::iterator_traits<Iterator>::pointer pointer;
        typedef typename std::iterator_traits<Iterator>::reference reference;
        typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
    public:
        specialized_generic_iterator(Iterator aIterator)
        {
            construct(wrapper<Iterator>(aIterator));
        }
        specialized_generic_iterator(const specialized_generic_iterator& aOther) :
            generic_iterator(aOther)
        {
        }
        specialized_generic_iterator(const generic_iterator& aOther) :
            generic_iterator(aOther)
        {
        }
    public:
        specialized_generic_iterator& operator++()
        {
            wrapped_iterator().operator++();
            return *this;
        }
        specialized_generic_iterator& operator--()
        {
            wrapped_iterator().operator--();
            return *this;
        }
        specialized_generic_iterator operator++(int)
        {
            specialized_generic_iterator old = *this;
            wrapped_iterator().operator++();
            return old;
        }
        specialized_generic_iterator operator--(int)
        {
            specialized_generic_iterator old = *this;
            wrapped_iterator().operator--();
            return old;
        }
        specialized_generic_iterator& operator+=(difference_type aDelta)
        {
            wrapped_iterator().operator+=(aDelta);
            return *this;
        }
        specialized_generic_iterator& operator-=(difference_type aDelta)
        {
            wrapped_iterator().operator-=(aDelta);
            return *this;
        }
        specialized_generic_iterator operator+(difference_type aDelta)
        {
            return generic_iterator::operator+(aDelta);
        }
        specialized_generic_iterator operator-(difference_type aDelta)
        {
            return generic_iterator::operator-(aDelta);
        }
        reference operator*() const
        {
            return dynamic_cast<const wrapper<Iterator>&>(wrapped_iterator()).operator*();
        }
        pointer operator->() const
        {
            return dynamic_cast<const wrapper<Iterator>&>(wrapped_iterator()).operator->();
        }
        bool operator==(const specialized_generic_iterator& aOther) const
        {
            return wrapped_iterator().operator==(aOther.wrapped_iterator());
        }
        bool operator!=(const specialized_generic_iterator& aOther) const
        {
            return wrapped_iterator().operator!=(aOther.wrapped_iterator());
        }
    };

    template <typename Iterator>
    inline specialized_generic_iterator<Iterator> make_generic_iterator(Iterator aIterator)
    {
        return specialized_generic_iterator<Iterator>{ aIterator };
    }
}