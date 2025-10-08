// i_jar.hpp
/*
 *  Copyright (c) 2020, 2024 Leigh Johnston.
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
#include <neolib/core/i_vector.hpp>
#include <neolib/core/i_reference_counted.hpp>

namespace neolib
{
    using cookie = std::uint32_t;
    using small_cookie = std::uint16_t;
    using large_cookie = std::uint64_t;

    template <class CookieType>
    struct cookie_type_helper { using type = CookieType; };
    template <class CookieType> requires EnumClass<CookieType>
    struct cookie_type_helper<CookieType> { using type = std::underlying_type_t<CookieType>; };
    template <class CookieType>
    using underlying_cookie_type_t = typename cookie_type_helper<CookieType>::type;

    template<class CookieType>
    constexpr CookieType invalid_cookie = static_cast<CookieType>(~underlying_cookie_type_t<CookieType>{});

    template <typename CookieType>
    class i_basic_cookie_consumer
    {
    public:
        using cookie_type = CookieType;
    public:
        struct invalid_release : std::logic_error { invalid_release() : std::logic_error("i_basic_cookie_consumer::invalid_release") {} };
    public:
        virtual ~i_basic_cookie_consumer() = default;
    public:
        virtual void add_ref(cookie_type aCookie, long aCount = 1) = 0;
        virtual void release(cookie_type aCookie, long aCount = 1) = 0;
        virtual long use_count(cookie_type aCookie) const = 0;
    };

    using i_cookie_consumer = i_basic_cookie_consumer<cookie>;
    using i_small_cookie_consumer = i_basic_cookie_consumer<small_cookie>;

    struct cookie_invalid : std::logic_error { cookie_invalid() : std::logic_error("neolib::cookie_invalid") {} };
    struct cookie_already_added : std::logic_error { cookie_already_added() : std::logic_error("neolib::cookie_already_added") {} };
    struct cookies_exhausted : std::logic_error { cookies_exhausted() : std::logic_error("neolib::cookies_exhausted") {} };
    struct no_pointer_value_type_cookie_lookup : std::logic_error { no_pointer_value_type_cookie_lookup() : std::logic_error("neolib::no_pointer_value_type_cookie_lookup") {} };

    template <typename T, typename Container = i_vector<T>, typename CookieType = cookie>
    class i_basic_jar : public i_reference_counted
    {
    public:
        using abstract_type = i_basic_jar;
        using cookie_type = CookieType;
    public:
        using value_type = T;
        using container_type = Container;
        using const_iterator = typename container_type::const_iterator;
        using iterator = typename container_type::iterator;
    public:
        virtual ~i_basic_jar() = default;
    public:
        virtual bool empty() const = 0;
        virtual std::size_t size() const = 0;
        virtual bool contains(cookie_type aCookie) const = 0;
        virtual const_iterator find(cookie_type aCookie) const = 0;
        virtual iterator find(cookie_type aCookie) = 0;
        virtual const value_type& operator[](cookie_type aCookie) const = 0;
        virtual value_type& operator[](cookie_type aCookie) = 0;
        virtual const value_type& at_index(std::size_t aIndex) const = 0;
        virtual value_type& at_index(std::size_t aIndex) = 0;
        virtual cookie_type insert(value_type const& aItem) = 0;
    public:
        virtual iterator erase(const_iterator aItem) = 0;
        virtual iterator add(cookie_type aCookie, value_type const& aItem) = 0;
        virtual iterator remove(value_type const& aItem) = 0;
        virtual iterator remove(cookie_type aCookie) = 0;
    public:
        virtual cookie_type item_cookie(value_type const& aItem) const = 0;
        virtual cookie_type next_cookie() = 0;
        virtual void return_cookie(cookie_type aCookie) = 0;
    public:
        virtual const_iterator cbegin() const = 0;
        virtual const_iterator begin() const = 0;
        virtual iterator begin() = 0;
        virtual const_iterator cend() const = 0;
        virtual const_iterator end() const = 0;
        virtual iterator end() = 0;
    public:
        virtual void clear() = 0;
        virtual const container_type& items() const = 0;
        virtual container_type& items() = 0;
    };

    template <typename T>
    using i_jar = i_basic_jar<T, i_vector<T>, cookie>;
    template <typename T>
    using i_small_jar = i_basic_jar<T, i_vector<T>, small_cookie>;
}
