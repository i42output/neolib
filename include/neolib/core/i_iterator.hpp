// i_iterator.hpp
/*
 *  Copyright (c) 2007,2025 Leigh Johnston.
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
#include <iterator>
#include <neolib/core/reference_counted.hpp>

namespace neolib
{
    struct singular_iterator : std::logic_error { singular_iterator() : std::logic_error("neolib::singular_iterator") {} };

    template <typename, typename, typename, typename, typename>
    class const_iterator;

    template <typename T, typename Category = std::bidirectional_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = const T*, typename Reference = const T&>
    class i_const_iterator : public i_reference_counted
    {
    public:
        typedef i_const_iterator abstract_type;
        typedef T value_type;
        typedef Difference difference_type;
        typedef Pointer pointer;
        typedef Reference reference;
        typedef Category iterator_category;
    public:
        typedef i_const_iterator abstract_iterator;
        typedef i_const_iterator abstract_const_iterator;
        typedef abstract_iterator abstract_base_iterator;
        typedef abstract_const_iterator abstract_base_const_iterator;
        typedef const_iterator<T, Category, Difference, Pointer, Reference> iterator_wrapper;
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
    };

    template <typename, typename, typename, typename, typename>
    class random_access_const_iterator;

    template <typename T, typename Category = std::random_access_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = const T*, typename Reference = const T&>
    class i_random_access_const_iterator : public i_const_iterator<T, Category, Difference, Pointer, Reference>
    {
    public:
        typedef i_random_access_const_iterator abstract_type;
        typedef i_const_iterator<T, Category, Difference, Pointer, Reference> base_type;
    public:
        using typename base_type::value_type;
        using typename base_type::difference_type;
        using typename base_type::pointer;
        using typename base_type::reference;
        using typename base_type::iterator_category;
    public:
        typedef i_random_access_const_iterator abstract_iterator;
        typedef i_random_access_const_iterator abstract_const_iterator;
        typedef abstract_iterator abstract_random_access_iterator;
        typedef abstract_const_iterator abstract_random_access_const_iterator;
        typedef random_access_const_iterator<T, Category, Difference, Pointer, Reference> iterator_wrapper;
    public:
        virtual i_random_access_const_iterator& operator+=(difference_type aDifference) = 0;
        virtual i_random_access_const_iterator& operator-=(difference_type aDifference) = 0;
        iterator_wrapper operator+(difference_type aDifference) const;
        iterator_wrapper operator-(difference_type aDifference) const;
        virtual reference operator[](difference_type aDifference) const = 0;
        virtual difference_type operator-(const i_random_access_const_iterator& aOther) const = 0;
        virtual bool operator<(const i_random_access_const_iterator& aOther) const = 0;
        virtual bool operator<=(const i_random_access_const_iterator& aOther) const = 0;
        virtual bool operator>(const i_random_access_const_iterator& aOther) const = 0;
        virtual bool operator>=(const i_random_access_const_iterator& aOther) const = 0;
    };

    template <typename, typename, typename, typename, typename>
    class i_iterator;
    template <typename, typename, typename, typename, typename>
    class iterator;

    template <typename T, typename Category = std::bidirectional_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = const T*, typename Reference = const T&>
    class const_iterator
    {
    public:
        typedef const_iterator abstract_type;
        typedef T value_type;
        typedef Difference difference_type;
        typedef Pointer pointer;
        typedef Reference reference;
        typedef Category iterator_category;
    public:
        typedef i_const_iterator<T, Category, Difference, Pointer, Reference> abstract_iterator;
        typedef i_const_iterator<T, Category, Difference, Pointer, Reference> abstract_const_iterator;
    public:
        const_iterator()
        {
        }
        const_iterator(abstract_iterator* aWrappedIterator)
        {
            iWrappedIterator = aWrappedIterator;
        }
        const_iterator(const const_iterator& aOther)
        {
            iWrappedIterator = aOther.clone();
        }
        const_iterator(const i_iterator<T, Category, Difference, T*, T&>& aOther)
        {
            iWrappedIterator = aOther.const_clone();
        }
        const_iterator(const iterator<T, Category, Difference, T*, T&>& aOther)
        {
            iWrappedIterator = aOther.const_clone();
        }
        ~const_iterator()
        {
        }
        const_iterator& operator=(const const_iterator& aOther)
        {
            iWrappedIterator = aOther.clone();
            return *this;
        }
        const_iterator& operator=(const iterator<T, Category, Difference, T*, T&>& aOther)
        {
            iWrappedIterator = aOther.const_clone();
            return *this;
        }
        operator abstract_iterator& ()
        {
            return wrapped_iterator();
        }
    public:
        abstract_iterator& operator++() { return ++wrapped_iterator(); }
        abstract_iterator& operator--() { return --wrapped_iterator(); }
        const_iterator operator++(int) { return wrapped_iterator()++; }
        const_iterator operator--(int) { return wrapped_iterator()--; }
        reference operator*() const { return wrapped_iterator().operator*(); }
        pointer operator->() const { return wrapped_iterator().operator->(); }
        bool operator==(const const_iterator& aOther) const { return wrapped_iterator() == aOther.wrapped_iterator(); }
        bool operator!=(const const_iterator& aOther) const { return !(*this == aOther); }
    public:
        bool is_singular() const
        {
            return iWrappedIterator == nullptr;
        }
        abstract_iterator& wrapped_iterator() const
        {
            if (is_singular())
                throw singular_iterator();
            return *iWrappedIterator;
        }
        neolib::ref_ptr<abstract_iterator> clone() const
        {
            if (is_singular())
                return nullptr;
            return wrapped_iterator().clone();
        }
    protected:
        neolib::ref_ptr<abstract_iterator> iWrappedIterator;
    };

    template <typename, typename, typename, typename, typename>
    class i_random_access_iterator;

    template <typename T, typename Category = std::random_access_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = const T*, typename Reference = const T&>
    class random_access_const_iterator : public const_iterator<T, Category, Difference, Pointer, Reference>
    {
        typedef const_iterator<T, Category, Difference, Pointer, Reference> base_type;
    public:
        typedef random_access_const_iterator abstract_type;
        using typename base_type::value_type;
        using typename base_type::difference_type;
        using typename base_type::pointer;
        using typename base_type::reference;
        using typename base_type::iterator_category;
    public:
        using abstract_iterator = i_random_access_const_iterator<T, Category, Difference, Pointer, Reference>;
        using abstract_const_iterator = abstract_type;
    public:
        random_access_const_iterator() :
            base_type()
        {
        }
        random_access_const_iterator(abstract_iterator* aWrappedIterator) :
            base_type(aWrappedIterator)
        {
        }
        random_access_const_iterator(const random_access_const_iterator& aOther) :
            base_type(aOther)
        {
        }
        random_access_const_iterator(const i_random_access_iterator<T, Category, Difference, T*, T&>& aOther) :
            base_type(aOther)
        {
        }
        random_access_const_iterator(const iterator<T, Category, Difference, T*, T&>& aOther) :
            base_type(aOther)
        {
        }
        ~random_access_const_iterator()
        {
        }
        random_access_const_iterator& operator=(const random_access_const_iterator& aOther)
        {
            base_type::operator=(aOther);
            return *this;
        }
        random_access_const_iterator& operator=(const i_random_access_iterator<T, Category, Difference, T*, T&>& aOther)
        {
            base_type::operator=(aOther);
            return *this;
        }
        random_access_const_iterator& operator=(const iterator<T, Category, Difference, T*, T&>& aOther)
        {
            base_type::operator=(aOther);
            return *this;
        }
        operator abstract_iterator& ()
        {
            return wrapped_iterator();
        }
    public:
        abstract_iterator& operator+=(difference_type aDifference) { return wrapped_iterator() += aDifference; }
        abstract_iterator& operator-=(difference_type aDifference) { return wrapped_iterator() -= aDifference; }
        random_access_const_iterator operator+(difference_type aDifference) const { return wrapped_iterator() + aDifference; }
        random_access_const_iterator operator-(difference_type aDifference) const { return wrapped_iterator() - aDifference; }
        reference operator[](difference_type aDifference) const { return wrapped_iterator()[aDifference]; }
        difference_type operator-(const random_access_const_iterator& aOther) const { return wrapped_iterator() - (aOther.wrapped_iterator()); }
        bool operator<(const random_access_const_iterator& aOther) const { return wrapped_iterator() < aOther.wrapped_iterator(); }
        bool operator<=(const random_access_const_iterator& aOther) const { return wrapped_iterator() <= aOther.wrapped_iterator(); }
        bool operator>(const random_access_const_iterator& aOther) const { return wrapped_iterator() > aOther.wrapped_iterator(); }
        bool operator>=(const random_access_const_iterator& aOther) const { return wrapped_iterator() >= aOther.wrapped_iterator(); }
    public:
        abstract_iterator& wrapped_iterator() const { return static_cast<abstract_iterator&>(base_type::wrapped_iterator()); }
    };

    template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
    inline const_iterator<T, Category, Difference, Pointer, Reference> i_const_iterator<T, Category, Difference, Pointer, Reference>::operator++(int) 
    { 
        const_iterator result = *this;
        ++result;
        return result;
    }
    template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
    inline const_iterator<T, Category, Difference, Pointer, Reference> i_const_iterator<T, Category, Difference, Pointer, Reference>::operator--(int) 
    { 
        const_iterator result = *this;
        --result;
        return result;
    }
    template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
    inline random_access_const_iterator<T, Category, Difference, Pointer, Reference> i_random_access_const_iterator<T, Category, Difference, Pointer, Reference>::operator+(difference_type aDifference) const
    {
        random_access_const_iterator<T, Category, Difference, Pointer, Reference> result = *this;
        result += aDifference;
        return result;
    }
    template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
    inline random_access_const_iterator<T, Category, Difference, Pointer, Reference> i_random_access_const_iterator<T, Category, Difference, Pointer, Reference>::operator-(difference_type aDifference) const
    {
        random_access_const_iterator<T, Category, Difference, Pointer, Reference> result = *this;
        result -= aDifference;
        return result;
    }

    template <typename, typename, typename, typename, typename>
    class iterator;

    template <typename T, typename Category = std::bidirectional_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = T*, typename Reference = T&>
    class i_iterator : public i_reference_counted
    {
    public:
        typedef i_iterator abstract_type;
    public:
        typedef T value_type;
        typedef Difference difference_type;
        typedef Pointer pointer;
        typedef Reference reference;
        typedef Category iterator_category;
    public:
        typedef i_iterator abstract_iterator;
        typedef i_const_iterator<T, Category, Difference, const T*, const T&> abstract_const_iterator;
        typedef abstract_iterator abstract_base_iterator;
        typedef abstract_const_iterator abstract_base_const_iterator;
        typedef iterator<T, Category, Difference, Pointer, Reference> iterator_wrapper;
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
        virtual abstract_const_iterator* const_clone() const = 0;
    };

    template <typename, typename, typename, typename, typename>
    class random_access_iterator;

    template <typename T, typename Category = std::random_access_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = T*, typename Reference = T&>
    class i_random_access_iterator : public i_iterator<T, Category, Difference, Pointer, Reference>
    {
        typedef i_iterator<T, Category, Difference, Pointer, Reference> base_type;
    public:
        typedef i_random_access_iterator abstract_type;
    public:
        using typename base_type::value_type;
        using typename base_type::difference_type;
        using typename base_type::pointer;
        using typename base_type::reference;
        using typename base_type::iterator_category;
    public:
        typedef i_random_access_iterator abstract_iterator;
        typedef i_random_access_const_iterator<T, Category, Difference, const T*, const T&> abstract_const_iterator;
        typedef abstract_iterator abstract_random_access_iterator;
        typedef abstract_const_iterator abstract_random_access_const_iterator;
        typedef random_access_iterator<T, Category, Difference, Pointer, Reference> iterator_wrapper;
    public:
        virtual i_random_access_iterator& operator+=(difference_type aDifference) = 0;
        virtual i_random_access_iterator& operator-=(difference_type aDifference) = 0;
        iterator_wrapper operator+(difference_type aDifference) const;
        iterator_wrapper operator-(difference_type aDifference) const;
        virtual reference operator[](difference_type aDifference) const = 0;
        virtual difference_type operator-(const i_random_access_iterator& aOther) const = 0;
        virtual bool operator<(const i_random_access_iterator& aOther) const = 0;
        virtual bool operator<=(const i_random_access_iterator& aOther) const = 0;
        virtual bool operator>(const i_random_access_iterator& aOther) const = 0;
        virtual bool operator>=(const i_random_access_iterator& aOther) const = 0;
    };

    template <typename T, typename Category = std::bidirectional_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = T*, typename Reference = T&>
    class iterator
    {
    public:
        typedef iterator abstract_type;
        typedef T value_type;
        typedef Difference difference_type;
        typedef Pointer pointer;
        typedef Reference reference;
        typedef Category iterator_category;
    public:
        typedef i_iterator<T, Category, Difference, Pointer, Reference> abstract_iterator;
        typedef i_const_iterator<T, Category, Difference, const T*, const T&> abstract_const_iterator;
    public:
        iterator()
        {
        }
        iterator(abstract_iterator* aWrappedIterator)
        {
            iWrappedIterator = aWrappedIterator;
        }
        iterator(const iterator& aOther)
        {
            iWrappedIterator = aOther.clone();
        }
        ~iterator()
        {
        }
        iterator& operator=(const iterator& aOther)
        {
            iWrappedIterator = aOther.clone();
            return *this;
        }
        operator abstract_iterator&()
        {
            return wrapped_iterator();
        }
    public:
        abstract_iterator& operator++() { return ++wrapped_iterator(); }
        abstract_iterator& operator--() { return --wrapped_iterator(); }
        iterator operator++(int) { return wrapped_iterator()++; }
        iterator operator--(int) { return wrapped_iterator()--; }
        reference operator*() const { return wrapped_iterator().operator*(); }
        pointer operator->() const { return wrapped_iterator().operator->(); }
        bool operator==(const iterator& aOther) const { return wrapped_iterator() == aOther.wrapped_iterator(); }
        bool operator!=(const iterator& aOther) const { return !(*this == aOther); }
    public:
        bool is_singular() const 
        { 
            return iWrappedIterator == nullptr; 
        }
        abstract_iterator& wrapped_iterator() const 
        { 
            if (is_singular())
                throw singular_iterator();
            return *iWrappedIterator;
        }
        neolib::ref_ptr<abstract_iterator> clone() const
        {
            if (is_singular())
                return nullptr;
            return wrapped_iterator().clone();
        }
        neolib::ref_ptr<abstract_const_iterator> const_clone() const
        {
            if (is_singular())
                return nullptr;
            return wrapped_iterator().const_clone();
        }
    protected:
        neolib::ref_ptr<abstract_iterator> iWrappedIterator;
    };

    template <typename T, typename Category = std::random_access_iterator_tag, typename Difference = std::ptrdiff_t, typename Pointer = T*, typename Reference = T&>
    class random_access_iterator : public iterator<T, Category, Difference, Pointer, Reference>
    {
        typedef iterator<T, Category, Difference, Pointer, Reference> base_type;
    public:
        typedef random_access_iterator abstract_type;
        using typename base_type::value_type;
        using typename base_type::difference_type;
        using typename base_type::pointer;
        using typename base_type::reference;
        using typename base_type::iterator_category;
    public:
        using abstract_iterator = i_random_access_iterator<T, Category, Difference, Pointer, Reference>;
        using abstract_const_iterator = i_random_access_const_iterator<T, Category, Difference, const T*, const T&>;
    public:
        random_access_iterator() :
            base_type()
        {
        }
        random_access_iterator(abstract_iterator* aWrappedIterator) :
            base_type(aWrappedIterator)
        {
        }
        random_access_iterator(const random_access_iterator& aOther) :
            base_type(aOther)
        {
        }
        ~random_access_iterator()
        {
        }
        random_access_iterator& operator=(const random_access_iterator& aOther)
        {
            base_type::operator=(aOther);
            return *this;
        }
        operator abstract_iterator&()
        {
            return wrapped_iterator();
        }
    public:
        abstract_iterator& operator+=(difference_type aDifference) { return wrapped_iterator() += aDifference; }
        abstract_iterator& operator-=(difference_type aDifference) { return wrapped_iterator() -= aDifference; }
        random_access_iterator operator+(difference_type aDifference) const { return wrapped_iterator() + aDifference; }
        random_access_iterator operator-(difference_type aDifference) const { return wrapped_iterator() - aDifference; }
        reference operator[](difference_type aDifference) const { return wrapped_iterator()[aDifference]; }
        difference_type operator-(const random_access_iterator& aOther) const { return wrapped_iterator() - (aOther.wrapped_iterator()); }
        bool operator<(const random_access_iterator& aOther) const { return wrapped_iterator() < aOther.wrapped_iterator(); }
        bool operator<=(const random_access_iterator& aOther) const { return wrapped_iterator() <= aOther.wrapped_iterator(); }
        bool operator>(const random_access_iterator& aOther) const { return wrapped_iterator() > aOther.wrapped_iterator(); }
        bool operator>=(const random_access_iterator& aOther) const { return wrapped_iterator() >= aOther.wrapped_iterator(); }
    public:
        abstract_iterator& wrapped_iterator() const { return static_cast<abstract_iterator&>(base_type::wrapped_iterator()); }
    };

    template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
    inline iterator<T, Category, Difference, Pointer, Reference> i_iterator<T, Category, Difference, Pointer, Reference>::operator++(int) 
    { 
        iterator result = *this;
        ++result;
        return result;
    }
    template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
    inline iterator<T, Category, Difference, Pointer, Reference> i_iterator<T, Category, Difference, Pointer, Reference>::operator--(int) 
    { 
        iterator result = *this;
        --result;
        return result;
    }
    template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
    inline random_access_iterator<T, Category, Difference, Pointer, Reference> i_random_access_iterator<T, Category, Difference, Pointer, Reference>::operator+(difference_type aDifference) const 
    { 
        random_access_iterator<T, Category, Difference, Pointer, Reference> result = *this;
        result += aDifference;
        return result;
    }
    template <typename T, typename Category, typename Difference, typename Pointer, typename Reference>
    inline random_access_iterator<T, Category, Difference, Pointer, Reference> i_random_access_iterator<T, Category, Difference, Pointer, Reference>::operator-(difference_type aDifference) const 
    { 
        random_access_iterator<T, Category, Difference, Pointer, Reference> result = *this;
        result -= aDifference;
        return result;
    }
}
