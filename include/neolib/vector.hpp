// vector.hpp - v1.0
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
#include <vector>
#include "reference_counted.hpp"
#include "i_vector.hpp"
#include "container_iterator.hpp"

namespace neolib
{
    template <typename T, typename ConcreteType = T>
    class vector : public reference_counted<i_vector<T> >
    {
        // types
    private:
        typedef reference_counted<i_vector<T> > base;
    public:
        typedef T value_type;
        typedef ConcreteType concrete_value_type;
        typedef std::vector<concrete_value_type> container_type;
        typedef typename i_vector<T>::size_type size_type;
        typedef typename i_vector<T>::const_iterator const_iterator;
        typedef typename i_vector<T>::iterator iterator;
        typedef typename i_vector<T>::generic_container_type generic_container_type;
    protected:
        typedef typename i_vector<T>::abstract_const_iterator abstract_const_iterator;
        typedef typename i_vector<T>::abstract_iterator abstract_iterator;
    protected:
        typedef container::random_access_const_iterator<T, typename container_type::const_iterator> container_const_iterator;
        typedef container::random_access_iterator<T, typename container_type::iterator, typename container_type::const_iterator> container_iterator;
        // construction
    public:
        vector() {}
        vector(const vector& aOther) : iVector{ aOther.begin(), aOther.end() } {}
        vector(vector&& aOther) : iVector{ std::move(aOther.iVector) } {}
        vector(const i_vector<T>& aOther) : iVector{ aOther.begin(), aOther.end() } {}
        vector(const container_type& aOtherContainer) : iVector{ aOtherContainer } {}
        template <typename InputIter>
        vector(InputIter aFirst, InputIter aLast) : iVector{ aFirst, aLast } {}
        vector& operator=(const vector& aOther) { assign(aOther); return *this; }
        vector& operator=(vector&& aOther) { iVector = std::move(aOther.iVector); return *this; }
        vector& operator=(const i_vector<T>& aOther) { assign(aOther); return *this; }
        // operations
    public:
        container_type& container() { return iVector; }
        const container_type& container() const { return iVector; }
        // implementation
    public:
        // from i_container
        size_type size() const override { return iVector.size(); }
        size_type max_size() const override { return iVector.max_size(); }
        void clear() override { iVector.clear(); }
        void assign(const generic_container_type& aOther) override { if (&aOther == this) return; iVector.assign(aOther.begin(), aOther.end()); }
    private:
        // from i_container
        abstract_const_iterator* do_begin(void* memory) const override { return new (memory) container_const_iterator(iVector.begin()); }
        abstract_const_iterator* do_end(void* memory) const override { return new (memory) container_const_iterator(iVector.end()); }
        abstract_iterator* do_begin(void* memory) override { return new (memory) container_iterator(iVector.begin()); }
        abstract_iterator* do_end(void* memory) override { return new (memory) container_iterator(iVector.end()); }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aPosition) override { return new (memory) container_iterator(iVector.erase(static_cast<const container_const_iterator&>(aPosition))); }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) override { return new (memory) container_iterator(iVector.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); }
    public:
        // from i_sequence_container
        size_type capacity() const override { return iVector.size(); }
        void reserve(size_type aCapacity) override { iVector.reserve(aCapacity); }
        void resize(size_type aSize, const value_type& aValue) override { iVector.resize(aSize, aValue); }
        void push_back(const value_type& aValue) override { iVector.push_back(aValue); }
        void pop_back() override { iVector.pop_back(); }
        const value_type& back() const override { return iVector.back(); }
        value_type& back() override { return iVector.back(); }
    private:
        // from i_sequence_container
        abstract_iterator* do_insert(void* memory, const abstract_const_iterator& aPosition, const value_type& aValue) override { return new (memory) container_iterator(iVector.insert(static_cast<const container_const_iterator&>(aPosition), aValue)); }
    public:
        // from i_vector
        const value_type& operator[](size_type aIndex) const override { return iVector[aIndex]; }
        value_type& operator[](size_type aIndex) override { return iVector[aIndex]; }
        // attributes
    private:
        std::vector<ConcreteType> iVector;
    };
}
