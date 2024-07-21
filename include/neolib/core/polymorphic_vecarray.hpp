// polymorphic_vecarray.hpp
/*
 *  Copyright (c) 2007,2023,2024 Leigh Johnston.
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
#include <neolib/core/small_buffer_allocator.hpp>
#include <neolib/core/i_vector.hpp>
#include <neolib/core/container_iterator.hpp>

namespace neolib
{
    namespace polymorphic
    {
        template<typename T, std::size_t Capacity, std::size_t MaxCapacity = Capacity>
        class vecarray : public reference_counted<i_vector<abstract_t<T>>>
        {
            using base_type = reference_counted<i_vector<abstract_t<T>>>;
            // types
        public:
            using abstract_type = i_vector<abstract_t<T>>;
            using value_type = T;
            using abstract_value_type = abstract_t<T>;
            using allocator_type = small_buffer_allocator<T, Capacity, MaxCapacity>;
            using std_type = std::vector<value_type, allocator_type>;
            using size_type = typename abstract_type::size_type;
            using const_iterator = typename abstract_type::const_iterator;
            using iterator = typename abstract_type::iterator;
            using generic_container_type = typename abstract_type::generic_container_type;
        protected:
            using abstract_const_iterator = typename abstract_type::abstract_const_iterator;
            using abstract_iterator = typename abstract_type::abstract_iterator;
        protected:
            using container_const_iterator = container::random_access_const_iterator<T, typename std_type::const_iterator>;
            using container_iterator = container::random_access_iterator<T, typename std_type::iterator, typename std_type::const_iterator>;
            // construction
        public:
            vecarray() :
                iVector{ allocator_type{ iSmallBuffer } }
            {
                iVector.reserve(Capacity);
            }
            vecarray(vecarray const& aOther) :
                iVector{ aOther.iVector, allocator_type{ iSmallBuffer } }
            {
                iVector.reserve(Capacity);
            }
            vecarray(vecarray&& aOther) :
                iVector{ std::move(aOther.iVector), allocator_type{ iSmallBuffer } }
            {
                iVector.reserve(Capacity);
            }
            vecarray(i_vector<abstract_value_type> const& aOther) :
                iVector{ allocator_type{ iSmallBuffer } }
            {
                iVector.reserve(Capacity);
                assign(aOther);
            }
            vecarray(std_type const& aOtherContainer) :
                iVector{ aOtherContainer, allocator_type{ iSmallBuffer } }
            {
                iVector.reserve(Capacity);
            }
            vecarray(std::initializer_list<value_type> aValues) :
                iVector{ aValues, allocator_type{ iSmallBuffer } }
            {
                iVector.reserve(Capacity);
            }
            template <typename InputIter>
            vecarray(InputIter aFirst, InputIter aLast) :
                iVector{ aFirst, aLast, allocator_type{ iSmallBuffer } }
            {
                iVector.reserve(Capacity);
            }
            vecarray& operator=(vecarray const& aOther)
            {
                assign(aOther);
                return *this;
            }
            vecarray& operator=(vecarray&& aOther)
            {
                iVector = std::move(aOther.iVector);
                return *this;
            }
            vecarray& operator=(i_vector<abstract_value_type> const& aOther)
            {
                assign(aOther);
                return *this;
            }
            // operations
        public:
            const std_type& as_std_vector() const
            {
                return iVector;
            }
            std_type& as_std_vector()
            {
                return iVector;
            }
            std::vector<T> to_std_vector() const
            {
                return std::vector<T>{ iVector.begin(), iVector.end() };
            }
            using base_type::insert;
            iterator insert(const_iterator aPos, const_iterator aFirst, const_iterator aLast)
            {
                auto newPos = iVector.insert(iVector.begin() + (aPos - abstract_type::cbegin()), aFirst, aLast);
                return abstract_type::begin() + (newPos - iVector.begin());
            }
            template <typename InputIter>
            iterator insert(const_iterator aPos, InputIter aFirst, InputIter aLast)
            {
                auto newPos = iVector.insert(iVector.begin() + (aPos - abstract_type::cbegin()), aFirst, aLast);
                return abstract_type::begin() + (newPos - iVector.begin());
            }
            template <typename... Args>
            iterator emplace(const_iterator aPos, Args&&... aArgs)
            {
                auto newPos = iVector.emplace(iVector.begin() + (aPos - abstract_type::cbegin()), std::forward<Args>(aArgs)...);
                return abstract_type::begin() + (newPos - iVector.begin());
            }
            // comparison
        public:
            constexpr bool operator==(const vecarray& that) const noexcept
            {
                return as_std_vector() == that.as_std_vector();
            }
            constexpr std::partial_ordering operator<=>(const vecarray& that) const noexcept
            {
                return as_std_vector() <=> that.as_std_vector();
            }
            // implementation
            // from i_container
        public:
            size_type size() const noexcept final
            {
                return iVector.size();
            }
            size_type max_size() const noexcept final
            {
                return iVector.max_size();
            }
            size_type available() const noexcept
            {
                return max_size() - size();
            }
            void clear() final
            {
                iVector.clear();
            }
            void assign(generic_container_type const& aOther) final
            {
                if (&aOther == this)
                    return; clear();
                reserve(aOther.size());
                std::copy(aOther.begin(), aOther.end(), std::back_insert_iterator{ iVector });
            }
            // from i_container
        private:
            abstract_const_iterator* do_begin(void* memory) const final
            {
                return new (memory) container_const_iterator(iVector.begin());
            }
            abstract_const_iterator* do_end(void* memory) const final
            {
                return new (memory) container_const_iterator(iVector.end());
            }
            abstract_iterator* do_begin(void* memory) final
            {
                return new (memory) container_iterator(iVector.begin());
            }
            abstract_iterator* do_end(void* memory) final
            {
                return new (memory) container_iterator(iVector.end());
            }
            abstract_iterator* do_erase(void* memory, abstract_const_iterator const& aPosition) final
            {
                return new (memory) container_iterator(iVector.erase(static_cast<container_const_iterator const&>(aPosition)));
            }
            abstract_iterator* do_erase(void* memory, abstract_const_iterator const& aFirst, abstract_const_iterator const& aLast) final
            {
                return new (memory) container_iterator(iVector.erase(static_cast<container_const_iterator const&>(aFirst), static_cast<container_const_iterator const&>(aLast)));
            }
            // from i_sequence_container
        public:
            size_type capacity() const final
            {
                return iVector.capacity();
            }
            void reserve(size_type aCapacity) final
            {
                iVector.reserve(aCapacity);
            }
            void resize(size_type aSize) final
            {
                if constexpr (std::is_default_constructible_v<value_type>)
                    iVector.resize(aSize);
                else if (aSize <= size())
                    iVector.erase(std::next(iVector.begin(), aSize), iVector.end());
                else
                    throw std::logic_error{ "neolib::vector::value_type not default constructible" };
            }
            void resize(size_type aSize, abstract_value_type const& aValue) final
            {
                iVector.resize(aSize, aValue);
            }
            void push_back(abstract_value_type const& aValue) final
            {
                iVector.push_back(aValue);
            }
            template <typename... Args>
            void emplace_back(Args&&... aArgs)
            {
                iVector.emplace_back(std::forward<Args>(aArgs)...);
            }
            void pop_back() final
            {
                iVector.pop_back();
            }
            const value_type& front() const final
            {
                return iVector.front();
            }
            value_type& front() final
            {
                return iVector.front();
            }
            const value_type& back() const final
            {
                return iVector.back();
            }
            value_type& back() final
            {
                return iVector.back();
            }
            // from i_random_access_container
        public:
            const value_type* cdata() const noexcept final
            {
                return iVector.data();
            }
            const value_type* data() const noexcept final
            {
                return iVector.data();
            }
            value_type* data() noexcept final
            {
                return iVector.data();
            }
        public:
            const value_type& at(size_type aIndex) const final
            {
                return iVector.at(aIndex);
            }
            value_type& at(size_type aIndex) final
            {
                return iVector.at(aIndex);
            }
            const value_type& operator[](size_type aIndex) const final
            {
                return iVector[aIndex];
            }
            value_type& operator[](size_type aIndex) final
            {
                return iVector[aIndex];
            }
        private:
            std::ptrdiff_t iterator_offset() const final
            {
                return sizeof(value_type);
            }
            // from i_sequence_container
        private:
            abstract_iterator* do_insert(void* memory, abstract_const_iterator const& aPosition, abstract_value_type const& aValue) final
            {
                return new (memory) container_iterator(iVector.insert(static_cast<container_const_iterator const&>(aPosition), aValue));
            }
            // attributes
        private:
            small_buffer<value_type, Capacity> iSmallBuffer;
            std_type iVector;
        };
    }
}
