// reference_counted.hpp - v1.0
/*
 *  Copyright (c) 2014 Leigh Johnston.
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
#include <functional>
#include "i_discoverable.hpp"

namespace neolib
{
	template <typename Base>
	class reference_counted : public Base
	{
	public:
		typedef Base base_type;
		typedef typename base_type::i_object_destruction_watcher i_object_destruction_watcher;
	public:
		reference_counted() : iReferenceCount(0), iPinned(false), iDestroying(false)
		{
		}
		reference_counted(const reference_counted& aOther) : iReferenceCount(0), iPinned(aOther.iPinned), iDestroying(false)
		{
		}
		virtual ~reference_counted()
		{
			iDestroying = true;
			for (auto i = iDestructionWatchers.begin(); i != iDestructionWatchers.end(); ++i)
				if (*i != 0)
					(*i)->object_being_destroyed(*this);
		}
		reference_counted& operator=(const reference_counted& aOther)
		{
			// do nothing
			return *this;
		}
	public:
		virtual void add_ref() const
		{
			++iReferenceCount;
		}
		virtual void release() const
		{
			if (--iReferenceCount <= 0 && !iPinned)
			{
				if (!iDestroying)
					delete this;
				else
					throw release_during_destruction();
			}
		}
		virtual const base_type* release_and_take_ownership() const
		{
			if (iReferenceCount != 1)
				throw too_many_references();
			iReferenceCount = 0;
			return this;
		}
		virtual base_type* release_and_take_ownership()
		{
			return const_cast<base_type*>(const_cast<const reference_counted*>(this)->release_and_take_ownership());
		}
		virtual void pin() const
		{
			iPinned = true;
		}
		virtual void unpin() const
		{
			iPinned = false;
			if (iReferenceCount <= 0)
				delete this;
		}
	public:
		virtual void subcribe_destruction_watcher(i_object_destruction_watcher& aWatcher) const
		{
			auto existingWatcher = std::find(iDestructionWatchers.begin(), iDestructionWatchers.end(), &aWatcher);
			if (existingWatcher != iDestructionWatchers.end())
				throw destruction_watcher_already_subscribed();
			iDestructionWatchers.push_back(&aWatcher);
		}
		virtual void unsubcribe_destruction_watcher(i_object_destruction_watcher& aWatcher) const
		{
			auto existingWatcher = std::find(iDestructionWatchers.begin(), iDestructionWatchers.end(), &aWatcher);
			if (existingWatcher == iDestructionWatchers.end())
				throw destruction_watcher_not_found();
			if (!iDestroying)
				iDestructionWatchers.erase(existingWatcher);
			else
				*existingWatcher = 0;
		}
	private:
		mutable int32_t iReferenceCount;
		mutable bool iPinned;
		bool iDestroying;
		mutable std::vector<i_object_destruction_watcher*> iDestructionWatchers;
	};

	template <typename Interface>
	class auto_ref : public i_auto_ref<Interface>
	{
	public:
		typedef i_auto_ref<Interface> base;
		typedef typename base::no_object no_object;
		typedef typename base::interface_not_found interface_not_found;
	public:
		auto_ref(Interface* aObject = 0) :
			iObject(aObject), iReferenceCounted(true)
		{
			if (valid())
				iObject->add_ref();
		}
		auto_ref(Interface& aObject) :
			iObject(&aObject), iReferenceCounted(false)
		{
		}
		auto_ref(const auto_ref& aOther) :
			iObject(aOther.ptr()), iReferenceCounted(aOther.reference_counted())
		{
			if (valid() && iReferenceCounted)
				iObject->add_ref();
		}
		auto_ref(const i_auto_ref& aOther) :
			iObject(aOther.ptr()), iReferenceCounted(aOther.reference_counted())
		{
			if (valid() && iReferenceCounted)
				iObject->add_ref();
		}
		auto_ref(i_discoverable& aDiscoverable) :
			iObject(0), iReferenceCounted(true)
		{
			if (!aDiscoverable.discover(*this))
				throw interface_not_found();
		}
		~auto_ref()
		{
			if (valid() && iReferenceCounted)
				iObject->release();
		}
		auto_ref& operator=(const auto_ref& aOther)
		{
			reset(aOther.ptr(), aOther.reference_counted());
			return *this;
		}
		auto_ref& operator=(const i_auto_ref& aOther)
		{
			reset(aOther.ptr(), aOther.reference_counted());
			return *this;
		}
	public:
		virtual bool reference_counted() const
		{
			return iReferenceCounted;
		}
		virtual void reset(Interface* aObject = 0, bool aReferenceCounted = true)
		{
			auto_ref copy(*this);
			if (valid() && iReferenceCounted)
				iObject->release();
			iObject = aObject;
			iReferenceCounted = aReferenceCounted;
			if (valid() && iReferenceCounted)
				iObject->add_ref();
		}
		virtual Interface* release()
		{
			if (iObject == 0)
				throw no_object();
			Interface* releasedObject = static_cast<Interface*>(iObject->release_and_take_ownership());
			iObject = 0;
			return releasedObject;
		}
		virtual bool valid() const
		{
			return iObject != 0;
		}
		virtual Interface* ptr() const
		{
			return iObject;
		}
		virtual Interface* operator->() const
		{
			if (iObject == 0)
				throw no_object();
			return iObject;
		}
		virtual Interface& operator*() const
		{
			if (iObject == 0)
				throw no_object();
			return *iObject;
		}
	private:
		Interface* iObject;
		bool iReferenceCounted;
	};

	template <typename Interface>
	class weak_auto_ref : public i_weak_auto_ref<Interface>
	{
	public:
		typedef i_weak_auto_ref<Interface> base;
		typedef typename base::no_object no_object;
		typedef typename base::interface_not_found interface_not_found;
		typedef typename base::bad_release bad_release;
		typedef typename base::wrong_object wrong_object;
	public:
		weak_auto_ref(Interface* aObject = 0) :
			iObject(aObject)
		{
			if (valid())
				iObject->subcribe_destruction_watcher(*this);
		}
		weak_auto_ref(Interface& aObject) :
			iObject(&aObject)
		{
			if (valid())
				iObject->subcribe_destruction_watcher(*this);
		}
		weak_auto_ref(const weak_auto_ref& aOther) :
			iObject(aOther.ptr())
		{
			if (valid())
				iObject->subcribe_destruction_watcher(*this);
		}
		weak_auto_ref(const i_auto_ref& aOther) :
			iObject(aOther.ptr())
		{
			if (valid())
				iObject->subcribe_destruction_watcher(*this);
		}
		weak_auto_ref(i_discoverable& aDiscoverable) :
			iObject(0)
		{
			if (!aDiscoverable.discover(*this))
				throw interface_not_found();
			if (valid())
				iObject->subcribe_destruction_watcher(*this);
		}
		~weak_auto_ref()
		{
			if (valid())
				iObject->unsubcribe_destruction_watcher(*this);
		}
		weak_auto_ref& operator=(const weak_auto_ref& aOther)
		{
			reset(aOther.ptr());
			return *this;
		}
		weak_auto_ref& operator=(const i_auto_ref<Interface>& aOther)
		{
			reset(aOther.ptr());
			return *this;
		}
	public:
		virtual bool reference_counted() const
		{
			return false;
		}
		virtual void reset(Interface* aObject = 0, bool = false)
		{
			weak_auto_ref copy(*this);
			iObject = aObject;
			if (valid())
				iObject->subcribe_destruction_watcher(*this);
		}
		virtual Interface* release()
		{
			if (iObject == 0)
				throw no_object();
			else
				throw bad_release();
		}
		virtual bool valid() const
		{
			return iObject != 0;
		}
		virtual Interface* ptr() const
		{
			return iObject;
		}
		virtual Interface* operator->() const
		{
			if (iObject == 0)
				throw no_object();
			return iObject;
		}
		virtual Interface& operator*() const
		{
			if (iObject == 0)
				throw no_object();
			return *iObject;
		}
	private:
		virtual void object_being_destroyed(i_reference_counted& aObject)
		{
			if (&aObject != iObject)
				throw wrong_object();
			iObject = 0;
		}
	private:
		Interface* iObject;
	};
}
