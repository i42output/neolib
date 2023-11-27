// gap_vector.hpp
/*
 *  Copyright (c) 2023 Leigh Johnston.
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
#include <algorithm>
#include <vector>
#include <optional>

namespace neolib
{
    namespace detail::gap_vector
    {
        template <typename Pointer>
        struct gap
        {
            using pointer = Pointer;

            pointer start;
            pointer end;
        };
    }

    template <typename T, std::size_t DefaultGapSize = 256, std::size_t NearnessFactor = 2, typename Allocator = std::allocator<T>>
    class gap_vector
    {
    private:
        using self_type = gap_vector<T, DefaultGapSize, NearnessFactor, Allocator>;
    public:
        using value_type = T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = value_type const&;
        using pointer = std::allocator_traits<Allocator>::pointer;
        using const_pointer = std::allocator_traits<Allocator>::const_pointer;
    private:
        template <typename BaseIterator, typename Reference, typename Pointer>
        class iterator_impl
        {
            friend class self_type;
        protected:
            using base_iterator = BaseIterator;
            using reference = Reference;
            using pointer = Pointer;
        public:
            iterator_impl()
            {
            }
            iterator_impl(iterator_impl const& aOther) :
                iContainer{ &aOther.c()},
                iBase{ aOther.base() }
            {
            }
        private:
            iterator_impl(self_type& aContainer, base_iterator aBase) :
                iContainer{ aContainer },
                iBase{ aBase }
            {
            }
        public:
            iterator_impl& operator=(iterator_impl const& aOther)
            {
                iContainer = &aOther.c();
                iBase = aOther.base();
                return *this;
            }
        public:
            reference operator*() const
            {
                return *base();
            }
            pointer operator->() const
            {
                return base();
            }
            iterator_impl& operator++()
            {
                auto const next = std::distance(c().std_type::begin(), base()) + 1;
                if (!c().gap_active() || next < c().gap_start() || next >= c().gap_end())
                    ++base();
                else
                    base() = std::next(c().std_type::begin(), c().gap_end());
                return *this;
            }
            iterator_impl& operator--()
            {
                auto const next = std::distance(c().std_type::begin(), base()) - 1;
                if (!c().gap_active() || next <= c().gap_start() || next > c().gap_end())
                    --base();
                else
                    base() = std::next(c().std_type::begin(), c().gap_start() - 1);
                return *this;
            }
            iterator_impl& operator+=(difference_type aDifference)
            {
                if (aDifference < 0)
                    return operator-=(-aDifference);
                auto const current = std::distance(c().std_type::begin(), base());
                auto const next = current + aDifference;
                if (!c().gap_active() || (current <= c().gap_start() && next <= c().gap_start()) || (current >= c().gap_end() && next >= c().gap_end()))
                    base() += aDifference;
                else if (current <= c().gap_start() && next >= c().gap_end())
                    base() += (aDifference + c().gap_size());
                return *this;
            }
            iterator_impl& operator-=(difference_type aDifference)
            {
                if (aDifference < 0)
                    return operator+=(-aDifference);
                auto const current = std::distance(c().std_type::begin(), base());
                auto const next = current - aDifference;
                if (!c().gap_active() || (current <= c().gap_start() && next <= c().gap_start()) || (current >= c().gap_end() && next >= c().gap_end()))
                    base() -= aDifference;
                else if (next <= c().gap_start() && current >= c().gap_end())
                    base() -= (aDifference + c().gap_size());
                return *this;
            }
            iterator_impl operator+(difference_type aDifference) const
            { 
                iterator_impl result(*this);
                result += aDifference; 
                return result; 
            }
            iterator_impl operator-(difference_type aDifference) const
            { 
                iterator result(*this); 
                result -= aDifference; 
                return result; 
            }
            reference operator[](difference_type aDifference) const 
            { 
                return *((*this) + aDifference); 
            }
            difference_type operator-(const iterator_impl& aOther) const
            {
                auto result = base() - aOther.base();
                if (c().gap_active())
                {
                    if (c().before_gap(aOther.base()) && c().after_gap(base()))
                        result -= c().gap_size();
                    else if (c().before_gap(base()) && c().after_gap(aOther.base()))
                        result += c().gap_size();
                }
                return result;
            }
            bool operator<(const iterator_impl& aOther) const
            {
                return base() < aOther.base();
            }
        protected:
            constexpr self_type& c() const noexcept
            {
                return *iContainer;
            }
            constexpr base_iterator const& base() const noexcept
            {
                return iBase;
            }
            constexpr base_iterator& base() noexcept
            {
                return iBase;
            }
        protected:
            self_type* iContainer = nullptr;
            base_iterator iBase = {};
        };
    public:
        class iterator : public iterator_impl<pointer, reference, pointer>
        {
            friend class self_type;
        private:
            using base_type = iterator_impl<pointer, reference, pointer>;
        public:
            iterator() : 
                iterator_impl{}
            {
            }
            iterator(iterator const& aOther) :
                iterator_impl{ aOther }
            {
            }
        private:
            iterator(self_type& aContainer, iterator aBase) :
                iterator_impl{ aContainer, aBase }
            {
            }
        public:
            iterator& operator=(iterator const& aOther)
            {
                base_type::operator=(aOther);
                return *this;
            }
            iterator& operator++()
            {
                base_type::operator++();
                return *this;
            }
            iterator operator++(int)
            {
                iterator temp = *this;
                operator++();
                return temp;
            }
            iterator& operator--()
            {
                base_type::operator--();
                return *this;
            }
            iterator operator--(int)
            {
                iterator temp = *this;
                operator--();
                return temp;
            }
            iterator& operator+=(difference_type aDifference)
            {
                base_type::operator+=(aDifference);
                return *this;
            }
            iterator& operator-=(difference_type aDifference)
            {
                base_type::operator-=(aDifference);
                return *this;
            }
            iterator operator+(difference_type aDifference) const
            {
                return base_type::operator+(aDifference);
            }
            iterator operator-(difference_type aDifference) const
            {
                return base_type::operator-(aDifference);
            }
        };
        class const_iterator : public iterator_impl<const_pointer, const_reference, const_pointer>
        {
            friend class self_type;
        private:
            using base_type = iterator_impl<const_pointer, const_reference, const_pointer>;
        public:
            const_iterator() :
                iterator_impl{}
            {
            }
            const_iterator(const_iterator const& aOther) :
                iterator_impl{ aOther }
            {
            }
            const_iterator(iterator const& aOther) :
                const_iterator{ aOther.c(), aOther.base() }
            {
            }
        private:
            const_iterator(self_type& aContainer, const_iterator aBase) :
                iterator_impl{ aContainer, aBase }
            {
            }
        public:
            const_iterator& operator=(const_iterator const& aOther)
            {
                base_type::operator=(aOther);
                return *this;
            }
            const_iterator& operator=(iterator const& aOther)
            {
                base_type::operator=(const_iterator{ aOther });
                return *this;
            }
            const_iterator& operator++()
            {
                base_type::operator++();
                return *this;
            }
            const_iterator operator++(int)
            {
                iterator temp = *this;
                operator++();
                return temp;
            }
            const_iterator& operator--()
            {
                base_type::operator--();
                return *this;
            }
            const_iterator operator--(int)
            {
                iterator temp = *this;
                operator--();
                return temp;
            }
            const_iterator& operator+=(difference_type aDifference)
            {
                base_type::operator+=(aDifference);
                return *this;
            }
            const_iterator& operator-=(difference_type aDifference)
            {
                base_type::operator-=(aDifference);
                return *this;
            }
            const_iterator operator+(difference_type aDifference) const
            {
                return base_type::operator+(aDifference);
            }
            const_iterator operator-(difference_type aDifference) const
            {
                return base_type::operator-(aDifference);
            }
        };
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    private:
        using gap_type = detail::gap_vector::gap<pointer>;
    public:
    public:
        constexpr allocator_type get_allocator() const noexcept
        {
            return iAlloc;
        }
    public:
        constexpr const_reference at(size_type pos) const
        {
            pos = adjusted_index(pos);
            if (pos < iDataEnd - iData)
                return iData[pos];
            throw std::out_of_range("neolib::gap_vector::at");
        }
        constexpr reference at(size_type pos)
        {
            pos = adjusted_index(pos);
            if (pos < iDataEnd - iData)
                return iData[pos];
            throw std::out_of_range("neolib::gap_vector::at");
        }
        constexpr const_reference operator[](size_type pos) const
        {
            return iData[adjusted_index(pos)];
        }
        constexpr reference operator[](size_type pos)
        {
            return iData[adjusted_index(pos)];
        }
        constexpr const_reference front() const
        {
            return *begin();
        }
        constexpr reference front()
        {
            return *begin();
        }
        constexpr const_reference back() const
        {
            return *std::prev(end());
        }
        constexpr reference back()
        {
            return *std::prev(end());
        }
        constexpr T const* data() const
        {
            unsplit();
            return iData;
        }
        constexpr T* data()
        {
            unsplit();
            return iData;
        }
    public:
        constexpr const_iterator begin() const noexcept
        {
            return const_iterator{ *this, iData };
        }
        constexpr const_iterator cbegin() const noexcept
        {
            return const_iterator{ *this, iData };
        }
        constexpr iterator begin() noexcept
        {
            return iterator{ *this, iData };
        }
        constexpr const_iterator end() const noexcept
        {
            return const_iterator{ *this, iDataEnd };
        }
        constexpr const_iterator cend() const noexcept
        {
            return const_iterator{ *this, iDataEnd };
        }
        constexpr iterator end() noexcept
        {
            return iterator{ *this, iDataEnd };
        }
        constexpr const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator{ const_iterator{ *this, iDataEnd } };
        }
        constexpr const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator{ const_iterator{ *this, iDataEnd } };
        }
        constexpr reverse_iterator rbegin() noexcept
        {
            return reverse_iterator{ iterator{ *this, iDataEnd } };
        }
        constexpr const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator{ const_iterator{ *this, iData } };
        }
        constexpr const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator{ const_iterator{ *this, iData } };
        }
        constexpr reverse_iterator rend() noexcept
        {
            return reverse_iterator{ iterator{ *this, iData } };
        }
    public:
        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return begin() == end();
        }
        constexpr size_type size() const noexcept
        {
            return std_type::size() - gap_size();
        }
        constexpr size_type max_size() const noexcept
        {
            return std::numeric_limits<difference_type>::max();
        }
        constexpr void reserve(size_type aNewCapacity)
        {
            if (aNewCapacity <= capacity())
                return;

            unsplit();

            auto existingData = iData;
            auto existingDataEnd = iDataEnd;
            auto existingStorageEnd = iStorageEnd;

            pointer newStorage = iAlloc.allocate(aNewCapacity);

            try
            {
                iData = newStorage;
                iDataEnd = newStorage;
                iStorageEnd = newStorage + aNewCapacity;

                iDataEnd = std::uninitialized_move(existingData, existingDataEnd, iData);
            }
            catch(...)
            {
                iAlloc.deallocate(newStorage);

                iData = existingData;
                iDataEnd = existingDataEnd;
                iStorageEnd = existingStorageEnd;

                throw;
            }
        }
        constexpr size_type capacity() const noexcept
        {
            return iStorageEnd - iData;
        }
        constexpr void shrink_to_fit()
        {
            self_type{ *this }.swap(*this);
        }
    public:
        constexpr void clear() noexcept
        {
            try
            {
                for (auto e = iData; e != iDataEnd; ++e)
                {
                    if (gap_active() && e >= gap().start && e < gap().end)
                        continue;
                    std::allocator_traits<allocator_type>::destroy(iAlloc, e);
                }
            }
            catch (...)
            {
            }

            iDataEnd = iData;
            iGap = std::nullopt;
        }
        constexpr iterator insert(const_iterator pos, const T& value)
        {
            std::allocator_traits<allocator_type>::construct(iAlloc, (pos = move_gap(pos, 1)), value);
            consume_gap();
            return iterator{ *this, pos };
        }
        constexpr iterator insert(const_iterator pos, T&& value)
        {
            std::allocator_traits<allocator_type>::construct(iAlloc, (pos = move_gap(pos, 1)), std::move(value));
            consume_gap();
            return iterator{ *this, pos };
        }
        constexpr iterator insert(const_iterator pos, size_type count, const T& value)
        {
            std::unitialized_copy_n(value, count, (pos = move_gap(pos, count)));
            consume_gap(count);
            return iterator{ *this, pos };
        }
        template<class InputIt>
        constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
        {
            size_type count = 0;
            while (first != last)
            {
                ++count;
                std::unitialized_copy_n(first++, 1, (pos = move_gap(pos, 1)));
                consume_gap();
            }
            return iterator{ *this, pos };
        }
        constexpr iterator insert(const_iterator pos, std::initializer_list<T> list)
        {
            std::unitialized_copy(list.begin(), list.end(), (pos = move_gap(pos, list.size())));
            consume_gap(list.size());
            return iterator{ *this, pos };
        }
        template<class... Args>
        constexpr iterator emplace(const_iterator pos, Args&&... args)
        {
            std::allocator_traits<allocator_type>::construct(iAlloc, pos = move_gap(pos, 1), std::forward<Args>(args)...);
            consume_gap();
            return pos;
        }
    public:
        constexpr void unsplit() const
        {
            if (!gap_active())
                return;
            std::unitialized_move(gap().end, std::min(gap().end + gap_size(), iDataEnd), gap().start);
            auto const newEnd = std::move(std::min(gap().end + gap_size(), iDataEnd), iDataEnd, gap().start);
            std::erase(newEnd, iDataEnd);
            iDataEnd = newEnd;
            iGap = std::nullopt;
        }
    private:
        constexpr size_type adjusted_index(size_type pos) const noexcept
        {
            if (!gap_active() || pos < gap_start())
                return pos;
            return pos + gap_size();
        }
        constexpr bool gap_active() const noexcept
        {
            return iGap != std::nullopt;
        }
        constexpr gap& gap() const
        {
            return gap.value();
        }
        constexpr size_type gap_start() const
        {
            return gap().start - data();
        }
        constexpr size_type gap_end() const
        {
            return gap().end - data();
        }
        constexpr size_type gap_size() const noexcept
        {
            if (gap_active())
                return gap().end - gap().start;
            return 0;
        }
        constexpr bool before_gap(const_iterator aPosition) const noexcept
        {
            return gap_active() && aPosition.base() < gap().start;
        }
        constexpr bool after_gap(const_iterator aPosition) const noexcept
        {
            return gap_active() && aPosition.base() >= gap().end;
        }
        constexpr bool near_gap(const_iterator aPosition) const noexcept
        {
            if (!gap_active())
                return false;
            return std::abs(aPosition.base() - gap().start + ((gap().end - gap().start) / 2)) <= DefaultGapSize * NearnessFactor;
        }
        constexpr void move_gap(const_iterator aPosition, size_type aCount)
        {
            if (near_gap(aPosition) && aCount <= gap_size())
            {
                if (before_gap(aPosition))
                {
                }
                else
                {
                }
            }
            else
            {
            }
        }
        constexpr void consume_gap(size_type aCount = 1)
        {
        }
    private:
        allocator iAlloc;
        mutable pointer iData = nullptr;
        mutable pointer iDataEnd = nullptr;
        mutable pointer iStorageEnd = nullptr;
        mutable std::optional<gap_type> iGap;
    };
}
