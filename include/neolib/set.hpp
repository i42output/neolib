// set.hpp - v1.0
/*
 *  Copyright (c) 2007-present, Leigh Johnston.  All Rights Reserved.
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
#include "mutable_set.hpp"
#include "container_iterator.hpp"
#include "i_set.hpp"

namespace neolib
{
	template <typename T, typename ConcreteType = T, typename Pred = std::less<typename ConcreteType::key_type> >
	class set : public reference_counted <i_set<T> >
	{
		// types
	public:
		typedef T abstract_key_type;
		typedef T abstract_value_type;
		typedef ConcreteType concrete_key_type;
		typedef ConcreteType concrete_value_type;
		typedef Pred compare_type;
		typedef mutable_set<concrete_key_type, compare_type> container_type;
	private:
		typedef i_set<T> abstract_base;
	public:
		typedef typename abstract_base::size_type size_type;
		typedef container::const_iterator<abstract_value_type, typename container_type::const_iterator> container_const_iterator;
		typedef container::iterator<abstract_value_type, typename container_type::iterator, typename container_type::const_iterator> container_iterator;
	protected:
		typedef typename abstract_base::abstract_const_iterator abstract_const_iterator;
		typedef typename abstract_base::abstract_iterator abstract_iterator;
		// construction
	public:
		set() :
			iEndConstIterator(new container_const_iterator(iSet.cend())),
			iEndIterator(new container_iterator(iSet.end()))
		{}
		set(const i_container& aOther) :
			iEndConstIterator(new container_const_iterator(iSet.cend())),
			iEndIterator(new container_iterator(iSet.end()))
		{
			assign(aOther);
		}
		template <typename InputIter>
		set(InputIter aFirst, InputIter aLast) :
			iSet(aFirst, aLast),
			iEndConstIterator(new container_const_iterator(iSet.cend())),
			iEndIterator(new container_iterator(iSet.end()))
		{}
		// operations
	public:
		const container_type& container() const { return iSet; }
		container_type& container() { return iSet; }
		// implementation
	public:
		// from i_container
		virtual size_type size() const { return iSet.size(); }
		virtual size_type max_size() const { return iSet.max_size(); }
		virtual void clear() { iSet.clear(); }
		virtual void assign(const i_container& aOther)
		{
			if (&aOther == this) 
				return;
			clear();
			for (const_iterator i = aOther.begin(); i != aOther.end(); ++i)
				iSet.insert(typename container_type::value_type(concrete_key_type(*i)));
		}
	private:
		// from i_container
		virtual abstract_const_iterator* do_begin() const { return new container_const_iterator(iSet.begin()); }
		virtual abstract_const_iterator* do_end() const { return iEndConstIterator.wrapped_iterator(); }
		virtual abstract_iterator* do_begin() { return new container_iterator(iSet.begin()); }
		virtual abstract_iterator* do_end() { return iEndIterator.wrapped_iterator(); }
		virtual abstract_iterator* do_erase(const abstract_const_iterator& aPosition) { return new container_iterator(iSet.erase(static_cast<const container_const_iterator&>(aPosition))); }
		virtual abstract_iterator* do_erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) { return new container_iterator(iSet.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); }
	public:
		// from i_set
		virtual abstract_iterator* do_insert(const abstract_value_type& aValue) { return new container_iterator(iSet.insert(concrete_value_type(aValue)).first); }
		virtual abstract_const_iterator* do_find(const abstract_key_type& aKey) const { return new container_const_iterator(iSet.find(concrete_key_type(aKey))); }
		virtual abstract_iterator* do_find(const abstract_key_type& aKey) { return new container_iterator(iSet.find(concrete_key_type(aKey))); }
	private:
		container_type iSet;
		const_iterator iEndConstIterator;
		iterator iEndIterator;
	};

	template <typename T, typename ConcreteType = T, typename Pred = std::less<typename ConcreteType::key_type> >
	class multiset : public reference_counted <i_multiset<T> >
	{
		// types
	public:
		typedef T abstract_key_type;
		typedef T abstract_value_type;
		typedef ConcreteType concrete_key_type;
		typedef ConcreteType concrete_value_type;
		typedef Pred compare_type;
		typedef mutable_multiset<concrete_key_type, compare_type> container_type;
	private:
		typedef i_multiset<T> abstract_base;
	public:
		typedef typename abstract_base::size_type size_type;
		typedef container::const_iterator<abstract_value_type, typename container_type::const_iterator> container_const_iterator;
		typedef container::iterator<abstract_value_type, typename container_type::iterator, typename container_type::const_iterator> container_iterator;
	protected:
		typedef typename abstract_base::abstract_const_iterator abstract_const_iterator;
		typedef typename abstract_base::abstract_iterator abstract_iterator;
		// construction
	public:
		multiset() :
			iEndConstIterator(new container_const_iterator(iSet.cend())),
			iEndIterator(new container_iterator(iSet.end()))
		{}
		multiset(const i_container& aOther) :
			iEndConstIterator(new container_const_iterator(iSet.cend())),
			iEndIterator(new container_iterator(iSet.end()))
		{
			assign(aOther);
		}
		template <typename InputIter>
		multiset(InputIter aFirst, InputIter aLast) :
			iSet(aFirst, aLast),
			iEndConstIterator(new container_const_iterator(iSet.cend())),
			iEndIterator(new container_iterator(iSet.end()))
		{}
		// operations
	public:
		const container_type& container() const { return iSet; }
		container_type& container() { return iSet; }
		// implementation
	public:
		// from i_container
		virtual size_type size() const { return iSet.size(); }
		virtual size_type max_size() const { return iSet.max_size(); }
		virtual void clear() { iSet.clear(); }
		virtual void assign(const i_container& aOther)
		{
			if (&aOther == this)
				return;
			clear();
			for (const_iterator i = aOther.begin(); i != aOther.end(); ++i)
				iSet.insert(typename container_type::value_type(concrete_key_type(*i)));
		}
	private:
		// from i_container
		virtual abstract_const_iterator* do_begin() const { return new container_const_iterator(iSet.begin()); }
		virtual abstract_const_iterator* do_end() const { return iEndConstIterator.wrapped_iterator(); }
		virtual abstract_iterator* do_begin() { return new container_iterator(iSet.begin()); }
		virtual abstract_iterator* do_end() { return iEndIterator.wrapped_iterator(); }
		virtual abstract_iterator* do_erase(const abstract_const_iterator& aPosition) { return new container_iterator(iSet.erase(static_cast<const container_const_iterator&>(aPosition))); }
		virtual abstract_iterator* do_erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) { return new container_iterator(iSet.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); }
	public:
		// from i_multiset
		virtual abstract_iterator* do_insert(const abstract_value_type& aValue) { return new container_iterator(iSet.insert(concrete_value_type(aValue))); }
		virtual abstract_const_iterator* do_find(const abstract_key_type& aKey) const { return new container_const_iterator(iSet.find(concrete_key_type(aKey))); }
		virtual abstract_iterator* do_find(const abstract_key_type& aKey) { return new container_iterator(iSet.find(concrete_key_type(aKey))); }
	private:
		container_type iSet;
		const_iterator iEndConstIterator;
		iterator iEndIterator;
	};
}
