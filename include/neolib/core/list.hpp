// list.hpp
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
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/i_list.hpp>
#include <neolib/core/container_iterator.hpp>

namespace neolib
{
    template <typename T>
    class list : public reference_counted<i_list<abstract_t<T>>>
    {
        typedef reference_counted<i_list<T> > base_type;
        // types
    public:
        typedef i_list<abstract_t<T>> abstract_type;
        typedef T value_type;
        typedef abstract_t<T> abstract_value_type;
        typedef std::list<value_type> container_type;
        using typename abstract_type::size_type;
        using typename abstract_type::const_iterator;
        using typename abstract_type::iterator;
        using typename abstract_type::generic_container_type;
    protected:
        using typename abstract_type::abstract_const_iterator;
        using typename abstract_type::abstract_iterator;
    protected:
        typedef container::const_iterator<T, typename container_type::const_iterator> container_const_iterator;
        typedef container::iterator<T, typename container_type::iterator, typename container_type::const_iterator> container_iterator;
        // construction
    public:
        list() {}
        list(const list& aOther) : 
            iList{ aOther.begin(), aOther.end() } {}
        list(list&& aOther) : 
            iList{ std::move(aOther.iList) } {}
        list(const i_list<T>& aOther) : 
            iList{ aOther.begin(), aOther.end() } {}
        list& operator=(const list& aOther) 
        { 
            assign(aOther); 
            return *this; 
        }
        list& operator=(list&& aOther) 
        { 
            iList = std::move(aOther.iList); 
            return *this; 
        }
        list& operator=(const i_list<T>& aOther) 
        { 
            assign(aOther); 
            return *this; 
        }
        // operations
    public:
        container_type& container() 
        { 
            return iList; 
        }
        const container_type& container() const 
        { 
            return iList; 
        }
        // implementation
    public:
        // from i_container
        size_type size() const override 
        { 
            return iList.size(); 
        }
        size_type max_size() const override 
        { 
            return iList.max_size(); 
        }
        void clear() override 
        { 
            iList.clear(); 
        }
        void assign(const generic_container_type& aOther) override 
        { 
            if (&aOther == this) 
                return; 
            iList.assign(aOther.begin(), aOther.end()); 
        }
    private:
        // from i_container
        abstract_const_iterator* do_begin(void* memory) const override 
        { 
            return new(memory) container_const_iterator(iList.begin()); 
        }
        abstract_const_iterator* do_end(void* memory) const override 
        { 
            return new(memory) container_const_iterator(iList.end()); 
        }
        abstract_iterator* do_begin(void* memory) override 
        { 
            return new(memory) container_iterator(iList.begin()); 
        }
        abstract_iterator* do_end(void* memory) override 
        { 
            return new(memory) container_iterator(iList.end()); 
        }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aPosition) override 
        { 
            return new (memory) container_iterator(iList.erase(static_cast<const container_const_iterator&>(aPosition))); 
        }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) override 
        { 
            return new (memory) container_iterator(iList.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); 
        }
    public:
        // from i_sequence_container
        size_type capacity() const override 
        { 
            return iList.max_size(); 
        }
        void reserve(size_type aCapacity) override 
        { /* do nothing */ 
        }
        void resize(size_type aSize) override 
        { 
            if constexpr (std::is_default_constructible_v<value_type>) 
                iList.resize(aSize); 
            else if (aSize <= size())
                iList.erase(std::next(iList.begin(), aSize), iList.end());
            else
                throw std::logic_error{ "neolib::list::value_type not default constructible" }; 
        }
        void resize(size_type aSize, const abstract_value_type& aValue) override 
        { 
            iList.resize(aSize, aValue); 
        }
        void push_back(const abstract_value_type& aValue) override 
        { 
            iList.push_back(aValue); 
        }
        void pop_back() override 
        { 
            iList.pop_back(); 
        }
        const abstract_value_type& back() const override 
        { 
            return to_abstract(iList.back()); 
        }
        abstract_value_type& back() override 
        { 
            return to_abstract(iList.back()); 
        }
    private:
        // from i_sequence_container
        abstract_iterator* do_insert(void* memory, const abstract_const_iterator& aPosition, const abstract_value_type& aValue) override { return new (memory) container_iterator(iList.insert(static_cast<const container_const_iterator&>(aPosition), aValue)); }
    public:
        // from i_list
        void push_front(const abstract_value_type& aValue) override { iList.push_front(aValue); }
        void pop_front() override { iList.pop_front(); }
        const abstract_value_type& front() const override { return to_abstract(iList.front()); }
        abstract_value_type& front() override { return to_abstract(iList.front()); }
        // attributes
    private:
        std::list<value_type> iList;
    };
}
