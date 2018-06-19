// observable.hpp v1.3
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
#include <vector>
#include <list>
#include <algorithm>
#include <cassert>
#include "lifetime.hpp"

namespace neolib
{
	template <typename Observer>
	class observable
	{
		// types
	public:
		typedef Observer observer_type;
		typedef observable<observer_type> our_type;
		typedef typename observer_type::notify_type notify_type;
	protected:
		typedef std::vector<observer_type*> observer_list;
	private:
		typedef std::list<observer_list> notification_list;
	public:
		struct already_an_observer : std::logic_error { already_an_observer() : std::logic_error("neolib::observable::already_an_observer") {} };

		// construction
	public:
		observable() 
		{
		}
		observable(const observable&) 
		{ // no op == no copying of observer lists
		} 
		observable& operator=(const observable&) 
		{ // no op == no copying of observer lists
			return *this; 
		} 
		virtual ~observable()
		{
		}

		// operations
	public:
		virtual void add_observer(observer_type& aObserver)
		{
			if (std::find(iObservers.begin(), iObservers.end(), &aObserver) == iObservers.end())
				iObservers.push_back(&aObserver);
			else
				throw already_an_observer();
		}
		virtual void remove_observer(observer_type& aObserver)
		{
			iObservers.erase(std::remove(iObservers.begin(), iObservers.end(), &aObserver), iObservers.end());
			for (typename notification_list::iterator i = iNotifications.begin(); i != iNotifications.end(); ++i)
				i->erase(std::remove(i->begin(), i->end(), &aObserver), i->end());
		}
		template <typename T>
		void notify_observers(notify_type aType, const T& aParameter) const
		{
			notify_observers(aType, static_cast<const void*>(&aParameter));
		}
		template <typename T>
		void notify_observers(notify_type aType, const T& aParameter)
		{
			notify_observers(aType, static_cast<const void*>(&aParameter));
		}
		template <typename T1, typename T2>
		void notify_observers(notify_type aType, const T1& aParameter, const T2& aParameter2) const
		{
			notify_observers(aType, static_cast<const void*>(&aParameter), static_cast<const void*>(&aParameter2));
		}
		template <typename T1, typename T2>
		void notify_observers(notify_type aType, const T1& aParameter, const T2& aParameter2)
		{
			notify_observers(aType, static_cast<const void*>(&aParameter), static_cast<const void*>(&aParameter2));
		}
		void notify_observers(notify_type aType, const void* aParameter = 0, const void* aParameter2 = 0) const
		{
			do_notify_observers(*this, aType, aParameter, aParameter2);
		}
		void notify_observers(notify_type aType, const void* aParameter = 0, const void* aParameter2 = 0)
		{
			do_notify_observers(*this, aType, aParameter, aParameter2);
		}
		template <typename T>
		void notify_observer(const observer_type& aObserver, notify_type aType, const T& aParameter) const
		{
			notify_observer(aObserver, aType, static_cast<const void*>(&aParameter));
		}
		template <typename T>
		void notify_observer(observer_type& aObserver, notify_type aType, const T& aParameter)
		{
			notify_observer(aObserver, aType, static_cast<const void*>(&aParameter));
		}
		template <typename T1, typename T2> const
		void notify_observer(const observer_type& aObserver, notify_type aType, const T1& aParameter, const T2& aParameter2) const
		{
			notify_observer(aObserver, aType, static_cast<const void*>(&aParameter), static_cast<const void*>(&aParameter2));
		}
		template <typename T1, typename T2>
		void notify_observer(observer_type& aObserver, notify_type aType, const T1& aParameter, const T2& aParameter2)
		{
			notify_observer(aObserver, aType, static_cast<const void*>(&aParameter), static_cast<const void*>(&aParameter2));
		}

		// implementation
	protected:
		virtual void notify_observer(const observer_type& aObserver, notify_type aType, const void* aParameter = 0, const void* aParameter2 = 0) const { /* default: do nothing. */ (void)aObserver; (void)aType; (void)aParameter; (void)aParameter2; };
		virtual void notify_observer(observer_type& aObserver, notify_type aType, const void* aParameter = 0, const void* aParameter2 = 0) { /* default: do nothing. */ (void)aObserver; (void)aType; (void)aParameter; (void)aParameter2; };
	private:
		template <typename OurType>
		static void do_notify_observers(OurType& aThis, notify_type aType, const void* aParameter = 0, const void* aParameter2 = 0)
		{
			lifetime::destroyed_flag destroyed(aThis.iDestroyable);
			aThis.iNotifications.push_front(aThis.iObservers);
			typename notification_list::iterator theNotifications = aThis.iNotifications.begin();
			while (!theNotifications->empty())
			{
				observer_type* nextObserver = theNotifications->back();
				theNotifications->pop_back();
				aThis.notify_observer(*nextObserver, aType, aParameter, aParameter2);
				if (destroyed)
					return;
			}
			aThis.iNotifications.erase(theNotifications);
		}


		// attributes
	protected:
		observer_list iObservers;
	private:
		mutable notification_list iNotifications;
		mutable lifetime iDestroyable;
	};
}
