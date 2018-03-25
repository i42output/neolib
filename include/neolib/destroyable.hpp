// destroyable.hpp
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
#include <set>
#include <boost/pool/pool_alloc.hpp>
#include <boost/optional.hpp>
#include "i_destroyable.hpp"

namespace neolib
{
	class destroyed_flag : public i_destroyed_flag
	{
	public:
		destroyed_flag(const i_destroyable& aOwner) : iOwner{ aOwner }, iState{ i_destroyable::Alive }
		{
			iOwner.add_flag(this);
		}
		destroyed_flag(const destroyed_flag& aOther) : iOwner{ aOther.iOwner }, iState{ aOther.iState }
		{
			iOwner.add_flag(this);
		}
		~destroyed_flag()
		{
			if (!is_destroyed())
				iOwner.remove_flag(this);
		}
	public:
		bool is_alive() const final
		{
			return iState == i_destroyable::Alive;
		}
		bool is_destroying() const final
		{
			return iState == i_destroyable::Destroying;
		}
		bool is_destroyed() const final
		{
			return iState == i_destroyable::Destroyed;
		}
		operator bool() const final
		{
			return is_destroyed();
		}
		void set_destroying() final
		{
			iState = i_destroyable::Destroying;
		}
		void set_destroyed() final
		{
			iState = i_destroyable::Destroyed;
			iOwner.remove_flag(this);
		}
	private:
		const i_destroyable& iOwner;
		i_destroyable::state_e iState;
	};

	typedef boost::optional<neolib::destroyed_flag> optional_destroyed_flag;

	class destroyable : public i_destroyable
	{
	public:
		typedef neolib::destroyed_flag destroyed_flag;
	public:
		destroyable() : iState{ Alive }
		{
		}
		virtual ~destroyable()
		{
			if (!is_destroyed())
			{
				set_destroying();
				set_destroyed();
			}
		}
	public:
		bool is_alive() const final
		{
			return iState == Alive;
		}
		bool is_destroying() const final
		{
			return iState == Destroying;
		}
		bool is_destroyed() const final
		{
			return iState == Destroyed;
		}
		operator bool() const final
		{
			return is_destroyed();
		}
		void set_destroying() override
		{
			if (!is_destroying())
			{
				if (is_destroyed())
					throw already_destroyed();
				iState = Destroying;
				for (auto i = iDestroyedFlags.begin(); i != iDestroyedFlags.end();)
					(*i++)->set_destroying();
			}
		}
		void set_destroyed() override
		{
			if (!is_destroyed())
			{
				if (iState == Alive)
					set_destroying();
				iState = Destroyed;
				for (auto i = iDestroyedFlags.begin(); i != iDestroyedFlags.end();)
					(*i++)->set_destroyed();
			}
		}
	public:
		void add_flag(i_destroyed_flag* aFlag) const final
		{
			iDestroyedFlags.insert(aFlag);
			switch (iState)
			{
			case Alive:
				break;
			case Destroying:
				aFlag->set_destroying();
				break;
			case Destroyed:
			default:
				aFlag->set_destroying();
				aFlag->set_destroyed();
				break;
			}
		}
		void remove_flag(i_destroyed_flag* aFlag) const final
		{
			iDestroyedFlags.erase(aFlag);
		}
	private:
		state_e iState;
		mutable std::set<i_destroyed_flag*, std::less<i_destroyed_flag*>, boost::fast_pool_allocator<i_destroyed_flag*>> iDestroyedFlags;
	};
}
