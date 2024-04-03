// vecarray.hpp
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

namespace neolib
{
    template<typename T, std::size_t ArraySize, std::size_t MaxVectorSize = ArraySize, typename Alloc = std::allocator<T>>
    class vecarray : private small_buffer<T, ArraySize>, public std::vector<T, small_buffer_allocator<T, ArraySize, MaxVectorSize, Alloc>>
    {
        using self_type = vecarray<T, ArraySize, MaxVectorSize>;
        using base_type = std::vector<T, small_buffer_allocator<T, ArraySize, MaxVectorSize, Alloc>>;
        // types
    public:
        using value_type = T;
        using std_type = base_type;
        using allocator_type = typename std_type::allocator_type;
        using size_type = typename std_type::size_type;
        using const_iterator = typename std_type::const_iterator;
        using iterator = typename std_type::iterator;
        // construction
    public:
        constexpr vecarray() :
            std_type{ allocator_type{ *this } }
        {
            if (std_type::capacity() == 0)
                std_type::reserve(ArraySize);
        }
        constexpr vecarray(vecarray const& aOther) :
            std_type{ aOther, allocator_type{ *this } }
        {
            if (std_type::capacity() == 0)
                std_type::reserve(ArraySize);
        }
        constexpr vecarray(vecarray&& aOther) :
            std_type{ aOther, allocator_type{ *this } }
        {
            if (std_type::capacity() == 0)
                std_type::reserve(ArraySize);
        }
        constexpr vecarray(std_type const& aOtherContainer) :
            std_type{ aOtherContainer, allocator_type{ *this } }
        {
            if (std_type::capacity() == 0)
                std_type::reserve(ArraySize);
        }
        constexpr vecarray(size_type count, const T& value) : 
            std_type{ count, value, allocator_type{ *this } }
        {
            if (std_type::capacity() == 0)
                std_type::reserve(ArraySize);
        }
        constexpr explicit vecarray(size_type count) :
            std_type{ count, allocator_type{ *this } }
        {
            if (std_type::capacity() == 0)
                std_type::reserve(ArraySize);
        }
        constexpr vecarray(std::initializer_list<value_type> aValues) :
            std_type{ aValues, allocator_type{ *this } }
        {
            if (std_type::capacity() == 0)
                std_type::reserve(ArraySize);
        }
        template <typename InputIter>
        constexpr vecarray(InputIter aFirst, InputIter aLast) :
            std_type{ aFirst, aLast, allocator_type{ *this } }
        {
            if (std_type::capacity() == 0)
                std_type::reserve(ArraySize);
        }
    public:
        constexpr vecarray& operator=(const vecarray& other)
        {
            std_type::operator=(other);
            return *this;
        }
        constexpr vecarray& operator=(vecarray&& other) noexcept
        {
            std_type::operator=(other);
            return *this;
        }
        constexpr vecarray& operator=(std::initializer_list<T> ilist)
        {
            std_type::operator=(ilist);
            return *this;
        }
        // operations
    public:
        const std_type& as_std_vector() const
        {
            return *this;
        }
        std_type& as_std_vector()
        {
            return *this;
        }
        std::vector<T, Alloc> to_std_vector() const
        {
            return std::vector<T, Alloc>{ std_type::begin(), std_type::end() };
        }
        size_type available() const noexcept
        {
            return std_type::max_size() - std_type::size();
        }
    };
}
