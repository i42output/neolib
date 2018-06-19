// refable.h
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
#include <stdexcept>

namespace neolib
{
	template <typename ObjectType>
	class basic_ref
	{
		// types
	private:
		typedef basic_ref<ObjectType> self;
	public:
		typedef ObjectType* pointer;
		typedef const ObjectType* const_pointer;
		typedef ObjectType& reference;
		typedef const ObjectType& const_reference;
	public:
		struct bad_reference : std::logic_error { bad_reference() : std::logic_error("neolib::basic_ref::bad_reference") {} };

		// construction
	public:
		basic_ref();
		basic_ref(pointer aObject);
		basic_ref(reference aObject);
		basic_ref(const self& aOther);
		~basic_ref();
		basic_ref& operator=(const basic_ref& aOther);

		// operations
	public:
		virtual bool weak() const;
		bool valid() const;
		operator bool() const;
		void reset();
		reference operator*() const;
		pointer operator->() const;
		template <typename ObjectType2>
		operator ObjectType2&() const { return dynamic_cast<ObjectType2&>(**this); }
		bool operator==(const self& aOther) const;
		bool operator==(const_reference aObject) const;
		bool operator!=(const self& aOther) const;
		bool operator!=(const_reference aObject) const;
		bool operator<(const self& aOther) const;
		const self* next() const;
		self* next();
	private:
		void link();
		void unlink();
		void link(self*& aHead);
		void unlink(self*& aHead);

		// attributes
	private:
		bool iWeak;
		pointer iObject;
		self* iPrevious;
		self* iNext;
	};

	template <typename ObjectType>
	class basic_weak_ref : public basic_ref<ObjectType>
	{
		// types
	private:
		typedef basic_weak_ref<ObjectType> self;
		typedef basic_ref<ObjectType> base;
	public:
		typedef typename base::pointer pointer;
		typedef typename base::reference reference;

		// construction
	public:
		basic_weak_ref();
		basic_weak_ref(pointer aObject);
		basic_weak_ref(reference aObject);
		basic_weak_ref(const base& aOther);
		~basic_weak_ref();
		basic_weak_ref& operator=(const base& aOther);

		// operations
	public:
		virtual bool weak() const;
	};

	template <typename ObjectType>
	class refable
	{
		friend class basic_ref<ObjectType>;
		friend class basic_ref<const ObjectType>;
		friend class basic_weak_ref<ObjectType>;
		friend class basic_weak_ref<const ObjectType>;

		// types
	public:
		typedef basic_ref<ObjectType> ref;
		typedef basic_ref<const ObjectType> const_ref;
		typedef basic_weak_ref<ObjectType> weak_ref;
		typedef basic_weak_ref<const ObjectType> const_weak_ref;

		// construction
	public:
		refable();
		virtual ~refable();

		// operations
	public:
		void destroy() const;
		bool destroying() const;
		bool any_strong_references() const;
		std::size_t strong_reference_count() const;
	private:
		const_ref*& head(const_ref&) const;
		ref*& head(ref&);
		virtual void add_ref() const;
		virtual void remove_ref() const;
		void invalidate_all_refs() const;

		// attributes
	private:
		mutable const_ref* iConstRefHead;
		ref* iRefHead;
	protected:
		mutable bool iDestroying;
	};

	template <typename ObjectType>
	basic_ref<ObjectType>::basic_ref() : 
		iWeak(false), iObject(nullptr), iPrevious(nullptr), iNext(nullptr) 
	{
	}
	
	template <typename ObjectType>
	basic_ref<ObjectType>::basic_ref(pointer aObject) :
		iWeak(false), iObject(aObject), iPrevious(nullptr), iNext(nullptr) 
	{
		link();
	}

	template <typename ObjectType>
	basic_ref<ObjectType>::basic_ref(reference aObject) :
		iWeak(false), iObject(&aObject), iPrevious(nullptr), iNext(nullptr) 
	{
		link();
	}

	template <typename ObjectType>
	basic_ref<ObjectType>::basic_ref(const self& aOther) : 
		iWeak(false), iObject(aOther.iObject), iPrevious(nullptr), iNext(nullptr) 
	{
		link();
	}

	template <typename ObjectType>
	basic_ref<ObjectType>::~basic_ref() 
	{ 
		reset();
	}

	template <typename ObjectType>
	basic_ref<ObjectType>& basic_ref<ObjectType>::operator=(const self& aOther)
	{
		if (iObject != aOther.iObject)
		{
			reset();
			iObject = aOther.iObject;
			link();
		}
		return *this;
	}

	template <typename ObjectType>
	bool basic_ref<ObjectType>::weak() const
	{
		return false;
	}

	template <typename ObjectType>
	bool basic_ref<ObjectType>::valid() const 
	{ 
		return iObject != nullptr; 
	}

	template <typename ObjectType>
	void basic_ref<ObjectType>::reset()
	{
		if (valid())
		{
			unlink();
			iObject = nullptr;
			iPrevious = nullptr;
			iNext = nullptr;
		}
	}

	template <typename ObjectType>
	basic_ref<ObjectType>::operator bool() const 
	{ 
		return valid(); 
	}
	
	template <typename ObjectType>
	typename basic_ref<ObjectType>::reference basic_ref<ObjectType>::operator*() const 
	{ 
		if (!valid()) 
			throw bad_reference(); 
		return *iObject; 
	}
	
	template <typename ObjectType>
	typename basic_ref<ObjectType>::pointer basic_ref<ObjectType>::operator->() const 
	{ 
		if (!valid()) 
			throw bad_reference(); 
		return iObject; 
	}
	
	template <typename ObjectType>
	bool basic_ref<ObjectType>::operator==(const self& aOther) const
	{
		if (iObject == nullptr || aOther.iObject == nullptr)
			return false;
		else
			return iObject == aOther.iObject;
	}

	template <typename ObjectType>
	bool basic_ref<ObjectType>::operator==(const_reference aObject) const
	{
		return iObject == &aObject;
	}

	template <typename ObjectType>
	bool basic_ref<ObjectType>::operator!=(const self& aOther) const
	{
		return !operator==(aOther);
	}

	template <typename ObjectType>
	bool basic_ref<ObjectType>::operator!=(const_reference aOther) const
	{
		return !operator==(aOther);
	}

	template <typename ObjectType>
	bool basic_ref<ObjectType>::operator<(const self& aOther) const
	{
		return iObject < aOther.iObject;
	}

	template <typename ObjectType>
	const typename basic_ref<ObjectType>::self* basic_ref<ObjectType>::next() const
	{
		return iNext;
	}

	template <typename ObjectType>
	typename basic_ref<ObjectType>::self* basic_ref<ObjectType>::next()
	{
		return iNext;
	}

	template <typename ObjectType>
	void basic_ref<ObjectType>::link()
	{
		if (iObject)
		{
			link(iObject->head(*this));
			iObject->add_ref();
		}
	}

	template <typename ObjectType>
	void basic_ref<ObjectType>::unlink()
	{
		if (iObject)
		{
			unlink(iObject->head(*this));
			iObject->remove_ref();
		}
	}

	template <typename ObjectType>
	void basic_ref<ObjectType>::link(self*& aHead)
	{
		if (aHead == nullptr)
			aHead = this;
		else
		{
			aHead->iPrevious = this;
			iNext = aHead;
			aHead = this;
		}
	}

	template <typename ObjectType>
	void basic_ref<ObjectType>::unlink(self*& aHead)
	{
		if (iPrevious)
			iPrevious->iNext = iNext;
		if (iNext)
			iNext->iPrevious = iPrevious;
		if (aHead == this)
			aHead = iNext;
	}

	template <typename ObjectType>
	basic_weak_ref<ObjectType>::basic_weak_ref() : base() 
	{ 
	}

	template <typename ObjectType>
	basic_weak_ref<ObjectType>::basic_weak_ref(pointer aObject) : base(aObject)
	{
	}

	template <typename ObjectType>
	basic_weak_ref<ObjectType>::basic_weak_ref(reference aObject) : base(aObject)
	{
	}

	template <typename ObjectType>
	basic_weak_ref<ObjectType>::basic_weak_ref(const base& aOther) : base(aOther) 
	{ 
	}

	template <typename ObjectType>
	basic_weak_ref<ObjectType>::~basic_weak_ref()
	{
		reset();
	}
	
	template <typename ObjectType>
	basic_weak_ref<ObjectType>& basic_weak_ref<ObjectType>::operator=(const base& aOther)
	{
		base::operator=(aOther);
		return *this;
	}

	template <typename ObjectType>
	bool basic_weak_ref<ObjectType>::weak() const
	{
		return true;
	}

	template <typename ObjectType>
	refable<ObjectType>::refable() : iConstRefHead(nullptr), iRefHead(nullptr), iDestroying(false)
	{
	}

	template <typename ObjectType>
	refable<ObjectType>::~refable()
	{
		iDestroying = true;
		invalidate_all_refs();
	}

	template <typename ObjectType>
	void refable<ObjectType>::destroy() const
	{
		if (!destroying())
		{
			iDestroying = true;
			delete this;
		}
	}

	template <typename ObjectType>
	bool refable<ObjectType>::destroying() const
	{
		return iDestroying;
	}

	template <typename ObjectType>
	bool refable<ObjectType>::any_strong_references() const
	{
		for (const const_ref* i = iConstRefHead; i != nullptr; i = i->next())
			if (!i->weak())
				return true;
		for (const ref* i = iRefHead; i != nullptr; i = i->next())
			if (!i->weak())
				return true;
		return false;
	}

	template <typename ObjectType>
	std::size_t refable<ObjectType>::strong_reference_count() const
	{
		std::size_t count = 0;
		for (const const_ref* i = iConstRefHead; i != nullptr; i = i->next())
			if (!i->weak())
				++count;;
		for (const ref* i = iRefHead; i != nullptr; i = i->next())
			if (!i->weak())
				++count;
		return count;
	}

	template <typename ObjectType>
	typename refable<ObjectType>::const_ref*& refable<ObjectType>::head(const_ref&) const
	{
		return iConstRefHead;
	}

	template <typename ObjectType>
	typename refable<ObjectType>::ref*& refable<ObjectType>::head(ref&)
	{
		return iRefHead;
	}

	template <typename ObjectType>
	void refable<ObjectType>::add_ref() const
	{
	}

	template <typename ObjectType>
	void refable<ObjectType>::remove_ref() const
	{
		if (!any_strong_references() && !iDestroying)
			destroy();
	}

	template <typename ObjectType>
	void refable<ObjectType>::invalidate_all_refs() const
	{
		const_ref* nextConstRef = iConstRefHead;
		while(nextConstRef)
		{
			const_ref* nextNext = nextConstRef->next();
			nextConstRef->reset();
			nextConstRef = nextNext;
		}
		ref* nextRef = iRefHead;
		while(nextRef)
		{
			ref* nextNext = nextRef->next();
			nextRef->reset();
			nextRef = nextNext;
		}
	}
}

