// list.hpp - v1.0
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
#include <list>
#include "reference_counted.hpp"
#include "i_list.hpp"
#include "container_iterator.hpp"
#include "container_helper.hpp"

namespace neolib
{
    template <typename T, typename ConcreteType = T>
    class list : public reference_counted<i_list<T> >
    {
        // types
    private:
        typedef reference_counted<i_list<T> > base;
    public:
        typedef T value_type;
        typedef ConcreteType concrete_value_type;
        typedef std::list<concrete_value_type> container_type;
        typedef typename i_list<T>::size_type size_type;
        typedef typename i_list<T>::const_iterator const_iterator;
        typedef typename i_list<T>::iterator iterator;
        typedef typename i_list<T>::generic_container_type generic_container_type;
    protected:
        typedef typename i_list<T>::abstract_const_iterator abstract_const_iterator;
        typedef typename i_list<T>::abstract_iterator abstract_iterator;
    protected:
        typedef container::const_iterator<T, typename container_type::const_iterator> container_const_iterator;
        typedef container::iterator<T, typename container_type::iterator, typename container_type::const_iterator> container_iterator;
        // construction
    public:
        list() {}
        list(const list& aOther) : iList{ aOther.begin(), aOther.end() } {}
        list(list&& aOther) : iList{ std::move(aOther.iList) } {}
        list(const i_list<T>& aOther) : iList{ aOther.begin(), aOther.end() } {}
        list& operator=(const list& aOther) { assign(aOther); return *this; }
        list& operator=(list&& aOther) { iList = std::move(aOther.iList); return *this; }
        list& operator=(const i_list<T>& aOther) { assign(aOther); return *this; }
        // operations
    public:
        container_type& container() { return iList; }
        const container_type& container() const { return iList; }
        // implementation
    public:
        // from i_container
        size_type size() const override { return iList.size(); }
        size_type max_size() const override { return iList.max_size(); }
        void clear() override { iList.clear(); }
        void assign(const generic_container_type& aOther) override { if (&aOther == this) return; iList.assign(aOther.begin(), aOther.end()); }
    private:
        // from i_container
        abstract_const_iterator* do_begin(void* memory) const override { return new(memory) container_const_iterator(iList.begin()); }
        abstract_const_iterator* do_end(void* memory) const override { return new(memory) container_const_iterator(iList.end()); }
        abstract_iterator* do_begin(void* memory) override { return new(memory) container_iterator(iList.begin()); }
        abstract_iterator* do_end(void* memory) override { return new(memory) container_iterator(iList.end()); }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aPosition) override { return new (memory) container_iterator(iList.erase(static_cast<const container_const_iterator&>(aPosition))); }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) override { return new (memory) container_iterator(iList.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); }
    public:
        // from i_sequence_container
        size_type capacity() const override { return iList.max_size(); }
        void reserve(size_type aCapacity) override { /* do nothing */ }
        void resize(size_type aSize, const value_type& aValue) override { iList.resize(aSize, container::helper::converter<const value_type, const concrete_value_type>::to_concrete_type(aValue)); }
        void push_back(const value_type& aValue) override { iList.push_back(container::helper::converter<const value_type, const concrete_value_type>::to_concrete_type(aValue)); }
        void pop_back() override { iList.pop_back(); }
        const value_type& back() const override { return container::helper::converter<const value_type, const concrete_value_type>::to_abstract_type(iList.back()); }
        value_type& back() override { return container::helper::converter<value_type, concrete_value_type>::to_abstract_type(iList.back()); }
    private:
        // from i_sequence_container
        abstract_iterator* do_insert(void* memory, const abstract_const_iterator& aPosition, const value_type& aValue) override { return new (memory) container_iterator(iList.insert(static_cast<const container_const_iterator&>(aPosition), container::helper::converter<const value_type, const concrete_value_type>::to_concrete_type(aValue))); }
    public:
        // from i_list
        void push_front(const value_type& aValue) override { iList.push_front(container::helper::converter<const value_type, const concrete_value_type>::to_concrete_type(aValue)); }
        void pop_front() override { iList.pop_front(); }
        const value_type& front() const override { return container::helper::converter<const value_type, const concrete_value_type>::to_abstract_type(iList.front()); }
        value_type& front() override { return container::helper::converter<value_type, concrete_value_type>::to_abstract_type(iList.front()); }
        // attributes
    private:
        std::list<ConcreteType> iList;
    };
}
