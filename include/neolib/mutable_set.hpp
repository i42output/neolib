// mutable_set.hpp v1.2
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
#include <map>
#include <memory>

namespace neolib 
{
	template <typename Container>
	class mutable_base : public Container
	{
	public:
		typedef Container container_type;
		typedef typename container_type::key_type key_type;
		typedef typename container_type::mapped_type value_type;
	public:
		class iterator : public container_type::iterator
		{
		public:
			typedef typename mutable_base::value_type value_type;
			typedef value_type* pointer;
			typedef value_type& reference;
		public:
			iterator() {}
			iterator(typename container_type::iterator aIterator) : container_type::iterator(aIterator) {}
			pointer operator->() const { return &container_type::iterator::operator*().second; }
			reference operator*() const { return container_type::iterator::operator*().second; }
		};
		class const_iterator : public container_type::const_iterator
		{
		public:
			typedef typename mutable_base::value_type value_type;
			typedef const value_type* pointer;
			typedef const value_type& reference;
		public:
			const_iterator() {}
			const_iterator(typename container_type::const_iterator aIterator) : container_type::const_iterator(aIterator) {}
			const_iterator(typename container_type::iterator aIterator) : container_type::const_iterator(aIterator) {}
			pointer operator->() const { return &container_type::const_iterator::operator*().second; }
			reference operator*() const { return container_type::const_iterator::operator*().second; }
		};
	public:
		iterator begin() { return container_type::begin(); }
		const_iterator begin() const { return container_type::begin(); }
		iterator end() { return container_type::end(); }
		const_iterator end() const { return container_type::end(); }
		iterator find(const key_type& aKey) { return container_type::find(aKey); }
		const_iterator find(const key_type& aKey) const { return container_type::find(aKey); }
	};

	template <typename T, typename Pr = std::less<typename T::key_type>, typename Alloc = std::allocator<std::pair<typename T::key_type const, T> > >
	class mutable_set : public mutable_base<std::map<typename T::key_type, T, Pr, Alloc> >
	{
		typedef mutable_base<std::map<typename T::key_type, T, Pr, Alloc> > base_type;
	public:
		typename base_type::iterator insert(const typename base_type::value_type& aValue)
		{
			return typename base_type::iterator(base_type::insert(std::make_pair(static_cast<typename base_type::key_type>(aValue), aValue)).first);
		}
		using base_type::find;
		typename base_type::iterator find(const typename base_type::value_type& aValue)
		{
			return typename base_type::iterator(base_type::find(static_cast<typename base_type::key_type>(aValue)));
		}
		typename base_type::const_iterator find(const typename base_type::value_type& aValue) const
		{
			return typename base_type::const_iterator(base_type::find(static_cast<typename base_type::key_type>(aValue)));
		}
	};

	template <typename T, typename Pr = std::less<typename T::key_type>, typename Alloc = std::allocator<std::pair<typename T::key_type const, T> > >
	class mutable_multiset : public mutable_base<std::multimap<typename T::key_type, T, Pr, Alloc> >
	{
		typedef mutable_base<std::multimap<typename T::key_type, T, Pr, Alloc> > base_type;
	public:
		typename base_type::iterator insert(const typename base_type::value_type& aValue)
		{
			return typename base_type::iterator(base_type::insert(std::make_pair(static_cast<typename base_type::key_type>(aValue), aValue)));
		}
		using base_type::find;
		typename base_type::iterator find(const typename base_type::value_type& aValue)
		{
			return typename base_type::iterator(base_type::find(static_cast<typename base_type::key_type>(aValue)));
		}
		typename base_type::const_iterator find(const typename base_type::value_type& aValue) const
		{
			return typename base_type::const_iterator(base_type::find(static_cast<typename base_type::key_type>(aValue)));
		}
	};
}
