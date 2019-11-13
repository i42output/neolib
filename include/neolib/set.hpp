// set.hpp - v1.0
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
#include "mutable_set.hpp"
#include "container_iterator.hpp"
#include "i_set.hpp"

namespace neolib
{
    template <typename T, typename Pred = std::less<typename crack_key<T>::key_type>, typename Alloc = std::allocator<T>>
    class set : public reference_counted<i_set<abstract_t<T>>>
    {
        // types
    public:
        typedef i_set<abstract_t<T>> abstract_type;
        typedef abstract_t<T> abstract_key_type;
        typedef abstract_t<T> abstract_value_type;
        typedef T key_type;
        typedef T value_type;
        typedef Pred compare_type;
        typedef Alloc allocator_type;
        typedef mutable_set<value_type, compare_type, allocator_type> container_type;
    private:
        typedef typename abstract_type::base abstract_container;
    public:
        typedef typename abstract_type::size_type size_type;
        typedef container::const_iterator<value_type, typename container_type::const_iterator> container_const_iterator;
        typedef container::iterator<value_type, typename container_type::iterator, typename container_type::const_iterator> container_iterator;
    protected:
        typedef typename abstract_type::abstract_const_iterator abstract_const_iterator;
        typedef typename abstract_type::abstract_iterator abstract_iterator;
    public:
        typedef typename abstract_type::const_iterator const_iterator;
        typedef typename abstract_type::iterator iterator;
        // construction
    public:
        set()
        {
        }
        set(const abstract_container& aOther)
        {
            assign(aOther);
        }
        set(std::initializer_list<value_type> aElements) :
            iSet(aElements)
        {}
        template <typename InputIter>
        set(InputIter aFirst, InputIter aLast) :
            iSet(aFirst, aLast)
        {}
        // operations
    public:
        const container_type& container() const { return iSet; }
        container_type& container() { return iSet; }
        // implementation
    public:
        // from i_container
        size_type size() const override { return iSet.size(); }
        size_type max_size() const override { return iSet.max_size(); }
        void clear() override { iSet.clear(); }
        void assign(const abstract_container& aOther) override
        {
            if (&aOther == this) 
                return;
            clear();
            for (const_iterator i = aOther.begin(); i != aOther.end(); ++i)
                iSet.insert(value_type{ *i });
        }
    private:
        // from i_container
        abstract_const_iterator* do_begin(void* memory) const override { return new (memory) container_const_iterator(iSet.begin()); }
        abstract_const_iterator* do_end(void* memory) const override { return new (memory) container_const_iterator(iSet.end()); }
        abstract_iterator* do_begin(void* memory) override { return new (memory) container_iterator(iSet.begin()); }
        abstract_iterator* do_end(void* memory) override { return new (memory) container_iterator(iSet.end()); }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aPosition) override { return new (memory) container_iterator(iSet.erase(static_cast<const container_const_iterator&>(aPosition))); }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) override { return new (memory) container_iterator(iSet.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); }
    public:
        // from i_set
        abstract_iterator* do_insert(void* memory, const abstract_value_type& aValue) override { return new (memory) container_iterator(iSet.insert(value_type(aValue))); }
        abstract_const_iterator* do_find(void* memory, const abstract_key_type& aKey) const override { return new (memory) container_const_iterator(iSet.find(value_type{ aKey })); }
        abstract_iterator* do_find(void* memory, const abstract_key_type& aKey) override { return new (memory) container_iterator(iSet.find(value_type{ aKey })); }
    private:
        container_type iSet;
    };

    template <typename T, typename Pred = std::less<typename crack_key<T>::key_type>, typename Alloc = std::allocator<T>>
    class multiset : public reference_counted<i_multiset<abstract_t<T>>>
    {
        // types
    public:
        typedef i_multiset<abstract_t<T>> abstract_type;
        typedef abstract_t<T> abstract_key_type;
        typedef abstract_t<T> abstract_value_type;
        typedef T key_type;
        typedef T value_type;
        typedef Pred compare_type;
        typedef Alloc allocator_type;
        typedef mutable_multiset<value_type, compare_type, allocator_type> container_type;
    private:
        typedef typename abstract_type::base abstract_container;
    public:
        typedef typename abstract_type::size_type size_type;
        typedef container::const_iterator<abstract_value_type, typename container_type::const_iterator> container_const_iterator;
        typedef container::iterator<abstract_value_type, typename container_type::iterator, typename container_type::const_iterator> container_iterator;
    protected:
        typedef typename abstract_type::abstract_const_iterator abstract_const_iterator;
        typedef typename abstract_type::abstract_iterator abstract_iterator;
    public:
        typedef typename abstract_type::const_iterator const_iterator;
        typedef typename abstract_type::iterator iterator;
        // construction
    public:
        multiset()
        {}
        multiset(const abstract_container& aOther)
        {
            assign(aOther);
        }
        multiset(std::initializer_list<value_type> aElements) :
            iSet(aElements)
        {}
        template <typename InputIter>
        multiset(InputIter aFirst, InputIter aLast) :
            iSet(aFirst, aLast)
        {}
        // operations
    public:
        const container_type& container() const { return iSet; }
        container_type& container() { return iSet; }
        // implementation
    public:
        // from i_container
        size_type size() const override { return iSet.size(); }
        size_type max_size() const override { return iSet.max_size(); }
        void clear() override { iSet.clear(); }
        void assign(const abstract_container& aOther) override
        {
            if (&aOther == this)
                return;
            clear();
            for (const_iterator i = aOther.begin(); i != aOther.end(); ++i)
                iSet.insert(value_type{ *i });
        }
    private:
        // from i_container
        abstract_const_iterator* do_begin(void* memory) const override { return new (memory) container_const_iterator(iSet.begin()); }
        abstract_const_iterator* do_end(void* memory) const override { return new (memory) container_const_iterator(iSet.end()); }
        abstract_iterator* do_begin(void* memory) override { return new (memory) container_iterator(iSet.begin()); }
        abstract_iterator* do_end(void* memory) override { return new (memory) container_iterator(iSet.end()); }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aPosition) override { return new (memory) container_iterator(iSet.erase(static_cast<const container_const_iterator&>(aPosition))); }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) override { return new (memory) container_iterator(iSet.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); }
    public:
        // from i_multiset
        abstract_iterator* do_insert(void* memory, const abstract_value_type& aValue) override { return new (memory) container_iterator(iSet.insert(value_type{ aValue })); }
        abstract_const_iterator* do_find(void* memory, const abstract_key_type& aKey) const override { return new (memory) container_const_iterator(iSet.find(value_type{ aKey })); }
        abstract_iterator* do_find(void* memory, const abstract_key_type& aKey) override { return new (memory) container_iterator(iSet.find(value_type{ aKey })); }
    private:
        container_type iSet;
    };
}
