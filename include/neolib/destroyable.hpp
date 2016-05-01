// destroyable.hpp
/*
 *  Copyright (c) 2012 Leigh Johnston.
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

namespace neolib
{
	class destroyable
	{
	public:
		class destroyed_flag
		{
		public:
			destroyed_flag(const destroyable& aOwner) : iOwner(aOwner), iDestroyed(false)
			{
				iOwner.add_flag(this);
			}
			~destroyed_flag()
			{
				if (!destroyed())
					iOwner.remove_flag(this);
			}
		public:
			bool destroyed() const
			{
				return iDestroyed;
			}
			operator bool() const
			{
				return iDestroyed;
			}
			void set_destroyed()
			{
				iDestroyed = true;
			}
		private:
			const destroyable& iOwner;
			bool iDestroyed;
		};
	public:
		virtual ~destroyable()
		{
			for (auto flag : iDestroyedFlags)
				flag->set_destroyed();
		}
	public:
		void add_flag(destroyed_flag* aFlag) const
		{
			iDestroyedFlags.insert(aFlag);
		}
		void remove_flag(destroyed_flag* aFlag) const
		{
			iDestroyedFlags.erase(aFlag);
		}
	private:
		mutable std::set<destroyed_flag*, std::less<destroyed_flag*>, boost::fast_pool_allocator<destroyed_flag*>> iDestroyedFlags;
	};
}
