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
#include <set>

namespace neolib
{
	class lifetime
	{
		// types
	public:
		class watcher
		{
			friend class lifetime;
			// construction
		public:
			watcher(lifetime& aParent, bool aMenu) : iParent(&aParent), iMenu(aMenu) { iParent->add(*this); }
			~watcher() { if (iParent != nullptr) iParent->remove(*this); }
			// operations
		public:
			bool lifetime_ended() const { return iParent == nullptr; }
			bool menu_open() const { return iMenu; }
		private:
			void lifetime_ending() { iParent = nullptr; }
			// attributes
		private:
			lifetime* iParent;
			bool iMenu;
		};
	private:
		typedef std::set<watcher*> watchers;
		// construction
	public:
		~lifetime()
		{
			for (watchers::iterator i = iWatchers.begin(); i != iWatchers.end(); ++i)
				(*i)->lifetime_ending();
		}
		// operations
	public:
		bool menu_open() const
		{
			for (watchers::const_iterator i = iWatchers.begin(); i != iWatchers.end(); ++i)
				if ((*i)->menu_open())
					return true;
			return false;
		}
		// implementation
	private:
		void add(watcher& aWatcher) 
		{ 
			iWatchers.insert(&aWatcher);
			lifetime_watcher_added(aWatcher);
		}
		void remove(watcher& aWatcher) 
		{ 
			watchers::iterator i = iWatchers.find(&aWatcher); 
			if (i != iWatchers.end())
				iWatchers.erase(i); 
			lifetime_watcher_removed(aWatcher);
		}
		virtual void lifetime_watcher_added(watcher& aWatcher) {}
		virtual void lifetime_watcher_removed(watcher& aWatcher) {}
		// attributes
	private:
		watchers iWatchers;
	};
}
