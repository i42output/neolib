// vector.hpp
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
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/i_vector.hpp>
#include <neolib/core/container_iterator.hpp>

namespace neolib
{
    template <typename T>
    class vector : public reference_counted<i_vector<abstract_t<T>>>
    {
        typedef vector<T> self_type;
        typedef reference_counted<i_vector<T>> base_type;
        // types
    public:
        typedef i_vector<abstract_t<T>> abstract_type;
        typedef T value_type;
        typedef abstract_t<T> abstract_value_type;
        typedef std::vector<value_type> std_type;
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
        vector() {}
        vector(vector const& aOther) :
            iVector(aOther.iVector) {}
        vector(vector&& aOther) : 
            iVector(std::move(aOther.iVector)) {}
        vector(i_vector<abstract_value_type> const& aOther)
        { 
            assign(aOther); 
        }
        vector(std_type const& aOtherContainer) :
            iVector(aOtherContainer) {}
        vector(std::initializer_list<value_type> aValues) : 
            iVector{ aValues } {}
        template <typename InputIter>
        vector(InputIter aFirst, InputIter aLast) : 
            iVector(aFirst, aLast) {}
        vector& operator=(vector const& aOther)
        { 
            assign(aOther); 
            return *this; 
        }
        vector& operator=(vector&& aOther) 
        { 
            iVector = std::move(aOther.iVector); 
            return *this; 
        }
        vector& operator=(i_vector<abstract_value_type> const& aOther)
        { 
            assign(aOther); 
            return *this; 
        }
        // operations
    public:
        std_type& to_std_vector()
        { 
            return iVector; 
        }
        const std_type& to_std_vector() const
        { 
            return iVector; 
        }
        template <typename... Args>
        iterator emplace(const_iterator aPos, Args&&... aArgs) 
        { 
            auto newPos = iVector.emplace(iVector.begin() + (aPos - abstract_type::cbegin()), std::forward<Args>(aArgs)...); 
            return abstract_type::begin() + (newPos - iVector.begin()); 
        }
        // comparison
    public:
        bool operator<(const self_type& aRhs) const 
        { 
            return to_std_vector() < aRhs.to_std_vector();
        }
        // implementation
        // from i_container
    public:
        size_type size() const override 
        { 
            return iVector.size(); 
        }
        size_type max_size() const override 
        { 
            return iVector.max_size(); 
        }
        void clear() override 
        { 
            iVector.clear(); 
        }
        void assign(generic_container_type const& aOther) override
        { 
            if (&aOther == this) 
                return; clear(); 
            reserve(aOther.size()); 
            std::copy(aOther.begin(), aOther.end(), std::back_insert_iterator{ iVector }); 
        }
        // from i_container
    private:
        abstract_const_iterator* do_begin(void* memory) const override 
        { 
            return new (memory) container_const_iterator(iVector.begin()); 
        }
        abstract_const_iterator* do_end(void* memory) const override 
        { 
            return new (memory) container_const_iterator(iVector.end()); 
        }
        abstract_iterator* do_begin(void* memory) override 
        { 
            return new (memory) container_iterator(iVector.begin()); 
        }
        abstract_iterator* do_end(void* memory) override 
        { 
            return new (memory) container_iterator(iVector.end()); 
        }
        abstract_iterator* do_erase(void* memory, abstract_const_iterator const& aPosition) override
        { 
            return new (memory) container_iterator(iVector.erase(static_cast<container_const_iterator const&>(aPosition)));
        }
        abstract_iterator* do_erase(void* memory, abstract_const_iterator const& aFirst, abstract_const_iterator const& aLast) override
        { 
            return new (memory) container_iterator(iVector.erase(static_cast<container_const_iterator const&>(aFirst), static_cast<container_const_iterator const&>(aLast)));
        }
        // from i_sequence_container
    public:
        size_type capacity() const override 
        { 
            return iVector.capacity(); 
        }
        void reserve(size_type aCapacity) override 
        { 
            iVector.reserve(aCapacity); 
        }
        void resize(size_type aSize) override 
        { 
            if constexpr (std::is_default_constructible_v<value_type>)
                iVector.resize(aSize);
            else if (aSize <= size())
                iVector.erase(std::next(iVector.begin(), aSize), iVector.end());
            else
                throw std::logic_error{ "neolib::vector::value_type not default constructible" }; 
        }
        void resize(size_type aSize, abstract_value_type const& aValue) override
        { 
            iVector.resize(aSize, aValue); 
        }
        void push_back(abstract_value_type const& aValue) override
        { 
            iVector.push_back(aValue); 
        }
        template <typename... Args>
        void emplace_back(Args&&... aArgs) 
        { 
            iVector.emplace_back(std::forward<Args>(aArgs)...); 
        }
        void pop_back() override 
        { 
            iVector.pop_back(); 
        }
        const value_type& front() const override 
        { 
            return iVector.front(); 
        }
        value_type& front() override 
        { 
            return iVector.front(); 
        }
        const value_type& back() const override 
        { 
            return iVector.back(); 
        }
        value_type& back() override 
        { 
            return iVector.back(); 
        }
        // from i_random_access_container
    public:
        const value_type* cdata() const override 
        { 
            return iVector.data(); 
        }
        const value_type* data() const override 
        { 
            return iVector.data(); 
        }
        value_type* data() override 
        { 
            return iVector.data(); 
        }
    public:
        const value_type& at(size_type aIndex) const override 
        { 
            return iVector.at(aIndex); 
        }
        value_type& at(size_type aIndex) override 
        { 
            return iVector.at(aIndex); 
        }
        const value_type& operator[](size_type aIndex) const override 
        { 
            return iVector[aIndex]; 
        }
        value_type& operator[](size_type aIndex) override 
        { 
            return iVector[aIndex]; 
        }
    private:
        std::ptrdiff_t iterator_offset() const override 
        { 
            return sizeof(value_type); 
        }
        // from i_sequence_container
    private:
        abstract_iterator* do_insert(void* memory, abstract_const_iterator const& aPosition, abstract_value_type const& aValue) override
        { 
            return new (memory) container_iterator(iVector.insert(static_cast<container_const_iterator const&>(aPosition), aValue));
        }
        // attributes
    private:
        std::vector<value_type> iVector;
    };
}
