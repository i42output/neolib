// lifetime.hpp
/*
 *  Copyright (c) 2007 Leigh Johnston.
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
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include "vecarray.hpp"
#include "null_mutex.hpp"
#include "allocator.hpp"
#include <optional>
#include "i_lifetime.hpp"

namespace neolib
{
	template <lifetime_state RequiredState, typename Owner = void>
	class lifetime_flag : public i_lifetime_flag
	{
		template <typename Mutex>
		friend class basic_lifetime;
	private:
		typedef const i_lifetime* subject_pointer;
		typedef Owner* owner_pointer;
	public:
		lifetime_flag(const i_lifetime& aSubject, owner_pointer aOwner = nullptr) : iCookie{ aSubject.next_cookie() }, iSubject { &aSubject }, iOwner{ aOwner }, iState{ aSubject.object_state() }, iDebug{ false }
		{
			subject().add_flag(this);
		}
		lifetime_flag(const lifetime_flag& aOther) : iCookie{ aOther.iSubject->next_cookie() }, iSubject{ aOther.iSubject }, iOwner{ aOther.iOwner }, iState { aOther.iSubject->object_state() }, iDebug{ false }
		{
			subject().add_flag(this);
		}
		~lifetime_flag()
		{
			if (!is_destroyed())
				subject().remove_flag(this);
		}
	public:
		cookie_type cookie() const
		{
			return iCookie;
		}
	public:
		bool is_creating() const final
		{
			return iState == lifetime_state::Creating;
		}
		bool is_alive() const final
		{
			return iState == lifetime_state::Alive;
		}
		bool is_destroying() const final
		{
			return iState == lifetime_state::Destroying;
		}
		bool is_destroyed() const final
		{
			return iState == lifetime_state::Destroyed;
		}
		operator bool() const final
		{
			return iState == RequiredState;
		}
		void set_alive() final
		{
			if (iState == lifetime_state::Alive)
				return;
			if (debug())
				std::cerr << "lifetime_flag::set_alive()" << std::endl;
			iState = lifetime_state::Alive;
		}
		void set_destroying() final
		{
			if (iState == lifetime_state::Destroying)
				return;
			if (debug())
				std::cerr << "lifetime_flag::set_destroying()" << std::endl;
			iState = lifetime_state::Destroying;
		}
		void set_destroyed() final
		{
			if (iState == lifetime_state::Destroyed)
				return;
			if (debug())
				std::cerr << "lifetime_flag::set_destroyed()" << std::endl;
			iState = lifetime_state::Destroyed;
		}
	public:
		bool debug() const override
		{
			return iDebug;
		}
		void set_debug(bool aDebug = true) override
		{
			iDebug = aDebug;
		}
	private:
		const i_lifetime& subject() const
		{
			return *iSubject;
		}
	private:
		cookie_type iCookie;
		subject_pointer iSubject;
		owner_pointer iOwner;
		std::atomic<lifetime_state> iState;
		bool iDebug;
	};

	typedef lifetime_flag<lifetime_state::Destroyed> destroyed_flag;
	typedef std::optional<destroyed_flag> optional_destroyed_flag;

	template <typename MutexType = std::recursive_mutex>
	class lifetime_flag_list
	{
	public:
		struct invalid_cookie : std::logic_error { invalid_cookie() : std::logic_error("neolib::lifetime_flag_list::invalid_cookie") {} };
		struct cookies_exhausted : std::logic_error { cookies_exhausted() : std::logic_error("neolib::lifetime_flag_list::cookies_exhausted") {} };
	public:
		typedef MutexType mutex_type;
		typedef i_lifetime_flag::cookie_type cookie_type;
		typedef std::vector<i_lifetime_flag*> flags_t;
		typedef typename flags_t::iterator iterator;
	private:
		typedef flags_t::size_type reverse_index_t;
		typedef std::vector<reverse_index_t> reverse_indices_t;
		typedef std::vector<cookie_type> free_cookies_t;
	private:
		static constexpr cookie_type INVALID_COOKIE = cookie_type{ ~0ul };
		static constexpr reverse_index_t INVALID_REVERSE_INDEX = reverse_index_t{ ~0ul };
	public:
		lifetime_flag_list() : iNextAvailableCookie{ 0ul }
		{
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
	public:
		i_lifetime_flag& flag(cookie_type aCookie)
		{
			std::lock_guard<mutex_type> lg{ mutex() };
			if (aCookie >= reverse_indices().size())
				throw invalid_cookie();
			auto flagIndex = reverse_indices()[aCookie];
			if (flagIndex == INVALID_REVERSE_INDEX)
				throw invalid_cookie();
			return *flags()[flagIndex].first;
		}
		void add_flag(i_lifetime_flag& aFlag)
		{
			std::lock_guard<mutex_type> lg{ mutex() };
			auto flagCookie = aFlag.cookie();
			flags().push_back(&aFlag);
			if (reverse_indices().size() < flagCookie + 1)
				reverse_indices().resize(flagCookie + 1, INVALID_REVERSE_INDEX);
			reverse_indices()[flagCookie] = iFlags.size() - 1;

		}
		void remove_flag(i_lifetime_flag& aFlag)
		{
			std::lock_guard<mutex_type> lg{ mutex() };
			auto cookie = aFlag.cookie();
			auto flagIndex = reverse_indices()[cookie];
			if (flagIndex == INVALID_REVERSE_INDEX)
				throw invalid_cookie();
			if (flagIndex < flags().size() - 1)
			{
				auto& reverseIndex = reverse_indices()[cookie];
				auto& index = flags()[flagIndex];
				reverseIndex = INVALID_REVERSE_INDEX;
				std::swap(index, flags().back());
				reverse_indices()[index->cookie()] = flagIndex;
			}
			flags().pop_back();
			free_cookies().push_back(cookie);
		}
	public:
		mutex_type& mutex() const
		{
			return iMutex;
		}
		iterator begin()
		{
			return iFlags.begin();
		}
		iterator end()
		{
			return iFlags.end();
		}
	private:
		flags_t& flags()
		{
			return iFlags;
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
		mutable std::atomic<cookie_type> iNextAvailableCookie;
		mutable free_cookies_t iFreeCookies;
		flags_t iFlags;
		reverse_indices_t iReverseIndices;
	};

	template <typename FlagList>
	class basic_lifetime : public i_lifetime
	{
	private:
		typedef FlagList flag_list_type;
	public:
		typedef neolib::destroyed_flag destroyed_flag;
		typedef typename flag_list_type::flags_t flags_t;
	private:
		typedef typename flag_list_type::mutex_type mutex_type;
	public:
		basic_lifetime(lifetime_state aState = lifetime_state::Alive) : iState{ aState }
		{
		}
		virtual ~basic_lifetime()
		{
			std::lock_guard<mutex_type> lk(iFlagList.mutex());
			if (!is_destroyed())
			{
				set_destroying();
				set_destroyed();
			}
		}
	public:
		lifetime_state object_state() const final
		{
			return iState;
		}
		bool is_creating() const final
		{
			return iState == lifetime_state::Creating;
		}
		bool is_alive() const final
		{
			return iState == lifetime_state::Alive;
		}
		bool is_destroying() const final
		{
			return iState == lifetime_state::Destroying;
		}
		bool is_destroyed() const final
		{
			return iState == lifetime_state::Destroyed;
		}
		void set_alive() override
		{
			std::lock_guard<mutex_type> lk(iFlagList.mutex());
			if (!is_creating())
				throw not_creating();
			iState = lifetime_state::Alive;
			for (auto i = flags().begin(); i != flags().end();)
				(*i++)->set_alive();
		}
		void set_destroying() override
		{
			std::lock_guard<mutex_type> lk(iFlagList.mutex());
			if (!is_destroying())
			{
				if (is_destroyed())
					throw already_destroyed();
				iState = lifetime_state::Destroying;
				for (auto i = flags().begin(); i != flags().end();)
					(*i++)->set_destroying();
			}
		}
		void set_destroyed() override
		{
			std::lock_guard<mutex_type> lk(iFlagList.mutex());
			if (!is_destroyed())
			{
				if (iState == lifetime_state::Creating || iState == lifetime_state::Alive)
					set_destroying();
				iState = lifetime_state::Destroyed;
				for (auto i = flags().begin(); i != flags().end();)
					(*i++)->set_destroyed();
			}
		}
	public:
		cookie_type next_cookie() const final
		{
			std::lock_guard<mutex_type> lk(flags().mutex());
			return flags().next_cookie();
		}
		void add_flag(i_lifetime_flag* aFlag) const final
		{
			std::lock_guard<mutex_type> lk(flags().mutex());
			flags().add_flag(*aFlag);
			switch (iState)
			{
			case lifetime_state::Creating:
			case lifetime_state::Alive:
				break;
			case lifetime_state::Destroying:
				aFlag->set_destroying();
				break;
			case lifetime_state::Destroyed:
			default:
				aFlag->set_destroying();
				aFlag->set_destroyed();
				break;
			}
		}
		void remove_flag(i_lifetime_flag* aFlag) const final
		{
			std::lock_guard<mutex_type> lk(flags().mutex());
			flags().remove_flag(*aFlag);
		}
	private:
		flag_list_type& flags() const
		{
			return iFlagList;
		}
	private:
		std::atomic<lifetime_state> iState;
		mutable flag_list_type iFlagList;
	};

	typedef basic_lifetime<lifetime_flag_list<null_mutex>> single_threaded_lifetime;
	typedef basic_lifetime<lifetime_flag_list<std::recursive_mutex>> multi_threaded_lifetime;

	typedef multi_threaded_lifetime lifetime;
}
