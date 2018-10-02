// cookie_jar.hpp
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

#include "neolib.hpp"
#include <vector>
#include <mutex>
#include <atomic>

namespace neolib
{
	typedef uint32_t cookie;

	class i_cookie_jar_item
	{
	public:
		virtual ~i_cookie_jar_item() {}
	public:
		virtual neolib::cookie cookie() const = 0;
	};

	inline cookie item_cookie(const i_cookie_jar_item& aItem)
	{
		return aItem.cookie();
	}

	inline cookie item_cookie(const i_cookie_jar_item* aItem)
	{
		return aItem->cookie();
	}

	template <typename T, typename MutexType = std::recursive_mutex>
	class cookie_jar
	{
	public:
		struct invalid_cookie : std::logic_error { invalid_cookie() : std::logic_error("neolib::cookie_jar::invalid_cookie") {} };
		struct cookies_exhausted : std::logic_error { cookies_exhausted() : std::logic_error("neolib::cookie_jar::cookies_exhausted") {} };
	public:
		typedef T value_type;
		typedef std::vector<value_type> jar_t;
		typedef typename jar_t::const_iterator const_iterator;
		typedef typename jar_t::iterator iterator;
		typedef MutexType mutex_type;
	private:
		typedef typename jar_t::size_type reverse_index_t;
		typedef std::vector<reverse_index_t> reverse_indices_t;
		typedef std::vector<neolib::cookie> free_cookies_t;
	private:
		static constexpr neolib::cookie INVALID_COOKIE = neolib::cookie{ ~0ul };
		static constexpr reverse_index_t INVALID_REVERSE_INDEX = reverse_index_t{ ~0ul };
	public:
		cookie_jar() : iNextAvailableCookie{ 0ul }
		{
		}
	public:
		const value_type& operator[](neolib::cookie aCookie) const
		{
			std::lock_guard<mutex_type> lg{ mutex() };
			if (aCookie >= reverse_indices().size())
				throw invalid_cookie();
			auto index = reverse_indices()[aCookie];
			if (index == INVALID_REVERSE_INDEX)
				throw invalid_cookie();
			return *jar()[index].first;
		}
		value_type& operator[](neolib::cookie aCookie)
		{
			return const_cast<value_type&>(const_cast<const decltype(*this)>(*this).operator[](aCookie));
		}
		iterator add(const value_type& aItem)
		{
			std::lock_guard<mutex_type> lg{ mutex() };
			auto cookie = item_cookie(aItem);
			auto result = jar().insert(jar().end(), aItem);
			if (reverse_indices().size() < cookie + 1)
				reverse_indices().resize(cookie + 1, INVALID_REVERSE_INDEX);
			reverse_indices()[cookie] = iJar.size() - 1;
			return result;
		}
		iterator remove(const value_type& aItem)
		{
			std::lock_guard<mutex_type> lg{ mutex() };
			auto cookie = item_cookie(aItem);
			auto cookieIndex = reverse_indices()[cookie];
			if (cookieIndex == INVALID_REVERSE_INDEX)
				throw invalid_cookie();
			iterator result = jar().end();
			if (cookieIndex < jar().size() - 1)
			{
				auto& reverseIndex = reverse_indices()[cookie];
				auto& item = jar()[reverseIndex];
				result = std::next(jar().begin(), reverseIndex);
				std::swap(item, jar().back());
				reverseIndex = INVALID_REVERSE_INDEX;
				reverse_indices()[item_cookie(item)] = cookieIndex;
			}
			jar().pop_back();
			return_cookie(cookie);
			return result;
		}
	public:
		neolib::cookie next_cookie()
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
		void return_cookie(neolib::cookie aCookie)
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
			return iJar.begin();
		}
		const_iterator begin() const
		{
			return cbegin();
		}
		iterator begin()
		{
			return iJar.begin();
		}
		const_iterator cend() const
		{
			return iJar.end();
		}
		const_iterator end() const
		{
			return cend();
		}
		iterator end()
		{
			return iJar.end();
		}
	public:
		void clear()
		{
			std::lock_guard<mutex_type> lg{ mutex() };
			iNextAvailableCookie = 0ul;
			iFreeCookies.clear();
			iJar.clear();
			iReverseIndices.clear();
		}
	private:
		jar_t& jar()
		{
			return iJar;
		}
		reverse_indices_t& reverse_indices()
		{
			return iReverseIndices;
		}
		free_cookies_t& free_cookies()
		{
			return iFreeCookies;
		}
	private:
		mutable mutex_type iMutex;
		mutable std::atomic<neolib::cookie> iNextAvailableCookie;
		mutable free_cookies_t iFreeCookies;
		jar_t iJar;
		reverse_indices_t iReverseIndices;
	};
}
