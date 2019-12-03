// jar.hpp
/*
 *  Copyright (c) 2018 Leigh Johnston.
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
#include <set>
#include <mutex>
#include <atomic>
#include <boost/stacktrace.hpp>

namespace neolib
{
    typedef uint32_t cookie;
    typedef uint16_t small_cookie;

    template <typename CookieType = cookie>
    class i_basic_jar_item
    {
    public:
        typedef CookieType cookie_type;
    public:
        virtual ~i_basic_jar_item() {}
    public:
        virtual cookie_type cookie() const = 0;
    };

    template <typename CookieType>
    inline CookieType item_cookie(const i_basic_jar_item<CookieType>& aItem)
    {
        return aItem.cookie();
    }

    template <typename CookieType>
    inline CookieType item_cookie(const i_basic_jar_item<CookieType>* aItem)
    {
        return aItem->cookie();
    }

    template <typename T>
    inline auto item_cookie(const std::unique_ptr<T>& aPtr) -> decltype(item_cookie(*aPtr))
    {
        return item_cookie(*aPtr);
    }

    template <typename CookieType>
    class i_basic_cookie_consumer
    {
    public:
        typedef CookieType cookie_type;
    public:
        struct invalid_release : std::logic_error { invalid_release() : std::logic_error("neolib::i_basic_cookie_consumer::invalid_release") {} };
    public:
        virtual ~i_basic_cookie_consumer() {}
    public:
        virtual void add_ref(cookie_type aCookie) = 0;
        virtual void release(cookie_type aCookie) = 0;
        virtual long use_count(cookie_type aCookie) const = 0;
    };

    template <typename CookieType>
    class basic_cookie_ref_ptr
    {
    public:
        typedef CookieType cookie_type;
        static constexpr cookie_type no_cookie = cookie_type{};
    public:
        basic_cookie_ref_ptr() :
            iConsumer{ nullptr },
            iCookie{ no_cookie }
        {
        }
        basic_cookie_ref_ptr(i_basic_cookie_consumer<cookie_type>& aConsumer, cookie_type aCookie) :
            iConsumer{ &aConsumer },
            iCookie{ aCookie }
        {
            add_ref();
        }
        ~basic_cookie_ref_ptr()
        {
            release();
        }
        basic_cookie_ref_ptr(const basic_cookie_ref_ptr& aOther) :
            iConsumer{ aOther.iConsumer },
            iCookie{ aOther.iCookie }
        {
            add_ref();
        }
        basic_cookie_ref_ptr(basic_cookie_ref_ptr&& aOther) :
            iConsumer{ aOther.iConsumer },
            iCookie{ aOther.iCookie }
        {
            add_ref();
            aOther.release();
        }
    public:
        basic_cookie_ref_ptr& operator=(const basic_cookie_ref_ptr& aOther)
        {
            if (&aOther == this)
                return *this;
            basic_cookie_ref_ptr temp{ std::move(*this) };
            iConsumer = aOther.iConsumer;
            iCookie = aOther.iCookie;
            add_ref();
            return *this;
        }
        basic_cookie_ref_ptr& operator=(basic_cookie_ref_ptr&& aOther)
        {
            if (&aOther == this)
                return *this;
            basic_cookie_ref_ptr temp{ std::move(*this) };
            iConsumer = aOther.iConsumer;
            iCookie = aOther.iCookie;
            add_ref();
            aOther.release();
            return *this;
        }
    public:
        bool operator==(const basic_cookie_ref_ptr& aRhs) const
        {
            return iConsumer == aRhs.iConsumer && iCookie == aRhs.iCookie;
        }
        bool operator!=(const basic_cookie_ref_ptr& aRhs) const
        {
            return !(*this == aRhs);
        }
        bool operator<(const basic_cookie_ref_ptr& aRhs) const
        {
            return std::tie(iConsumer, iCookie) < std::tie(aRhs.iConsumer, aRhs.iCookie);
        }
    public:
        bool valid() const
        {
            return have_consumer() && have_cookie();
        }
        bool expired() const
        {
            return !valid();
        }
        cookie_type cookie() const
        {
            return iCookie;
        }
    private:
        void add_ref() const
        {
            if (!valid())
                return;
            consumer().add_ref(cookie());
        }
        void release() const
        {
            if (!valid())
                return;
            consumer().release(cookie());
            iConsumer = nullptr;
            iCookie = no_cookie;
        }
        bool have_consumer() const
        {
            return iConsumer != nullptr;
        }
        i_basic_cookie_consumer<cookie_type>& consumer() const
        {
            return *iConsumer;
        }
        bool have_cookie() const
        {
            return iCookie != no_cookie;
        }
    private:
        mutable i_basic_cookie_consumer<cookie_type>* iConsumer;
        mutable cookie_type iCookie;
    };

    template <typename T, typename CookieType = cookie, typename MutexType = std::recursive_mutex>
    class basic_jar
    {
    public:
        typedef CookieType cookie_type;
    public:
        typedef T value_type;
        typedef std::vector<value_type> container_type;
        typedef typename container_type::const_iterator const_iterator;
        typedef typename container_type::iterator iterator;
        typedef MutexType mutex_type;
    private:
        typedef typename container_type::size_type reverse_index_t;
        typedef std::vector<reverse_index_t> reverse_indices_t;
        typedef std::vector<cookie_type> free_cookies_t;
    private:
        static constexpr cookie_type INVALID_COOKIE = static_cast<cookie_type>(~cookie_type{});
        static constexpr reverse_index_t INVALID_REVERSE_INDEX = static_cast<reverse_index_t>(~reverse_index_t{});
    public:
        struct invalid_cookie : std::logic_error { invalid_cookie() : std::logic_error("neolib::basic_jar::invalid_cookie") {} };
        struct cookie_already_added : std::logic_error { cookie_already_added() : std::logic_error("neolib::basic_jar::cookie_already_added") {} };
        struct cookies_exhausted : std::logic_error { cookies_exhausted() : std::logic_error("neolib::basic_jar::cookies_exhausted") {} };
    public:
        basic_jar() : iNextAvailableCookie{}
        {
        }
    public:
        bool contains(cookie_type aCookie) const
        {
            std::lock_guard<mutex_type> lg{ mutex() };
            return aCookie < reverse_indices().size() && reverse_indices()[aCookie] != INVALID_REVERSE_INDEX;
        }
        const value_type& operator[](cookie_type aCookie) const
        {
            std::lock_guard<mutex_type> lg{ mutex() };
            if (aCookie >= reverse_indices().size())
                throw invalid_cookie();
            auto reverseIndex = reverse_indices()[aCookie];
            if (reverseIndex == INVALID_REVERSE_INDEX)
                throw invalid_cookie();
            return jar()[reverseIndex];
        }
        value_type& operator[](cookie_type aCookie)
        {
            return const_cast<value_type&>(to_const(*this)[aCookie]);
        }
        template <typename... Args>
        cookie_type emplace(Args&&... aArgs)
        {
            auto cookie = next_cookie();
            try
            {
                add(value_type{ cookie, std::forward<Args>(aArgs)... });
            }
            catch (...)
            {
                return_cookie(cookie);
                throw;
            }
            return cookie;
        }
        template <typename Arg>
        iterator add(Arg&& aItem)
        {
            std::lock_guard<mutex_type> lg{ mutex() };
            auto cookie = item_cookie(aItem);
            if (reverse_indices().size() <= cookie)
                reverse_indices().resize(cookie + 1, INVALID_REVERSE_INDEX);
            if (reverse_indices()[cookie] != INVALID_REVERSE_INDEX)
                throw cookie_already_added();
            auto result = jar().insert(jar().end(), std::forward<Arg>(aItem));
            reverse_indices()[cookie] = jar().size() - 1;
            return result;
        }
        iterator remove(const value_type& aItem)
        {
            std::lock_guard<mutex_type> lg{ mutex() };
            return remove(item_cookie(aItem));
        }
        iterator remove(cookie_type aCookie)
        {
            std::lock_guard<mutex_type> lg{ mutex() };
            auto& reverseIndex = reverse_indices()[aCookie];
            if (reverseIndex == INVALID_REVERSE_INDEX)
                throw invalid_cookie();
            if (reverseIndex < jar().size() - 1)
            {
                auto& item = jar()[reverseIndex];
                std::swap(item, jar().back());
                reverse_indices()[item_cookie(item)] = reverseIndex;
            }
            jar().pop_back();
            iterator result = std::next(jar().begin(), reverseIndex);
            reverseIndex = INVALID_REVERSE_INDEX;
            return_cookie(aCookie);
            return result;
        }
    public:
        cookie_type next_cookie()
        {
            std::lock_guard<mutex_type> lg{ mutex() };
            if (!free_cookies().empty())
            {
                auto nextCookie = free_cookies().back();
                free_cookies().pop_back();
                return nextCookie;
            }
            auto nextCookie = ++iNextAvailableCookie;
            if (nextCookie == INVALID_COOKIE)
                throw cookies_exhausted();
            return nextCookie;
        }
        void return_cookie(cookie_type aCookie)
        {
            std::lock_guard<mutex_type> lg{ mutex() };
            free_cookies().push_back(aCookie);
        }
    public:
        mutex_type& mutex() const
        {
            return iMutex;
        }
        const_iterator cbegin() const
        {
            return jar().begin();
        }
        const_iterator begin() const
        {
            return cbegin();
        }
        iterator begin()
        {
            return jar().begin();
        }
        const_iterator cend() const
        {
            return jar().end();
        }
        const_iterator end() const
        {
            return cend();
        }
        iterator end()
        {
            return jar().end();
        }
    public:
        void clear()
        {
            std::lock_guard<mutex_type> lg{ mutex() };
            iNextAvailableCookie = 0ul;
            free_cookies().clear();
            jar().clear();
            reverse_indices().clear();
        }
    private:
        const container_type& jar() const
        {
            return iJar;
        }
        container_type& jar()
        {
            return iJar;
        }
        const reverse_indices_t& reverse_indices() const
        {
            return iReverseIndices;
        }
        reverse_indices_t& reverse_indices()
        {
            return iReverseIndices;
        }
        const free_cookies_t& free_cookies() const
        {
            return iFreeCookies;
        }
        free_cookies_t& free_cookies()
        {
            return iFreeCookies;
        }
    private:
        mutable mutex_type iMutex;
        mutable std::atomic<cookie_type> iNextAvailableCookie;
        mutable free_cookies_t iFreeCookies;
        container_type iJar;
        reverse_indices_t iReverseIndices;
    };

    typedef i_basic_cookie_consumer<cookie> i_cookie_consumer;
    typedef i_basic_cookie_consumer<small_cookie> i_small_cookie_consumer;
    typedef i_basic_jar_item<cookie> i_jar_item;
    typedef i_basic_jar_item<small_cookie> i_small_jar_item;
    typedef basic_cookie_ref_ptr<cookie> cookie_ref_ptr;
    typedef basic_cookie_ref_ptr<small_cookie> small_cookie_ref_ptr;
    template <typename T, typename MutexType = std::recursive_mutex>
    using jar = basic_jar<T, cookie, MutexType>;
    template <typename T, typename MutexType = std::recursive_mutex>
    using small_jar = basic_jar<T, small_cookie, MutexType>;
}
