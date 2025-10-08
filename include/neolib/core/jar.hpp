// jar.hpp
/*
 *  Copyright (c) 2018, 2020 Leigh Johnston.
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
#include <optional>
#include <vector>
#include <set>
#include <mutex>
#include <atomic>
#include <neolib/core/vector.hpp>
#include <neolib/core/mutex.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/i_jar.hpp>

namespace neolib
{
    template <typename Consumer, typename Cookie>
    concept CookieConsumer = requires(Consumer a, Cookie c)
    {  
        a.add_ref(c);
        a.release(c);
        a.add_ref(c, 2);
        a.release(c, 2);
    };

    template <typename CookieType, CookieConsumer<CookieType> ConsumerType = i_basic_cookie_consumer<CookieType>>
    class basic_cookie_ref_ptr
    {
    public:
        using cookie_type = CookieType;
        using consumer_type = ConsumerType;
        static constexpr cookie_type no_cookie = cookie_type{};
    public:
        basic_cookie_ref_ptr() :
            iConsumer{ nullptr },
            iCookie{ no_cookie }
        {
        }
        basic_cookie_ref_ptr(consumer_type& aConsumer, cookie_type aCookie) :
            iConsumer{ &aConsumer },
            iCookie{ aCookie }
        {
            add_ref();
        }
        ~basic_cookie_ref_ptr()
        {
            release();
        }
        basic_cookie_ref_ptr(basic_cookie_ref_ptr const& aOther) :
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
        basic_cookie_ref_ptr& operator=(basic_cookie_ref_ptr const& aOther)
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
        bool operator==(basic_cookie_ref_ptr const& aRhs) const
        {
            return iConsumer == aRhs.iConsumer && iCookie == aRhs.iCookie;
        }
        bool operator!=(basic_cookie_ref_ptr const& aRhs) const
        {
            return !(*this == aRhs);
        }
        bool operator<(basic_cookie_ref_ptr const& aRhs) const
        {
            return std::forward_as_tuple(iConsumer, iCookie) < std::forward_as_tuple(aRhs.iConsumer, aRhs.iCookie);
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
        void reset() const
        {
            iConsumer = nullptr;
            iCookie = no_cookie;
        }
    private:
        void add_ref(long count = 1) const
        {
            if (!valid())
                return;
            consumer().add_ref(cookie(), count);
        }
        void release(long count = 1) const
        {
            if (!valid())
                return;
            consumer().release(cookie(), count);
            reset();
        }
        bool have_consumer() const
        {
            return iConsumer != nullptr;
        }
        consumer_type& consumer() const
        {
            return *iConsumer;
        }
        bool have_cookie() const
        {
            return iCookie != no_cookie;
        }
    private:
        mutable consumer_type* iConsumer;
        mutable cookie_type iCookie;
    };

    namespace detail
    {
        template<typename T> struct is_smart_ptr : std::false_type {};
        template<typename T> struct is_smart_ptr<std::shared_ptr<T>> : std::true_type {};
        template<typename T> struct is_smart_ptr<std::unique_ptr<T>> : std::true_type {};
        template<typename T> struct is_smart_ptr<ref_ptr<T>> : std::true_type {};
        template<typename T>
        inline constexpr bool is_smart_ptr_v = is_smart_ptr<T>::value;
    }

    template <typename T, typename Container = vector<T>, typename CookieType = cookie, typename MutexType = null_mutex>
    class basic_jar : public reference_counted<i_basic_jar<abstract_t<T>, abstract_t<Container>, CookieType>>
    {
    public:
        using cookie_type = CookieType;
        using underlying_cookie_type = underlying_cookie_type_t<cookie_type>;
    public:
        using value_type = T;
        using container_type = Container;
        using const_iterator = typename container_type::const_iterator;
        using iterator = typename container_type::iterator;
        using mutex_type = MutexType;
    private:
        using reverse_index_t = typename container_type::size_type;
        using reverse_indices_t = std::vector<reverse_index_t>;
        using cookies_t = std::vector<cookie_type>;
    private:
        static constexpr cookie_type INVALID_COOKIE = invalid_cookie<cookie_type>;
        static constexpr reverse_index_t INVALID_REVERSE_INDEX = static_cast<reverse_index_t>(~reverse_index_t{});
    public:
        basic_jar() : iNextAvailableCookie{}
        {
        }
        basic_jar(basic_jar const& aOther) :
            iNextAvailableCookie{ aOther.iNextAvailableCookie.load() },
            iAllocatedCookies{ aOther.iAllocatedCookies },
            iItems{ aOther.iItems },
            iFreeCookies{ aOther.iFreeCookies },
            iReverseIndices{ aOther.iReverseIndices }
        {
        }
        basic_jar(basic_jar&& aOther) :
            iNextAvailableCookie{ aOther.iNextAvailableCookie.load() },
            iAllocatedCookies{ std::move(aOther.iAllocatedCookies) },
            iItems{ std::move(aOther.iItems) },
            iFreeCookies{ std::move(aOther.iFreeCookies) },
            iReverseIndices{ std::move(aOther.iReverseIndices) }
        {
            aOther.iNextAvailableCookie.store({});
        }
    public:
        basic_jar& operator=(basic_jar const& aOther)
        {
            iNextAvailableCookie = aOther.iNextAvailableCookie.load();
            iAllocatedCookies = aOther.iAllocatedCookies;
            iItems = aOther.iItems;
            iFreeCookies = aOther.iFreeCookies;
            iReverseIndices = aOther.iReverseIndices;

            return *this;
        }
        basic_jar& operator=(basic_jar&& aOther)
        {
            iNextAvailableCookie = aOther.iNextAvailableCookie.load();
            iAllocatedCookies = std::move(aOther.iAllocatedCookies);
            iItems = std::move(aOther.iItems);
            iFreeCookies = std::move(aOther.iFreeCookies);
            iReverseIndices = std::move(aOther.iReverseIndices);

            aOther.iNextAvailableCookie.store({});

            return *this;
        }
    public:
        bool empty() const final
        {
            return items().empty();
        }
        std::size_t size() const final
        {
            return items().size();
        }
        bool contains(cookie_type aCookie) const final
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            return static_cast<underlying_cookie_type>(aCookie) < reverse_indices().size() && 
                reverse_indices()[static_cast<underlying_cookie_type>(aCookie)] != INVALID_REVERSE_INDEX;
        }
        const_iterator find(cookie_type aCookie) const final
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (contains(aCookie))
                return std::next(begin(), reverse_indices()[static_cast<underlying_cookie_type>(aCookie)]);
            return end();
        }
        iterator find(cookie_type aCookie) final
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (contains(aCookie))
                return std::next(begin(), reverse_indices()[static_cast<underlying_cookie_type>(aCookie)]);
            return end();
        }
        const value_type& operator[](cookie_type aCookie) const final
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (static_cast<underlying_cookie_type>(aCookie) >= reverse_indices().size())
                throw cookie_invalid();
            auto reverseIndex = reverse_indices()[static_cast<underlying_cookie_type>(aCookie)];
            if (reverseIndex == INVALID_REVERSE_INDEX)
                throw cookie_invalid();
            return items()[reverseIndex];
        }
        value_type& operator[](cookie_type aCookie) final
        {
            return const_cast<value_type&>(to_const(*this)[aCookie]);
        }
        const value_type& at_index(std::size_t aIndex) const final
        {
            return items().at(aIndex);
        }
        value_type& at_index(std::size_t aIndex) final
        {
            return items().at(aIndex);
        }
        cookie_type insert(abstract_t<value_type> const& aItem) final
        {
            auto cookie = next_cookie();
            try
            {
                add(cookie, aItem);
            }
            catch (...)
            {
                return_cookie(cookie);
                throw;
            }
            return cookie;
        }
        template <typename... Args>
        cookie_type emplace(Args&&... aArgs)
        {
            auto cookie = next_cookie();
            try
            {
                add(cookie, std::forward<Args>(aArgs)...);
            }
            catch (...)
            {
                return_cookie(cookie);
                throw;
            }
            return cookie;
        }
        iterator add(cookie_type aCookie, abstract_t<value_type> const& aItem) final
        {
            return add<const abstract_t<value_type>&>(aCookie, aItem);
        }
        iterator erase(const_iterator aItem) final
        {
            return remove(*aItem);
        }
        template <typename... Args>
        iterator add(cookie_type aCookie, Args&&... aArgs)
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            assert(std::find(free_cookies().begin(), free_cookies().end(), aCookie) == free_cookies().end());
            if (reverse_indices().size() <= static_cast<underlying_cookie_type>(aCookie))
                reverse_indices().insert(reverse_indices().end(), 
                    (static_cast<underlying_cookie_type>(aCookie) + underlying_cookie_type{ 1 }) - reverse_indices().size(), INVALID_REVERSE_INDEX);
            if (reverse_indices()[static_cast<underlying_cookie_type>(aCookie)] != INVALID_REVERSE_INDEX)
                throw cookie_already_added();
            std::optional<iterator> result;
            if constexpr (!detail::is_smart_ptr_v<value_type>)
                result = items().emplace(items().end(), std::forward<Args>(aArgs)...);
            else if constexpr (detail::is_smart_ptr_v<value_type> && std::is_abstract_v<typename value_type::element_type>)
                result = items().emplace(items().end(), std::forward<Args>(aArgs)...);
            else if constexpr (detail::is_smart_ptr_v<value_type> && !std::is_abstract_v<typename value_type::element_type>)
                result = items().insert(items().end(), value_type{ new typename value_type::element_type{std::forward<Args>(aArgs)...} });
            try
            {
                allocated_cookies().insert(allocated_cookies().end(), aCookie);
            }
            catch (...)
            {
                items().pop_back();
                throw;
            }
            reverse_indices()[static_cast<underlying_cookie_type>(aCookie)] = items().size() - 1;
            return *result;
        }
        iterator remove(abstract_t<value_type> const& aItem) final
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            return remove(item_cookie(aItem));
        }
        iterator remove(cookie_type aCookie) final
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            assert(std::find(free_cookies().begin(), free_cookies().end(), aCookie) == free_cookies().end());
            if (static_cast<underlying_cookie_type>(aCookie) >= reverse_indices().size())
                throw cookie_invalid();
            auto& reverseIndex = reverse_indices()[static_cast<underlying_cookie_type>(aCookie)];
            if (reverseIndex == INVALID_REVERSE_INDEX)
                throw cookie_invalid();
            if (reverseIndex < items().size() - 1)
            {
                auto& item = items()[reverseIndex];
                std::swap(item, items().back());
                auto& cookie = allocated_cookies()[reverseIndex];
                std::swap(cookie, allocated_cookies().back());
                reverse_indices()[static_cast<underlying_cookie_type>(cookie)] = reverseIndex;
            }
            auto resultIndex = reverseIndex;
            reverseIndex = INVALID_REVERSE_INDEX;
            allocated_cookies().pop_back();
            return_cookie(aCookie);
            items().pop_back();
            return resultIndex < items().size() ? std::next(items().begin(), resultIndex) : items().end();
        }
    public:
        cookie_type item_cookie(abstract_t<value_type> const& aItem) const final
        {
            if constexpr (!std::is_pointer_v<value_type>)
                return allocated_cookies()[&static_cast<value_type const&>(aItem) - &items()[0]];
            else
                throw no_pointer_value_type_cookie_lookup();
        }
        cookie_type next_cookie() final
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (!free_cookies().empty())
            {
                auto const nextCookie = free_cookies().back();
                free_cookies().pop_back();
                return nextCookie;
            }
            auto const nextCookie = static_cast<cookie_type>(static_cast<underlying_cookie_type>(iNextAvailableCookie.load()) + underlying_cookie_type{ 1 });
            iNextAvailableCookie = nextCookie;
            if (nextCookie == INVALID_COOKIE)
                throw cookies_exhausted();
            assert(std::find(free_cookies().begin(), free_cookies().end(), nextCookie) == free_cookies().end());
            return nextCookie;
        }
        void return_cookie(cookie_type aCookie) final
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            assert(std::find(free_cookies().begin(), free_cookies().end(), aCookie) == free_cookies().end());
            free_cookies().push_back(aCookie);
        }
    public:
        mutex_type& mutex() const
        {
            return iMutex;
        }
        const_iterator cbegin() const final
        {
            return items().begin();
        }
        const_iterator begin() const final
        {
            return cbegin();
        }
        iterator begin() final
        {
            return items().begin();
        }
        const_iterator cend() const final
        {
            return items().end();
        }
        const_iterator end() const final
        {
            return cend();
        }
        iterator end() final
        {
            return items().end();
        }
    public:
        void clear() final
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            iNextAvailableCookie = cookie_type{};
            allocated_cookies().clear();
            free_cookies().clear();
            items().clear();
            reverse_indices().clear();
        }
        const container_type& items() const final
        {
            return iItems;
        }
        container_type& items() final
        {
            return iItems;
        }
    private:
        const cookies_t& allocated_cookies() const
        {
            return iAllocatedCookies;
        }
        cookies_t& allocated_cookies()
        {
            return iAllocatedCookies;
        }
        const cookies_t& free_cookies() const
        {
            return iFreeCookies;
        }
        cookies_t& free_cookies()
        {
            return iFreeCookies;
        }
        const reverse_indices_t& reverse_indices() const
        {
            return iReverseIndices;
        }
        reverse_indices_t& reverse_indices()
        {
            return iReverseIndices;
        }
    private:
        mutable mutex_type iMutex;
        mutable std::atomic<cookie_type> iNextAvailableCookie;
        cookies_t iAllocatedCookies;
        container_type iItems;
        mutable cookies_t iFreeCookies;
        reverse_indices_t iReverseIndices;
    };

    template <typename T, typename CookieType = cookie, typename MutexType = null_mutex>
    class basic_std_vector_jar
    {
    public:
        using cookie_type = CookieType;
        using underlying_cookie_type = underlying_cookie_type_t<cookie_type>;
    public:
        using value_type = T;
        using container_type = std::vector<value_type>;
        using const_iterator = typename container_type::const_iterator;
        using iterator = typename container_type::iterator;
        using mutex_type = MutexType;
    private:
        using reverse_index_t = typename container_type::size_type;
        using reverse_indices_t = std::vector<reverse_index_t>;
        using cookies_t = std::vector<cookie_type>;
    private:
        static constexpr cookie_type INVALID_COOKIE = invalid_cookie<cookie_type>;
        static constexpr reverse_index_t INVALID_REVERSE_INDEX = static_cast<reverse_index_t>(~reverse_index_t{});
    public:
        basic_std_vector_jar() : iNextAvailableCookie{}
        {
        }
        basic_std_vector_jar(basic_std_vector_jar const& aOther) :
            iNextAvailableCookie{ aOther.iNextAvailableCookie.load() },
            iAllocatedCookies{ aOther.iAllocatedCookies },
            iItems{ aOther.iItems },
            iFreeCookies { aOther.iFreeCookies },
            iReverseIndices{ aOther.iReverseIndices }
        {
        }
        basic_std_vector_jar(basic_std_vector_jar&& aOther) :
            iNextAvailableCookie{ aOther.iNextAvailableCookie.load() },
            iAllocatedCookies{ std::move(aOther.iAllocatedCookies) },
            iItems{ std::move(aOther.iItems) },
            iFreeCookies{ std::move(aOther.iFreeCookies) },
            iReverseIndices{ std::move(aOther.iReverseIndices) }
        {
            aOther.iNextAvailableCookie.store({});
        }
    public:
        basic_std_vector_jar& operator=(basic_std_vector_jar const& aOther)
        {
            iNextAvailableCookie = aOther.iNextAvailableCookie.load();
            iAllocatedCookies = aOther.iAllocatedCookies;
            iItems = aOther.iItems;
            iFreeCookies = aOther.iFreeCookies;
            iReverseIndices = aOther.iReverseIndices;
            
            return *this;
        }
        basic_std_vector_jar& operator=(basic_std_vector_jar&& aOther)
        {
            iNextAvailableCookie = aOther.iNextAvailableCookie.load();
            iAllocatedCookies = std::move(aOther.iAllocatedCookies);
            iItems = std::move(aOther.iItems);
            iFreeCookies = std::move(aOther.iFreeCookies);
            iReverseIndices = std::move(aOther.iReverseIndices);

            aOther.iNextAvailableCookie.store({});

            return *this;
        }
    public:
        bool empty() const
        {
            return items().empty();
        }
        std::size_t size() const
        {
            return items().size();
        }
        bool contains(cookie_type aCookie) const
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            return static_cast<underlying_cookie_type>(aCookie) < reverse_indices().size() && 
                reverse_indices()[static_cast<underlying_cookie_type>(aCookie)] != INVALID_REVERSE_INDEX;
        }
        const_iterator find(cookie_type aCookie) const
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (contains(aCookie))
                return std::next(begin(), reverse_indices()[static_cast<underlying_cookie_type>(aCookie)]);
            return end();
        }
        iterator find(cookie_type aCookie)
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (contains(aCookie))
                return std::next(begin(), reverse_indices()[static_cast<underlying_cookie_type>(aCookie)]);
            return end();
        }
        const value_type& operator[](cookie_type aCookie) const
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (static_cast<underlying_cookie_type>(aCookie) >= reverse_indices().size())
                throw cookie_invalid();
            auto reverseIndex = reverse_indices()[static_cast<underlying_cookie_type>(aCookie)];
            if (reverseIndex == INVALID_REVERSE_INDEX)
                throw cookie_invalid();
            return items()[reverseIndex];
        }
        value_type& operator[](cookie_type aCookie)
        {
            return const_cast<value_type&>(to_const(*this)[aCookie]);
        }
        const value_type& at_index(std::size_t aIndex) const
        {
            return items().at(aIndex);
        }
        value_type& at_index(std::size_t aIndex)
        {
            return items().at(aIndex);
        }
        cookie_type insert(value_type const& aItem)
        {
            auto cookie = next_cookie();
            try
            {
                add(cookie, aItem);
            }
            catch (...)
            {
                return_cookie(cookie);
                throw;
            }
            return cookie;
        }
        template <typename... Args>
        cookie_type emplace(Args&&... aArgs)
        {
            auto cookie = next_cookie();
            try
            {
                add(cookie, std::forward<Args>(aArgs)...);
            }
            catch (...)
            {
                return_cookie(cookie);
                throw;
            }
            return cookie;
        }
        iterator add(cookie_type aCookie, value_type const& aItem)
        {
            return add<const value_type&>(aCookie, aItem);
        }
        iterator erase(const_iterator aItem)
        {
            return remove(*aItem);
        }
        template <typename... Args>
        iterator add(cookie_type aCookie, Args&&... aArgs)
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            assert(std::find(free_cookies().begin(), free_cookies().end(), aCookie) == free_cookies().end());
            if (reverse_indices().size() <= static_cast<underlying_cookie_type>(aCookie))
                reverse_indices().insert(reverse_indices().end(), 
                    (static_cast<underlying_cookie_type>(aCookie) + underlying_cookie_type{ 1 }) - reverse_indices().size(), INVALID_REVERSE_INDEX);
            if (reverse_indices()[static_cast<underlying_cookie_type>(aCookie)] != INVALID_REVERSE_INDEX)
                throw cookie_already_added();
            std::optional<iterator> result;
            if constexpr (!detail::is_smart_ptr_v<value_type> || (sizeof...(Args) == 1 && detail::is_smart_ptr_v<std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>>))
                result = items().emplace(items().end(), std::forward<Args>(aArgs)...);
            else if constexpr (detail::is_smart_ptr_v<value_type> && std::is_abstract_v<typename value_type::element_type>)
                result = items().emplace(items().end(), std::forward<Args>(aArgs)...);
            else if constexpr (detail::is_smart_ptr_v<value_type> && !std::is_abstract_v<typename value_type::element_type>)
                result = items().insert(items().end(), value_type{ new typename value_type::element_type{std::forward<Args>(aArgs)...} });
            try
            {
                allocated_cookies().insert(allocated_cookies().end(), aCookie);
            }
            catch (...)
            {
                items().pop_back();
                throw;
            }
            reverse_indices()[static_cast<underlying_cookie_type>(aCookie)] = items().size() - 1;
            return *result;
        }
        iterator remove(value_type const& aItem)
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            return remove(item_cookie(aItem));
        }
        iterator remove(cookie_type aCookie)
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            assert(std::find(free_cookies().begin(), free_cookies().end(), aCookie) == free_cookies().end());
            if (static_cast<underlying_cookie_type>(aCookie) >= reverse_indices().size())
                throw cookie_invalid();
            auto& reverseIndex = reverse_indices()[static_cast<underlying_cookie_type>(aCookie)];
            if (reverseIndex == INVALID_REVERSE_INDEX)
                throw cookie_invalid();
            if (reverseIndex < items().size() - 1)
            {
                auto& item = items()[reverseIndex];
                std::swap(item, items().back());
                auto& cookie = allocated_cookies()[reverseIndex];
                std::swap(cookie, allocated_cookies().back());
                reverse_indices()[static_cast<underlying_cookie_type>(cookie)] = reverseIndex;
            }
            auto resultIndex = reverseIndex;
            reverseIndex = INVALID_REVERSE_INDEX;
            allocated_cookies().pop_back();
            return_cookie(aCookie);
            items().pop_back();
            return resultIndex < items().size() ? std::next(items().begin(), resultIndex) : items().end();
        }
    public:
        cookie_type item_cookie(value_type const& aItem) const
        {
            if constexpr (!std::is_pointer_v<value_type>)
                return allocated_cookies()[&static_cast<value_type const&>(aItem) - &items()[0]];
            else
                throw no_pointer_value_type_cookie_lookup();
        }
        cookie_type next_cookie()
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            if (!free_cookies().empty())
            {
                auto const nextCookie = free_cookies().back();
                free_cookies().pop_back();
                return nextCookie;
            }
            auto const nextCookie = static_cast<cookie_type>(static_cast<underlying_cookie_type>(iNextAvailableCookie.load()) + underlying_cookie_type{ 1 });
            iNextAvailableCookie = nextCookie;
            if (nextCookie == INVALID_COOKIE)
                throw cookies_exhausted();
            assert(std::find(free_cookies().begin(), free_cookies().end(), nextCookie) == free_cookies().end());
            return nextCookie;
        }
        void return_cookie(cookie_type aCookie)
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            assert(std::find(free_cookies().begin(), free_cookies().end(), aCookie) == free_cookies().end());
            free_cookies().push_back(aCookie);
        }
    public:
        mutex_type& mutex() const
        {
            return iMutex;
        }
        const_iterator cbegin() const
        {
            return items().begin();
        }
        const_iterator begin() const
        {
            return cbegin();
        }
        iterator begin()
        {
            return items().begin();
        }
        const_iterator cend() const
        {
            return items().end();
        }
        const_iterator end() const
        {
            return cend();
        }
        iterator end()
        {
            return items().end();
        }
    public:
        void clear()
        {
            std::scoped_lock<mutex_type> lock{ mutex() };
            iNextAvailableCookie = cookie_type{};
            allocated_cookies().clear();
            free_cookies().clear();
            items().clear();
            reverse_indices().clear();
        }
        const container_type& items() const
        {
            return iItems;
        }
        container_type& items()
        {
            return iItems;
        }
    private:
        const cookies_t& allocated_cookies() const
        {
            return iAllocatedCookies;
        }
        cookies_t& allocated_cookies()
        {
            return iAllocatedCookies;
        }
        const cookies_t& free_cookies() const
        {
            return iFreeCookies;
        }
        cookies_t& free_cookies()
        {
            return iFreeCookies;
        }
        const reverse_indices_t& reverse_indices() const
        {
            return iReverseIndices;
        }
        reverse_indices_t& reverse_indices()
        {
            return iReverseIndices;
        }
    private:
        mutable mutex_type iMutex;
        mutable std::atomic<cookie_type> iNextAvailableCookie;
        cookies_t iAllocatedCookies;
        container_type iItems;
        mutable cookies_t iFreeCookies;
        reverse_indices_t iReverseIndices;
    };

    typedef basic_cookie_ref_ptr<cookie> cookie_ref_ptr;
    typedef basic_cookie_ref_ptr<small_cookie> small_cookie_ref_ptr;

    template <typename T, typename MutexType = null_mutex>
    using jar = basic_jar<T, vector<T>, cookie, MutexType>;
    template <typename T, typename MutexType = null_mutex>
    using small_jar = basic_jar<T, vector<T>, small_cookie, MutexType>;

    template <typename T, typename MutexType = null_mutex>
    using std_vector_jar = basic_std_vector_jar<T, cookie, MutexType>;
    template <typename T, typename MutexType = null_mutex>
    using small_std_vector_jar = basic_std_vector_jar<T, small_cookie, MutexType>;
}
