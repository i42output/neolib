// lifetime.hpp
/*
 *  Copyright (c) 2007-present, Leigh Johnston.
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
#include <unordered_set>
#include <boost/pool/pool_alloc.hpp>
#include <boost/optional.hpp>
#include "i_lifetime.hpp"

namespace neolib
{
	template <lifetime_state RequiredState>
	class lifetime_flag : public i_lifetime_flag
	{
		friend class lifetime;
	public:
		lifetime_flag(const i_lifetime& aOwner) : iOwner{ aOwner }, iState{ iOwner.state() }
		{
			iOwner.add_flag(this);
		}
		lifetime_flag(const lifetime_flag& aOther) : iOwner{ aOther.iOwner }, iState{ iOwner.state() }
		{
			iOwner.add_flag(this);
		}
		~lifetime_flag()
		{
			if (!is_destroyed())
				iOwner.remove_flag(this);
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
	private:
		void set_alive() final
		{
			iState = lifetime_state::Alive;
		}
		void set_destroying() final
		{
			iState = lifetime_state::Destroying;
		}
		void set_destroyed() final
		{
			iState = lifetime_state::Destroyed;
			iOwner.remove_flag(this);
		}
	private:
		const i_lifetime& iOwner;
		lifetime_state iState;
	};

	typedef lifetime_flag<lifetime_state::Destroyed> destroyed_flag;
	typedef boost::optional<destroyed_flag> optional_destroyed_flag;

	class lifetime : public i_lifetime
	{
	public:
		typedef neolib::destroyed_flag destroyed_flag;
	private:
		typedef std::unordered_set<i_lifetime_flag*, std::hash<i_lifetime_flag*>, std::equal_to<i_lifetime_flag*>, boost::fast_pool_allocator<i_lifetime_flag*>> lifetime_flag_list;
	public:
		lifetime(lifetime_state aState = lifetime_state::Alive) : iState{ aState }
		{
		}
		virtual ~lifetime()
		{
			if (!is_destroyed())
			{
				set_destroying();
				set_destroyed();
			}
		}
	public:
		lifetime_state state() const final
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
			if (!is_creating())
				throw not_creating();
			iState = lifetime_state::Alive;
			for (auto i = iFlags.begin(); i != iFlags.end();)
				(*i++)->set_alive();
		}
		void set_destroying() override
		{
			if (!is_destroying())
			{
				if (is_destroyed())
					throw already_destroyed();
				iState = lifetime_state::Destroying;
				for (auto i = iFlags.begin(); i != iFlags.end();)
					(*i++)->set_destroying();
			}
		}
		void set_destroyed() override
		{
			if (!is_destroyed())
			{
				if (iState == lifetime_state::Creating || iState == lifetime_state::Alive)
					set_destroying();
				iState = lifetime_state::Destroyed;
				for (auto i = iFlags.begin(); i != iFlags.end();)
					(*i++)->set_destroyed();
			}
		}
	public:
		void add_flag(i_lifetime_flag* aFlag) const final
		{
			iFlags.insert(aFlag);
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
			iFlags.erase(aFlag);
		}
	private:
		lifetime_state iState;
		mutable lifetime_flag_list iFlags;
	};
}
