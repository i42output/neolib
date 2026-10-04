// secure_string.hpp
/*
 *  Copyright (c) 2026 Leigh Johnston.
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

#include <cstddef>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <new>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <string_view>

namespace neolib
{
    // Overwrites memory with multiple passes (0x00, 0xFF, random bytes; cf. DoD 5220.22-M) followed
    // by a final pass of zeros. All writes are volatile so that the compiler cannot elide any pass
    // as a dead store.
    inline void secure_erase(void* aData, std::size_t aSize) noexcept
    {
        if (aData == nullptr || aSize == 0u)
            return;
        auto const fill = [&](unsigned char aValue)
        {
            auto p = static_cast<volatile unsigned char*>(aData);
            for (std::size_t i = 0u; i < aSize; ++i)
                p[i] = aValue;
        };
        auto const randomize = [&]()
        {
            thread_local std::mt19937_64 tEngine{ []()
            {
                std::random_device rd;
                std::seed_seq seed{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
                return std::mt19937_64{ seed };
            }() };
            auto p = static_cast<volatile unsigned char*>(aData);
            std::uint64_t bits = 0u;
            for (std::size_t i = 0u; i < aSize; ++i)
            {
                if (i % sizeof(bits) == 0u)
                    bits = tEngine();
                p[i] = static_cast<unsigned char>(bits);
                bits >>= 8u;
            }
        };
        fill(0x00u);
        fill(0xFFu);
        randomize();
        fill(0x00u);
    }

    // Allocates whole pages of memory that are locked into physical memory so that they are never
    // written to the swap file/partition (VirtualLock on Windows; mlock, and MADV_DONTDUMP to exclude
    // them from core dumps, on POSIX). Each allocation has its own pages so that unlocking one
    // allocation can never unlock memory belonging to another. Throws std::bad_alloc if memory
    // cannot be allocated and std::system_error if it cannot be locked.
    NEOLIB_EXPORT void* secure_allocate(std::size_t aSize);
    // Securely erases (secure_erase), unlocks and frees memory allocated by secure_allocate.
    NEOLIB_EXPORT void secure_deallocate(void* aMemory, std::size_t aSize) noexcept;

    template <typename T>
    struct secure_allocator
    {
        typedef T value_type;
        secure_allocator() noexcept = default;
        template <typename U>
        secure_allocator(secure_allocator<U> const&) noexcept {}
        T* allocate(std::size_t aCount)
        {
            if (aCount > std::numeric_limits<std::size_t>::max() / sizeof(T))
                throw std::bad_array_new_length{};
            return static_cast<T*>(secure_allocate(aCount * sizeof(T)));
        }
        void deallocate(T* aPointer, std::size_t aCount) noexcept
        {
            secure_deallocate(aPointer, aCount * sizeof(T));
        }
        friend bool operator==(secure_allocator const&, secure_allocator const&) noexcept
        {
            return true;
        }
    };

    // A string for secrets (passwords, passphrases etc.):
    // * characters are only ever held in heap memory allocated by secure_allocator (capacity is
    //   always reserved beyond the small string buffer before any characters are written, so they
    //   never live inside the string object itself and are never stranded there by growth or a move);
    // * any characters removed (clear, erase, resize, pop_back, assignment) are securely erased;
    // * the destructor securely erases the whole capacity;
    // * std::basic_string is a private base so a secure string cannot be sliced or implicitly
    //   converted to (and so copied as) an ordinary string.
    template <typename CharT, typename Traits = std::char_traits<CharT>>
    class basic_secure_string : private std::basic_string<CharT, Traits, secure_allocator<CharT>>
    {
        typedef std::basic_string<CharT, Traits, secure_allocator<CharT>> base_type;
    public:
        using typename base_type::traits_type;
        using typename base_type::value_type;
        using typename base_type::allocator_type;
        using typename base_type::size_type;
        using typename base_type::difference_type;
        using typename base_type::reference;
        using typename base_type::const_reference;
        using typename base_type::pointer;
        using typename base_type::const_pointer;
        using typename base_type::iterator;
        using typename base_type::const_iterator;
        using typename base_type::reverse_iterator;
        using typename base_type::const_reverse_iterator;
        typedef std::basic_string_view<CharT, Traits> view_type;
        using base_type::npos;
        static constexpr size_type MinimumCapacity = 256u;
    public:
        basic_secure_string()
        {
            base_type::reserve(MinimumCapacity);
        }
        basic_secure_string(CharT const* aString) :
            basic_secure_string{}
        {
            append(aString);
        }
        basic_secure_string(CharT const* aString, size_type aLength) :
            basic_secure_string{}
        {
            append(aString, aLength);
        }
        basic_secure_string(size_type aCount, CharT aCharacter) :
            basic_secure_string{}
        {
            append(aCount, aCharacter);
        }
        explicit basic_secure_string(view_type aString) :
            basic_secure_string{}
        {
            append(aString);
        }
        basic_secure_string(basic_secure_string const& aOther) :
            basic_secure_string{}
        {
            append(aOther);
        }
        basic_secure_string(basic_secure_string&& aOther) :
            base_type{ std::move(static_cast<base_type&>(aOther)) }
        {
            // the moved-from string is left using its small string buffer; move it back to the heap
            aOther.base_type::reserve(MinimumCapacity);
        }
        ~basic_secure_string()
        {
            erase_capacity();
        }
    public:
        basic_secure_string& operator=(basic_secure_string const& aOther)
        {
            if (this != &aOther)
                assign(aOther.view());
            return *this;
        }
        basic_secure_string& operator=(basic_secure_string&& aOther)
        {
            if (this != &aOther)
            {
                clear();
                base_type::operator=(std::move(static_cast<base_type&>(aOther))); // our old block is erased by secure_allocator
                aOther.base_type::reserve(MinimumCapacity);
            }
            return *this;
        }
        basic_secure_string& operator=(CharT const* aString)
        {
            return assign(view_type{ aString });
        }
        basic_secure_string& operator=(view_type aString)
        {
            return assign(aString);
        }
    public:
        using base_type::size;
        using base_type::length;
        using base_type::capacity;
        using base_type::max_size;
        using base_type::empty;
        using base_type::data;
        using base_type::c_str;
        using base_type::operator[];
        using base_type::at;
        using base_type::front;
        using base_type::back;
        using base_type::begin;
        using base_type::end;
        using base_type::cbegin;
        using base_type::cend;
        using base_type::rbegin;
        using base_type::rend;
        using base_type::crbegin;
        using base_type::crend;
        view_type view() const noexcept
        {
            return view_type{ base_type::data(), base_type::size() };
        }
        void reserve(size_type aCapacity)
        {
            base_type::reserve(std::max(aCapacity, MinimumCapacity));
        }
    public:
        basic_secure_string& assign(view_type aString)
        {
            clear();
            return append(aString);
        }
        basic_secure_string& append(view_type aString)
        {
            base_type::append(aString.data(), aString.size());
            return *this;
        }
        basic_secure_string& append(CharT const* aString)
        {
            return append(view_type{ aString });
        }
        basic_secure_string& append(CharT const* aString, size_type aLength)
        {
            return append(view_type{ aString, aLength });
        }
        basic_secure_string& append(basic_secure_string const& aString)
        {
            return append(aString.view());
        }
        basic_secure_string& append(size_type aCount, CharT aCharacter)
        {
            base_type::append(aCount, aCharacter);
            return *this;
        }
        basic_secure_string& operator+=(view_type aString)
        {
            return append(aString);
        }
        basic_secure_string& operator+=(CharT const* aString)
        {
            return append(aString);
        }
        basic_secure_string& operator+=(basic_secure_string const& aString)
        {
            return append(aString);
        }
        basic_secure_string& operator+=(CharT aCharacter)
        {
            push_back(aCharacter);
            return *this;
        }
        void push_back(CharT aCharacter)
        {
            base_type::push_back(aCharacter);
        }
        void pop_back()
        {
            erase(size() - 1u, 1u);
        }
        basic_secure_string& erase(size_type aPosition = 0u, size_type aCount = npos)
        {
            auto const oldSize = size();
            base_type::erase(aPosition, aCount);
            auto const newSize = size();
            // characters shifted down by erase are left beyond the new end; erase them
            base_type::resize(oldSize);
            secure_erase(base_type::data() + newSize, (oldSize - newSize) * sizeof(CharT));
            base_type::resize(newSize);
            return *this;
        }
        void resize(size_type aCount, CharT aCharacter = CharT{})
        {
            if (aCount < size())
                erase(aCount);
            else
                base_type::resize(aCount, aCharacter);
        }
        void clear() noexcept
        {
            erase_capacity();
            base_type::clear();
        }
        void swap(basic_secure_string& aOther) noexcept
        {
            base_type::swap(static_cast<base_type&>(aOther));
        }
        friend void swap(basic_secure_string& aLhs, basic_secure_string& aRhs) noexcept
        {
            aLhs.swap(aRhs);
        }
    public:
        friend bool operator==(basic_secure_string const& aLhs, basic_secure_string const& aRhs) noexcept
        {
            return aLhs.view() == aRhs.view();
        }
        friend bool operator==(basic_secure_string const& aLhs, view_type aRhs) noexcept
        {
            return aLhs.view() == aRhs;
        }
        friend bool operator==(basic_secure_string const& aLhs, CharT const* aRhs)
        {
            return aLhs.view() == view_type{ aRhs };
        }
    private:
        void erase_capacity() noexcept
        {
            // capacity is always beyond the small string buffer so resize cannot allocate
            base_type::resize(base_type::capacity());
            secure_erase(base_type::data(), base_type::size() * sizeof(CharT));
        }
    };

    using secure_string = basic_secure_string<char>;
}
