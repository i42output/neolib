// manager_of.hpp v1.2
/*
 *  Copyright (c) 2012-present, Leigh Johnston.
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
#include <memory>
#include "observable.hpp"

namespace neolib
{
	template <typename Manager, typename ManagerObserver, typename T>
	class manager_of
	{
		// types
	public:
		typedef T value_type;
		typedef std::tr1::shared_ptr<T> value_ptr;
		typedef typename ManagerObserver::notify_type notify_type;
		typedef typename observable<ManagerObserver> observable_type;
		// construction
	public:
		manager_of(Manager& aManager, notify_type aCreatedNotification, notify_type aDestroyedNotification) : 
			iManager(aManager), iCreatedNotification(aCreatedNotification), iDestroyedNotification(aDestroyedNotification) {}
		// operations
	public:
		virtual void object_created(value_type& aObject) 
		{
			static_cast<observable_type&>(iManager).notify_observers(iCreatedNotification, aObject);
		}
		virtual void object_destroyed(value_type& aObject)
		{ 
			static_cast<observable_type&>(iManager).notify_observers(iDestroyedNotification, aObject);
		}
	protected:
		template <typename C>
		void erase_object(C& aContainer, typename C::iterator aIter)
		{
			if (aContainer.empty())
				return;
			// smart pointers are used to ensure manager object count does not include the 
			// object being erased
			value_ptr tmp(Manager::value(aIter));
			aContainer.erase(aIter);
		}
		template <typename C>
		void erase_objects(C& aContainer, typename C::iterator aFirst, typename C::iterator aLast)
		{
			for (typename C::iterator i = aFirst; i != aLast;)
				erase_object(aContainer, i++);
		}
		// attributes
	private:
		Manager& iManager;
		notify_type iCreatedNotification;
		notify_type iDestroyedNotification;
	};
}
