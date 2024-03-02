// small_buffer_allocator.hpp
/*
 *  Copyright (c) 2024 Leigh Johnston.
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

namespace neolib
{
    template <typename T, std::size_t SmallBufferSize = 8u, std::size_t MaxSize = std::numeric_limits<std::size_t>::max() / sizeof(T), typename Alloc = std::allocator<T>>
    class basic_small_buffer_allocator;

    template <typename T, typename R>
    struct small_buffer_allocator_types
    {
        using controlled_value_type = T;
        using rebound_value_type = R;
    };

    template <typename T, std::size_t SmallBufferSize>
    struct small_buffer
    {
        using value_type = T;
        using buffer_storage_t = std::byte[sizeof(value_type) * SmallBufferSize];
        alignas(value_type) buffer_storage_t storage;
        bool allocated;
        small_buffer() : allocated{ false } {}
        small_buffer(const small_buffer&) : allocated{ false } {}
        small_buffer& operator=(const small_buffer&) { return *this; }
    };

    template <typename T, typename R, std::size_t SmallBufferSize, std::size_t MaxSize, typename Alloc>
    class basic_small_buffer_allocator<small_buffer_allocator_types<T, R>, SmallBufferSize, MaxSize, Alloc> : public std::allocator_traits<Alloc>::template rebind_alloc<R>
    {
        typedef basic_small_buffer_allocator<small_buffer_allocator_types<T, R>, SmallBufferSize, MaxSize, Alloc> self_type;
    public:
        struct no_small_buffer : std::logic_error { no_small_buffer() : std::logic_error("neolib::basic_small_buffer_allocator::no_small_buffer") {} };
    public:
        using types = small_buffer_allocator_types<T, R>;
        using propagate_on_container_move_assignment = std::false_type;
        using is_always_equal = std::false_type;
        template<class U> struct rebind { typedef basic_small_buffer_allocator<small_buffer_allocator_types<T, U>, SmallBufferSize, MaxSize, Alloc> other; };
    public:
        using controlled_value_type = T;
        using value_type = R;
        using default_allocator_type = std::allocator<value_type>;
        using size_type = typename default_allocator_type::size_type;
        using pointer = typename std::allocator_traits<default_allocator_type>::pointer;
        using small_buffer_type = small_buffer<controlled_value_type, SmallBufferSize>;
    public:
        basic_small_buffer_allocator() :
            default_allocator_type{},
            iBuffer{ nullptr }
        {
        }
        basic_small_buffer_allocator(small_buffer_type& aBuffer) :
            default_allocator_type{},
            iBuffer{ &aBuffer }
        {
        }
        basic_small_buffer_allocator(const basic_small_buffer_allocator& aOther) :
            default_allocator_type{ aOther },
            iBuffer{ aOther.iBuffer }
        {
        }
        basic_small_buffer_allocator(basic_small_buffer_allocator&& aOther) :
            default_allocator_type(std::move(aOther)),
            iBuffer{ nullptr }
        {
        }
        template <typename U>
        basic_small_buffer_allocator(const basic_small_buffer_allocator<U, SmallBufferSize, MaxSize, Alloc>& aOther) :
            default_allocator_type{ aOther },
            iBuffer{ nullptr }
        {
        }
        template <typename U>
        basic_small_buffer_allocator(const basic_small_buffer_allocator<U, SmallBufferSize, MaxSize, Alloc>&& aOther) :
            default_allocator_type(std::move(aOther)),
            iBuffer{ nullptr }
        {
        }
    public:
        basic_small_buffer_allocator& operator=(const basic_small_buffer_allocator& aOther)
        {
            iBuffer = aOther.iBuffer;
            return *this;
        }
        basic_small_buffer_allocator& operator=(basic_small_buffer_allocator&& aOther)
        {
            iBuffer = nullptr;
            return *this;
        }
        template <typename U>
        basic_small_buffer_allocator& operator=(const basic_small_buffer_allocator<U, SmallBufferSize, MaxSize, Alloc>& aOther)
        {
            iBuffer = nullptr;
            return *this;
        }
        template <typename U>
        basic_small_buffer_allocator& operator=(basic_small_buffer_allocator<U, SmallBufferSize, MaxSize, Alloc>&& aOther)
        {
            iBuffer = nullptr;
            return *this;
        }
    public:
        bool operator==(const basic_small_buffer_allocator& aOther) const
        {
            return false;
        }
        bool operator!=(const basic_small_buffer_allocator& aOther) const
        {
            return true;
        }
    public:
        size_type max_size() const
        {
            return MaxSize;
        }
        pointer allocate(std::size_t n)
        {
            return allocate(n, nullptr);
        }
        pointer allocate(std::size_t n, const void*)
        {
            if constexpr (std::is_same_v<value_type, controlled_value_type>)
            {
                if (n <= SmallBufferSize && is_buffer_available())
                {
                    buffer().allocated = true;
                    return reinterpret_cast<pointer>(&buffer().storage);
                }
                else
                    return default_allocator_type::allocate(n);
            }
            else
                return default_allocator_type::allocate(n);
        }
        void deallocate(pointer p, std::size_t n)
        {
            if constexpr (std::is_same_v<value_type, controlled_value_type>)
            {
                if (is_buffer_used() && p == reinterpret_cast<pointer>(&buffer().storage))
                    buffer().allocated = false;
                else
                    default_allocator_type::deallocate(p, n);
            }
            else
                default_allocator_type::deallocate(p, n);
        }
    public:
        bool has_buffer() const
        {
            return iBuffer != nullptr;
        }
        bool is_buffer_available() const
        {
            return has_buffer() && !buffer().allocated;
        }
        bool is_buffer_used() const
        {
            return has_buffer() && buffer().allocated;
        }
        const small_buffer_type& buffer() const
        {
            if (has_buffer())
                return *iBuffer;
            throw no_small_buffer();
        }
        small_buffer_type& buffer()
        {
            return const_cast<small_buffer_type&>(to_const(*this).buffer());
        }
    private:
        small_buffer_type* iBuffer;
    };

    template <typename T, typename U, std::size_t SmallBufferSize, std::size_t MaxSize, typename Alloc>
    inline bool operator==(const basic_small_buffer_allocator<T, SmallBufferSize, MaxSize, Alloc>&, const basic_small_buffer_allocator<U, SmallBufferSize, MaxSize, Alloc>&)
    {
        return false;
    }

    template <typename T, typename U, std::size_t SmallBufferSize, std::size_t MaxSize, typename Alloc>
    inline bool operator!=(const basic_small_buffer_allocator<T, SmallBufferSize, MaxSize, Alloc>&, const basic_small_buffer_allocator<U, SmallBufferSize, MaxSize, Alloc>&)
    {
        return true;
    }

    template <typename T, std::size_t SmallBufferSize = 8u, std::size_t MaxSize = std::numeric_limits<std::size_t>::max() / sizeof(T), typename Alloc = std::allocator<T>>
    using small_buffer_allocator = basic_small_buffer_allocator<small_buffer_allocator_types<T, T>, SmallBufferSize, MaxSize, Alloc>;

}
