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
    template <typename T, std::size_t DefaultGapSize_ = 256, std::size_t NearnessFactor_ = 2, typename Allocator = std::allocator<T>>
    class gap_vector
    {
    private:
        using self_type = gap_vector<T, DefaultGapSize_, NearnessFactor_, Allocator>;
    public:
        using value_type = T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = value_type const&;
        using pointer = std::allocator_traits<Allocator>::pointer;
        using const_pointer = std::allocator_traits<Allocator>::const_pointer;
    public:
        static constexpr size_type DefaultGapSize = DefaultGapSize_;
        static constexpr size_type NearnessFactor = NearnessFactor_;
    private:
        template <typename ContainerType, typename BaseIterator, typename Reference, typename Pointer>
        class iterator_impl
        {
            friend class self_type;
        private:
            using container_type = ContainerType;
        protected:
            using base_iterator = BaseIterator;
            using reference = Reference;
            using pointer = Pointer;
        public:
            iterator_impl()
            {
            }
        protected:
            iterator_impl(container_type& aContainer, base_iterator aBase) :
                iContainer{ &aContainer },
                iBase{ aBase }
            {
            }
            iterator_impl(iterator_impl const& aOther) :
                iContainer{ aOther.iContainer },
                iBase{ aOther.iBase }
            {
            }
            iterator_impl(iterator_impl&& aOther) :
                iContainer{ aOther.iContainer },
                iBase{ aOther.iBase }
            {
                aOther.iContainer = nullptr;
                aOther.iBase = nullptr;
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
                return *iBase;
            }
            pointer operator->() const
            {
                return iBase;
            }
            iterator_impl& operator++()
            {
                auto const next = std::next(iBase);
                if (!c().gap_active() || next < c().iGapStart || (c().iGapEnd == c().iDataEnd && next == c().iGapStart) || next > c().iGapEnd)
                    ++iBase;
                else
                    iBase = c().iGapEnd;
                return *this;
            }
            iterator_impl& operator--()
            {
                auto const prev = std::prev(iBase);
                if (!c().gap_active() || prev < c().iGapStart || prev > c().iGapEnd)
                    --iBase;
                else
                    iBase = std::prev(c().iGapStart);
                return *this;
            }
            iterator_impl& operator+=(difference_type aDifference)
            {
                if (aDifference == 0)
                    return *this;
                else if (aDifference < 0)
                    return operator-=(-aDifference);
                auto const current = iBase;
                auto const next = current + aDifference;
                if (!c().gap_active() || next <= c().iGapStart || current >= c().iGapEnd)
                    iBase += aDifference;
                else
                    iBase += (aDifference + c().gap_size());
                return *this;
            }
            iterator_impl& operator-=(difference_type aDifference)
            {
                if (aDifference == 0)
                    return *this;
                else if (aDifference < 0)
                    return operator+=(-aDifference);
                auto const current = iBase;
                auto const next = current - aDifference;
                if (!c().gap_active() || current < c().iGapStart || next >= c().iGapEnd)
                    iBase -= aDifference;
                else
                    iBase -= (aDifference + c().gap_size());
                return *this;
            }
            iterator_impl operator+(difference_type aDifference) const
            { 
                iterator_impl result{ *this };
                result += aDifference; 
                return result; 
            }
            iterator_impl operator-(difference_type aDifference) const
            { 
                iterator_impl result{ *this };
                result -= aDifference; 
                return result; 
            }
            reference operator[](difference_type aDifference) const 
            { 
                return *((*this) + aDifference); 
            }
            difference_type operator-(const iterator_impl& aOther) const
            {
                auto result = iBase - aOther.base();
                if (c().gap_active())
                {
                    if (c().before_gap(aOther.base()) && c().after_gap(iBase))
                        result -= c().gap_size();
                    else if (c().before_gap(iBase) && c().after_gap(aOther.base()))
                        result += c().gap_size();
                }
                return result;
            }
            auto operator<=>(const iterator_impl&) const = default;
        protected:
            constexpr container_type& c() const noexcept
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
        private:
            container_type* iContainer = nullptr;
            base_iterator iBase = {};
        };
    public:
        class iterator : public iterator_impl<self_type, pointer, reference, pointer>
        {
            friend class self_type;
        private:
            using base_type = iterator_impl<self_type, pointer, reference, pointer>;
            using container_type = self_type;
        public:
            using difference_type = typename container_type::difference_type;
            using value_type = typename container_type::value_type;
            using pointer = typename container_type::pointer;
            using reference = typename container_type::reference;
            using iterator_category = std::random_access_iterator_tag;
        public:
            iterator() : 
                base_type{}
            {
            }
            iterator(iterator const& aOther) :
                base_type{ aOther }
            {
            }
        private:
            iterator(container_type& aContainer, pointer aBase) :
                base_type{ aContainer, aBase }
            {
            }
            iterator(base_type& aOther) :
                base_type{ aOther }
            {
            }
            iterator(base_type&& aOther) :
                base_type{ aOther }
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
                return iterator{ base_type::operator+(aDifference) };
            }
            iterator operator-(difference_type aDifference) const
            {
                return iterator{ base_type::operator-(aDifference) };
            }
            using base_type::operator-;
        };
        class const_iterator : public iterator_impl<self_type const, const_pointer, const_reference, const_pointer>
        {
            friend class self_type;
        private:
            using base_type = iterator_impl<self_type const, const_pointer, const_reference, const_pointer>;
            using container_type = self_type const;
        public:
            using difference_type = typename container_type::difference_type;
            using value_type = typename container_type::value_type;
            using pointer = typename container_type::const_pointer;
            using reference = typename container_type::const_reference;
            using iterator_category = std::random_access_iterator_tag;
        public:
            const_iterator() :
                base_type{}
            {
            }
            const_iterator(const_iterator const& aOther) :
                base_type{ aOther }
            {
            }
            const_iterator(iterator const& aOther) :
                const_iterator{ aOther.c(), aOther.base() }
            {
            }
        private:
            const_iterator(container_type& aContainer, pointer aBase) :
                base_type{ aContainer, aBase }
            {
            }
            const_iterator(base_type&& aOther) :
                base_type{ aOther }
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
                const_iterator temp = *this;
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
                const_iterator temp = *this;
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
                return const_iterator{ base_type::operator+(aDifference) };
            }
            const_iterator operator-(difference_type aDifference) const
            {
                return const_iterator{ base_type::operator-(aDifference) };
            }
            using base_type::operator-;
        };
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    public:
        constexpr gap_vector() noexcept(noexcept(Allocator()))
        {
        }
        constexpr explicit gap_vector(const Allocator& alloc) noexcept : iAlloc{ alloc }
        {
        }
        constexpr gap_vector(size_type count, const value_type& value, const Allocator& alloc = Allocator()) : iAlloc{ alloc }
        {
            resize(count, value);
        }
        constexpr explicit gap_vector(size_type count, const Allocator& alloc = Allocator()) : iAlloc{ alloc }
        {
            resize(count);
        }
        template<class InputIt>
        constexpr gap_vector(InputIt first, InputIt last, const Allocator& alloc = Allocator()) : iAlloc{ alloc }
        {
            (void)insert(begin(), first, last);
        }
        constexpr gap_vector(const gap_vector& other)
        {
            (void)insert(begin(), other.begin(), other.end());
        }
        constexpr gap_vector(const gap_vector& other, const Allocator& alloc) : iAlloc{ alloc }
        {
            (void)insert(begin(), other.begin(), other.end());
        }
        constexpr gap_vector(gap_vector&& other) noexcept
        {
            swap(other);
        }
        constexpr gap_vector(gap_vector&& other, const Allocator& alloc) : iAlloc{ alloc }
        {
            swap(other);
        }
        constexpr gap_vector(std::initializer_list<T> init, const Allocator& alloc = Allocator()) : iAlloc{ alloc }
        {
            (void)insert(begin(), init);
        }
        ~gap_vector()
        {
            clear();
        }
    public:
        constexpr void swap(self_type& other) noexcept
        {
            std::swap(iData, other.iData);
            std::swap(iDataEnd, other.iDataEnd);
            std::swap(iStorageEnd, other.iStorageEnd);
            std::swap(iGapStart, other.iGapStart);
            std::swap(iGapEnd, other.iGapEnd);
        }
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
            return const_iterator{ *this, !gap_active() || iData != iGapStart ? iData : iGapEnd };
        }
        constexpr const_iterator cbegin() const noexcept
        {
            return const_iterator{ *this, !gap_active() || iData != iGapStart ? iData : iGapEnd };
        }
        constexpr iterator begin() noexcept
        {
            return iterator{ *this, !gap_active() || iData != iGapStart ? iData : iGapEnd };
        }
        constexpr const_iterator end() const noexcept
        {
            return const_iterator{ *this, !gap_active() || iDataEnd != iGapEnd ? iDataEnd : iGapStart };
        }
        constexpr const_iterator cend() const noexcept
        {
            return const_iterator{ *this, !gap_active() || iDataEnd != iGapEnd ? iDataEnd : iGapStart };
        }
        constexpr iterator end() noexcept
        {
            return iterator{ *this, !gap_active() || iDataEnd != iGapEnd ? iDataEnd : iGapStart };
        }
        constexpr const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator{ const_iterator{ *this, !gap_active() || iDataEnd != iGapEnd ? iDataEnd : iGapStart } };
        }
        constexpr const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator{ const_iterator{ *this, !gap_active() || iDataEnd != iGapEnd ? iDataEnd : iGapStart } };
        }
        constexpr reverse_iterator rbegin() noexcept
        {
            return reverse_iterator{ iterator{ *this, !gap_active() || iDataEnd != iGapEnd ? iDataEnd : iGapStart } };
        }
        constexpr const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator{ const_iterator{ *this, !gap_active() || iData != iGapStart ? iData : iGapEnd } };
        }
        constexpr const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator{ const_iterator{ *this, !gap_active() || iData != iGapStart ? iData : iGapEnd } };
        }
        constexpr reverse_iterator rend() noexcept
        {
            return reverse_iterator{ iterator{ *this, !gap_active() || iData != iGapStart ? iData : iGapEnd } };
        }
    public:
        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return begin() == end();
        }
        constexpr size_type size() const noexcept
        {
            return (iDataEnd - iData) - gap_size();
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

            pointer newStorage = std::allocator_traits<allocator_type>::allocate(iAlloc, aNewCapacity);

            try
            {
                iData = newStorage;
                iDataEnd = newStorage;
                iStorageEnd = newStorage + aNewCapacity;

                iDataEnd = std::uninitialized_move(existingData, existingDataEnd, iData);
            }
            catch(...)
            {
                std::allocator_traits<allocator_type>::deallocate(iAlloc, newStorage, aNewCapacity);
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
            for (auto e = iData; e != iDataEnd; ++e)
            {
                if (gap_active() && e >= iGapStart && e < iGapEnd)
                    continue;
                std::allocator_traits<allocator_type>::destroy(iAlloc, e);
            }
        }
        constexpr iterator erase(const_iterator pos)
        {
            return erase(pos, std::next(pos));
        }
        constexpr iterator erase(const_iterator first, const_iterator last)
        {
            if (first == last)
                return std::next(begin(), std::distance(cbegin(), last));
            auto const firstIndex = std::distance(cbegin(), first);
            auto const lastIndex = std::distance(cbegin(), last);
            auto const firstPos = std::next(begin(), firstIndex).base();
            auto const lastPos = std::next(begin(), lastIndex).base();
            auto const garbageCount = lastIndex - firstIndex;
            if (near_gap(firstPos) || near_gap(lastPos))
            {
                if (before_gap(std::prev(lastPos)))
                {
                    for (auto src = lastPos, dest = firstPos; src < iGapStart; ++src, ++dest)
                        *dest = std::move(*src);
                    for (auto garbage = iGapStart - garbageCount; garbage != iGapStart; ++garbage)
                        std::allocator_traits<allocator_type>::destroy(iAlloc, garbage);
                    std::advance(iGapStart, -garbageCount);
                }
                else if (after_gap(firstPos))
                {
                    for (auto src = std::prev(firstPos), dest = std::prev(lastPos); src >= iGapEnd; --src, --dest)
                        *dest = std::move(*src);
                    for (auto garbage = iGapEnd; garbage != iGapEnd + garbageCount; ++garbage)
                        std::allocator_traits<allocator_type>::destroy(iAlloc, garbage);
                    std::advance(iGapEnd, garbageCount);
                }
                else
                {
                    for (auto garbage = firstPos; garbage != lastPos; ++garbage)
                    {
                        if (garbage < iGapStart)
                            std::allocator_traits<allocator_type>::destroy(iAlloc, garbage);
                        else if (garbage >= iGapEnd)
                            std::allocator_traits<allocator_type>::destroy(iAlloc, garbage);
                    }
                    iGapStart = firstPos;
                    iGapEnd = lastPos;
                }
                return std::next(begin(), firstIndex);
            }
            else
            {
                unsplit();
                auto const newFirstPos = std::next(begin(), firstIndex).base();
                auto const garbageStart = std::prev(iDataEnd, garbageCount);
                for (auto src = std::next(newFirstPos, garbageCount), dest = newFirstPos; src != iDataEnd; ++src, ++dest)
                    *dest = std::move(*src);
                for (auto garbage = iDataEnd - garbageCount; garbage != iDataEnd; ++garbage)
                    std::allocator_traits<allocator_type>::destroy(iAlloc, garbage);
                std::advance(iDataEnd, -garbageCount);
                return iterator{ *this, newFirstPos };
            }
        }
        constexpr void pop_back()
        {
            (void)erase(std::prev(end()));
        }
        constexpr iterator insert(const_iterator pos, value_type const& value)
        {
            auto memory = allocate_from_gap(pos, 1);
            std::allocator_traits<allocator_type>::construct(iAlloc, memory, value);
            return iterator{ *this, memory };
        }
        constexpr iterator insert(const_iterator pos, value_type&& value)
        {
            auto memory = allocate_from_gap(pos, 1);
            std::allocator_traits<allocator_type>::construct(iAlloc, memory, std::move(value));
            return iterator{ *this, memory };
        }
        constexpr iterator insert(const_iterator pos, size_type count, value_type const& value)
        {
            auto memory = allocate_from_gap(pos, count);
            std::uninitialized_fill_n(memory, count, value);
            return iterator{ *this, memory };
        }
        template<class InputIt>
        constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
        {
            auto posIndex = std::distance(cbegin(), pos);
            while (first != last)
            {
                auto memory = allocate_from_gap(pos, 1);
                std::uninitialized_copy_n(first++, 1, memory);
            }
            return std::next(begin(), posIndex);
        }
        constexpr iterator insert(const_iterator pos, std::initializer_list<value_type> list)
        {
            auto memory = allocate_from_gap(pos, list.size());
            std::uninitialized_copy(list.begin(), list.end(), memory);
            return iterator{ *this, memory };
        }
        template<class... Args>
        constexpr iterator emplace(const_iterator pos, Args&&... args)
        {
            auto memory = allocate_from_gap(pos, 1);
            std::allocator_traits<allocator_type>::construct(iAlloc, memory, std::forward<Args>(args)...);
            return iterator{ *this, memory };
        }
        constexpr void push_back(value_type const& value)
        {
            (void)insert(end(), value);
        }
        constexpr void push_back(value_type&& value)
        {
            (void)insert(end(), std::move(value));
        }
        template<class... Args>
        constexpr void emplace_back(Args&&... args)
        {
            (void)emplace(end(), std::forward<Args>(args)...);
        }
        constexpr void resize(size_type count)
        {
            if (count < size())
                (void)erase(std::prev(end(), size() - count), end());
            else if (count > size())
                (void)insert(end(), count - size(), value_type{});
        }
        constexpr void resize(size_type count, value_type const& value)
        {
            if (count < size())
                (void)erase(std::prev(end(), size() - count), end());
            else if (count > size())
                (void)insert(end(), count - size(), value);
        }
    public:
        constexpr void unsplit() const
        {
            if (!gap_active())
                return;
            auto const next = std::uninitialized_move(iGapEnd, std::min(iGapEnd + gap_size(), iDataEnd), iGapStart);
            auto const newEnd = std::move(std::min(iGapEnd + gap_size(), iDataEnd), iDataEnd, next);
            for (auto garbage = newEnd; garbage != iDataEnd; ++garbage)
                std::allocator_traits<allocator_type>::destroy(iAlloc, garbage);
            iDataEnd = newEnd;
            iGapStart = nullptr;
            iGapEnd = nullptr;
        }
    private:
        constexpr size_type room() const noexcept
        {
            return capacity() - size();
        }
        constexpr size_type adjusted_index(size_type pos) const noexcept
        {
            if (!gap_active() || pos < (iGapStart - iData))
                return pos;
            return pos + gap_size();
        }
        constexpr bool gap_active() const noexcept
        {
            return iGapStart != iGapEnd;
        }
        constexpr size_type gap_size() const noexcept
        {
            return iGapEnd - iGapStart;
        }
        constexpr bool before_gap(const_pointer aPosition) const noexcept
        {
            return gap_active() && aPosition < iGapStart;
        }
        constexpr bool after_gap(const_pointer aPosition) const noexcept
        {
            return gap_active() && aPosition >= iGapEnd;
        }
        constexpr bool near_gap(const_pointer aPosition) const noexcept
        {
            if (!gap_active())
                return false;
            return std::abs(aPosition - iGapStart) <= DefaultGapSize * NearnessFactor || 
                std::abs(aPosition - iGapEnd) <= DefaultGapSize * NearnessFactor;
        }
        constexpr bool before_gap(const_iterator aPosition) const noexcept
        {
            return before_gap(aPosition.base());
        }
        constexpr bool after_gap(const_iterator aPosition) const noexcept
        {
            return after_gap(aPosition.base());
        }
        constexpr bool near_gap(const_iterator aPosition) const noexcept
        {
            return near_gap(aPosition.base());
        }
        constexpr pointer allocate_from_gap(const_iterator aPosition, size_type aCount)
        {
            if (near_gap(aPosition) && aCount <= gap_size())
            {
                auto const posIndex = std::distance(cbegin(), aPosition);
                auto const pos = std::next(begin(), posIndex).base();
                if (before_gap(aPosition))
                {
                    auto dest = std::next(iGapStart, aCount - 1);
                    auto src = iGapStart;
                    do
                    {
                        --src;
                        if (dest >= iGapStart)
                            std::allocator_traits<allocator_type>::construct(iAlloc, dest--, std::move(*src));
                        else
                            *dest-- = std::move(*src);
                        if (src < pos + aCount)
                            std::allocator_traits<allocator_type>::destroy(iAlloc, src);
                    } while (src != pos);
                    iGapStart += aCount;
                    return pos;
                }
                else if (after_gap(aPosition))
                {
                    if (pos != iGapEnd)
                    {
                        auto dest = std::prev(iGapEnd, aCount);
                        auto src = iGapEnd;
                        do
                        {
                            if (dest < iGapEnd)
                                std::allocator_traits<allocator_type>::construct(iAlloc, dest++, std::move(*src));
                            else
                                *dest++ = std::move(*src);
                            if (src >= pos - aCount)
                                std::allocator_traits<allocator_type>::destroy(iAlloc, src);
                            ++src;
                        } while (src != pos);
                    }
                    iGapEnd -= aCount;
                    return pos - aCount;
                }
                // pos should be iGapStart
                iGapStart += aCount;
                return pos;
            }
            else
            {
                // todo: consider optimising by removing the need to call unsplit
                auto const posIndex = std::distance(cbegin(), aPosition);
                unsplit();
                if (room() < aCount)
                    grow(aCount);
                auto const pos = std::next(iData, posIndex);
                auto const initialGapSize = std::min(room(), DefaultGapSize);
                auto src = iDataEnd;
                auto dest = std::next(iDataEnd, initialGapSize);
                while (src != pos)
                {
                    --src;
                    --dest;
                    if (dest >= iDataEnd)
                        std::allocator_traits<allocator_type>::construct(iAlloc, dest, std::move(*src));
                    else
                        *dest = std::move(*src);
                };
                auto const garbageCount = std::min<size_type>(aCount, iDataEnd - pos);
                for (auto garbage = std::next(pos, garbageCount); garbage != pos;)
                    std::allocator_traits<allocator_type>::destroy(iAlloc, --garbage);
                std::advance(iDataEnd, initialGapSize);

                iGapStart = std::next(pos, aCount);
                iGapEnd = std::next(pos, initialGapSize);
                return pos;
            }
        }
        constexpr void grow(size_type aCount)
        {
            double constexpr GrowthFactor = 1.5;
            reserve(static_cast<size_type>((capacity() + DefaultGapSize + aCount) * GrowthFactor));
        }
    private:
        mutable allocator_type iAlloc = {};
        mutable pointer iData = nullptr;
        mutable pointer iDataEnd = nullptr;
        mutable pointer iStorageEnd = nullptr;
        mutable pointer iGapStart = nullptr;
        mutable pointer iGapEnd = nullptr;
    };
}
