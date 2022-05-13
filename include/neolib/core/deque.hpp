// deque.hpp
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
#include <deque>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/i_deque.hpp>
#include <neolib/core/container_iterator.hpp>

namespace neolib
{
    template <typename T>
    class deque : public reference_counted<i_deque<abstract_t<T>>>
    {
        typedef deque<T> self_type;
        typedef reference_counted<i_deque<T>> base_type;
        // types
    public:
        typedef i_deque<abstract_t<T>> abstract_type;
        typedef T value_type;
        typedef abstract_t<T> abstract_value_type;
        typedef std::deque<value_type> std_type;
        using typename abstract_type::size_type;
        using typename abstract_type::const_iterator;
        using typename abstract_type::iterator;
        using typename abstract_type::generic_container_type;
    protected:
        using typename abstract_type::abstract_const_iterator;
        using typename abstract_type::abstract_iterator;
    protected:
        typedef container::random_access_const_iterator<T, typename std_type::const_iterator> container_const_iterator;
        typedef container::random_access_iterator<T, typename std_type::iterator, typename std_type::const_iterator> container_iterator;
        // construction
    public:
        deque() {}
        deque(const deque& aOther) : 
            iDeque(aOther.iDeque) {}
        deque(deque&& aOther) : 
            iDeque(std::move(aOther.iDeque)) {}
        deque(const i_deque<abstract_value_type>& aOther) 
        { 
            assign(aOther); 
        }
        deque(const std_type& aOtherContainer) : 
            iDeque(aOtherContainer) {}
        deque(std::initializer_list<value_type> aValues) : 
            iDeque{ aValues } {}
        template <typename InputIter>
        deque(InputIter aFirst, InputIter aLast) : 
            iDeque(aFirst, aLast) {}
        deque& operator=(const deque& aOther) 
        { 
            assign(aOther); 
            return *this; 
        }
        deque& operator=(deque&& aOther) 
        { 
            iDeque = std::move(aOther.iDeque); 
            return *this; 
        }
        deque& operator=(const i_deque<abstract_value_type>& aOther) 
        { 
            assign(aOther); 
            return *this; 
        }
        // operations
    public:
        std_type& to_std_deque()
        { 
            return iDeque; 
        }
        const std_type& to_std_deque() const
        { 
            return iDeque; 
        }
        template <typename... Args>
        iterator emplace(const_iterator aPos, Args&&... aArgs) 
        { 
            auto newPos = iDeque.emplace(iDeque.begin() + (aPos - abstract_type::cbegin()), std::forward<Args>(aArgs)...); 
            return abstract_type::begin() + (newPos - iDeque.begin()); 
        }
        // comparison
    public:
        constexpr bool operator==(const self_type& that) const noexcept
        {
            return to_std_deque() == that.to_std_deque();
        }
        constexpr std::partial_ordering operator<=>(const self_type& that) const noexcept
        { 
            return to_std_deque() <=> that.to_std_deque();
        }
        // implementation
        // from i_container
    public:
        size_type size() const noexcept final 
        { 
            return iDeque.size(); 
        }
        size_type max_size() const noexcept final
        { 
            return iDeque.max_size(); 
        }
        void clear() final 
        { 
            iDeque.clear(); 
        }
        void assign(const generic_container_type& aOther) final 
        { 
            if (&aOther == this) 
                return; clear(); 
            reserve(aOther.size()); 
            std::copy(aOther.begin(), aOther.end(), std::back_insert_iterator{ iDeque }); 
        }
        // from i_container
    private:
        abstract_const_iterator* do_begin(void* memory) const final 
        { 
            return new (memory) container_const_iterator(iDeque.begin()); 
        }
        abstract_const_iterator* do_end(void* memory) const final 
        { 
            return new (memory) container_const_iterator(iDeque.end()); 
        }
        abstract_iterator* do_begin(void* memory) final 
        { 
            return new (memory) container_iterator(iDeque.begin()); 
        }
        abstract_iterator* do_end(void* memory) final 
        { 
            return new (memory) container_iterator(iDeque.end()); 
        }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aPosition) final 
        { 
            return new (memory) container_iterator(iDeque.erase(static_cast<const container_const_iterator&>(aPosition))); 
        }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) final 
        { 
            return new (memory) container_iterator(iDeque.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); 
        }
        // from i_sequence_container
    public:
        size_type capacity() const final 
        { 
            return iDeque.max_size();
        }
        void reserve(size_type aCapacity) final 
        { 
            /* do nothing */
        }
        void resize(size_type aSize) final 
        { 
            if constexpr (std::is_default_constructible_v<value_type>)
                iDeque.resize(aSize);
            else if (aSize <= size())
                iDeque.erase(std::next(iDeque.begin(), aSize), iDeque.end());
            else
                throw std::logic_error{ "neolib::deque::value_type not default constructible" }; 
        }
        void resize(size_type aSize, const abstract_value_type& aValue) final 
        { 
            iDeque.resize(aSize, aValue); 
        }
        void push_front(const abstract_value_type& aValue) final
        {
            iDeque.push_front(aValue);
        }
        template <typename... Args>
        void emplace_front(Args&&... aArgs)
        {
            iDeque.emplace_front(std::forward<Args>(aArgs)...);
        }
        void pop_front() final
        {
            iDeque.pop_front();
        }
        void push_back(const abstract_value_type& aValue) final
        { 
            iDeque.push_back(aValue); 
        }
        template <typename... Args>
        void emplace_back(Args&&... aArgs) 
        { 
            iDeque.emplace_back(std::forward<Args>(aArgs)...); 
        }
        void pop_back() final 
        { 
            iDeque.pop_back(); 
        }
        const value_type& front() const final
        { 
            return iDeque.front(); 
        }
        value_type& front() final 
        { 
            return iDeque.front(); 
        }
        const value_type& back() const final 
        { 
            return iDeque.back(); 
        }
        value_type& back() final 
        { 
            return iDeque.back(); 
        }
        // from i_random_access_container
    public:
        const value_type& at(size_type aIndex) const final 
        { 
            return iDeque.at(aIndex); 
        }
        value_type& at(size_type aIndex) final 
        { 
            return iDeque.at(aIndex); 
        }
        const value_type& operator[](size_type aIndex) const final 
        { 
            return iDeque[aIndex]; 
        }
        value_type& operator[](size_type aIndex) final 
        { 
            return iDeque[aIndex]; 
        }
        // from i_sequence_container
    private:
        abstract_iterator* do_insert(void* memory, const abstract_const_iterator& aPosition, const abstract_value_type& aValue) final 
        { 
            return new (memory) container_iterator(iDeque.insert(static_cast<const container_const_iterator&>(aPosition), aValue)); 
        }
        // attributes
    private:
        std::deque<value_type> iDeque;
    };
}
