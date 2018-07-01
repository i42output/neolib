// list.hpp - v1.0
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
#include <list>
#include "reference_counted.hpp"
#include "i_list.hpp"
#include "container_iterator.hpp"
#include "container_helper.hpp"

namespace neolib
{
	template <typename T, typename ConcreteType = T>
	class list : public reference_counted<i_list<T> >
	{
		// types
	private:
		typedef reference_counted<i_list<T> > base;
	public:
		typedef T value_type;
		typedef ConcreteType concrete_type;
		typedef std::list<concrete_type> container_type;
		typedef typename i_list<T>::size_type size_type;
	protected:
		typedef container::const_iterator<T, typename container_type::const_iterator> container_const_iterator;
		typedef container::iterator<T, typename container_type::iterator, typename container_type::const_iterator> container_iterator;
		typedef typename base::abstract_const_iterator abstract_const_iterator;
		typedef typename base::abstract_iterator abstract_iterator;
		// construction
	public:
		list() : iEndConstIterator(new container_const_iterator(iList.end())), iEndIterator(new container_iterator(iList.end())) {}
		list(const list& aOther) : iList(aOther.begin(), aOther.end()), iEndConstIterator(new container_const_iterator(iList.end())), iEndIterator(new container_iterator(iList.end())) {}
		list(const i_list<T>& aOther) : iList(aOther.begin(), aOther.end()), iEndConstIterator(new container_const_iterator(iList.end())), iEndIterator(new container_iterator(iList.end())) {}
		list& operator=(const list& aOther) { assign(aOther); return *this; }
		list& operator=(const i_list<T>& aOther) { assign(aOther); return *this; }
		// operations
	public:
		container_type& container() { return iList; }
		const container_type& container() const { return iList; }
		// implementation
	public:
		// from i_container
		virtual size_type size() const { return iList.size(); }
		virtual size_type max_size() const { return iList.max_size(); }
		virtual void clear() { iList.clear(); }
		virtual void assign(const i_container& aOther) { if (&aOther == this) return; iList.assign(aOther.begin(), aOther.end()); }
	private:
		// from i_container
		virtual abstract_const_iterator* do_begin() const { return new container_const_iterator(iList.begin()); }
		virtual abstract_const_iterator* do_end() const { return iEndConstIterator.wrapped_iterator(); }
		virtual abstract_iterator* do_begin() { return new container_iterator(iList.begin()); }
		virtual abstract_iterator* do_end() { return iEndIterator.wrapped_iterator(); }
		virtual abstract_iterator* do_erase(const abstract_const_iterator& aPosition) { return new container_iterator(iList.erase(static_cast<const container_const_iterator&>(aPosition))); }
		virtual abstract_iterator* do_erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) { return new container_iterator(iList.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); }
	public:
		// from i_sequence_container
		virtual size_type capacity() const { return iList.max_size(); }
		virtual void reserve(size_type aCapacity) { /* do nothing */ }
		virtual void resize(size_type aSize, const value_type& aValue) { iList.resize(aSize, container::helper::converter<const value_type, const concrete_type>::to_concrete_type(aValue)); }
		virtual void push_back(const value_type& aValue) { iList.push_back(container::helper::converter<const value_type, const concrete_type>::to_concrete_type(aValue)); }
		virtual void pop_back() { iList.pop_back(); }
		virtual const value_type& back() const { return container::helper::converter<const value_type, const concrete_type>::to_abstract_type(iList.back()); }
		virtual value_type& back() { return container::helper::converter<value_type, concrete_type>::to_abstract_type(iList.back()); }
	private:
		// from i_sequence_container
		virtual abstract_iterator* do_insert(const abstract_const_iterator& aPosition, const value_type& aValue) { return new container_iterator(iList.insert(static_cast<const container_const_iterator&>(aPosition), container::helper::converter<const value_type, const concrete_type>::to_concrete_type(aValue))); }
	public:
		// from i_list
		virtual void push_front(const value_type& aValue) { iList.push_front(container::helper::converter<const value_type, const concrete_type>::to_concrete_type(aValue)); }
		virtual void pop_front() { iList.pop_front(); }
		virtual const value_type& front() const { return container::helper::converter<const value_type, const concrete_type>::to_abstract_type(iList.front()); }
		virtual value_type& front() { return container::helper::converter<value_type, concrete_type>::to_abstract_type(iList.front()); }
		// attributes
	private:
		std::list<ConcreteType> iList;
		const_iterator iEndConstIterator;
		iterator iEndIterator;
	};
}
