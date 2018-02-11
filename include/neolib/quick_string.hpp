// quick_string.h
/*
 *  Copyright (c) 2012-present, Leigh Johnston.  All Rights Reserved.
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
#include <string>
#include <array>
#include <set>
#include <algorithm>
#include <memory>
#include <iterator>
#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <boost/utility/string_view.hpp>
#include <neolib/variant.hpp>

namespace neolib 
{
	template <typename charT, typename Traits = std::char_traits<charT>, typename Alloc = std::allocator<charT> >
	struct quick_string
	{
	public:
		typedef typename std::basic_string<charT, Traits, Alloc> string_type;
		typedef typename string_type::traits_type traits_type;
		typedef typename string_type::allocator_type allocator_type;
		typedef typename string_type::value_type value_type;
		typedef typename string_type::size_type size_type;
		typedef typename string_type::difference_type difference_type;
		typedef typename string_type::reference reference;
		typedef typename string_type::pointer pointer;
		typedef typename string_type::iterator iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef typename boost::basic_string_view<charT, Traits> string_view_type;
		typedef typename string_view_type::const_reference const_reference;
		typedef typename string_view_type::const_pointer const_pointer;
		typedef typename string_view_type::const_iterator const_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		static const size_type npos;
	private:
		typedef std::pair<string_view_type, Alloc> view_contents_type;
		typedef neolib::variant<string_type, view_contents_type> contents_type;
	public:
		// construct/copy/destroy
		explicit quick_string(const Alloc& a = Alloc()) : iContents{ string_type{ a } } {}
		quick_string(const string_type& str) : iContents{ str } {}
		quick_string(const quick_string& str) : iContents{ str.iContents } {}
		quick_string(const quick_string& str, size_type pos, size_type n = npos) : iContents{ view_contents_type{ string_view_type{ str.begin() + pos, str.begin() + pos + (n == npos ? str.size() - pos : n) }, str.get_allocator() } } {}
		quick_string(const charT* s, size_type n, const Alloc& a = Alloc()) : iContents{ view_contents_type{ string_view_type{ s, n }, a } } {}
		quick_string(const charT* s, const Alloc& a = Alloc()) : iContents{ view_contents_type{ string_view_type{ s, }, a } } {}
		quick_string(size_type n, charT c, const Alloc& a = Alloc()) : iContents{ string_type{ n, c, a } } {}
		template<class InputIterator>
		quick_string(InputIterator begin, InputIterator end, const Alloc& a = Alloc()) : iContents{ string_type{ begin, end, a } } {}
		quick_string(const quick_string& str, const Alloc& a) : iContents{ str.iContents.is<string_type>() ?
			contents_type{ string_type{ static_variant_cast<const string_type&>(str.iContents), a } } :
			contents_type{ view_contents_type{ static_variant_cast<const view_contents_type&>(str.iContents).first, a } } } {}
		quick_string(const_iterator begin, const_iterator end, const Alloc& a = Alloc()) : iContents{ view_contents_type{ string_view_type{ begin, static_cast<size_type>(std::distance(begin, end)) }, a } } {}
		quick_string(iterator begin, iterator end) : iContents(begin, end) : iContents{ view_contents_type{ string_view_type{ begin, end }, a } } {}
		quick_string& operator=(const quick_string& str) { iContents = str.iContents; return *this; }
		quick_string& operator=(const charT* s) { iContents = view_contents_type{ string_view_type{ s }, Alloc{} }; return *this; }
		quick_string& operator=(charT c) { iContents = string_type{ 1, c }; return *this; }
		// iterators
		iterator begin() { return get_string().begin(); }
		const_iterator begin() const { return is_view() ? get_view_string().begin() : static_cast<string_view_type>(get_string()).begin(); }
		const_iterator cbegin() const { return is_view() ? get_view_string().begin() : static_cast<string_view_type>(get_string()).begin(); }
		iterator end() { return get_string().end(); }
		const_iterator end() const { return is_view() ? get_view_string().end() : static_cast<string_view_type>(get_string()).end(); }
		const_iterator cend() const { return is_view() ? get_view_string().end() : static_cast<string_view_type>(get_string()).end(); }
		reverse_iterator rbegin() { return reverse_iterator(end()); }
		const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
		const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
		reverse_iterator rend() { return reverse_iterator(begin()); }
		const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
		const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }
		// capacity
		size_type size() const { return is_view() ? get_view_string().end() - get_view_string().begin() : get_string().size(); }
		size_type length() const { return is_view() ? get_view_string().end() - get_view_string().begin() : get_string().length(); }
		size_type max_size() const { return get_string().max_size(); }
		void resize(size_type n, charT c) { get_string().resize(n, c); }
		void resize(size_type n) { get_string().resize(n); }
		size_type capacity() const { return get_string().capacity(); }
		void reserve(size_type res_arg = 0) { get_string().reserve(res_arg); }
		void shrink_to_fit() { get_string().shrink_to_fit(); }
		void clear() { get_string().clear(); }
		bool empty() const { return is_view() ? get_view_string().begin() == get_view_string().end() : get_string().empty(); }
		// element access
		const_reference operator[](size_type pos) const 
		{ 
			if (is_view())
				return get_view_string()[pos];
			else
				return get_string()[pos]; 
		}
		reference operator[](size_type pos)
		{ 
			return get_string()[pos]; 
		}
		const_reference at(size_type n) const 
		{ 
			if (is_view()) 
				return get_view_string().at(n);
			else
				return get_string().at(n);
		}
		reference at(size_type n)
		{ 
			return get_string().at(n);
		}
		// modifiers
		quick_string& operator+=(const string_type& str) { get_string().operator+=(str); return *this; }
		quick_string& operator+=(const charT* s) { get_string().operator+=(s); return *this; }
		quick_string& operator+=(charT c) { get_string().operator+=(c); return *this; }
		quick_string& append(const string_type& str) { get_string().append(str); return *this; }
		quick_string& append(const string_type& str, size_type pos, size_type n) { get_string().append(str, pos, n); return *this; }
		quick_string& append(const charT* s, size_type n) { get_string().append(s, n); return *this; }
		quick_string& append(const charT* s) { get_string().append(s); return *this; }
		quick_string& append(size_type n, charT c) { get_string().append(n, c); return *this;  }
		template<class InputIterator>
		quick_string& append(InputIterator first, InputIterator last) { get_string().append(first, last); return *this; }
		void push_back(charT c) { get_string().push_back(c); }
		quick_string& assign(const string_type& str) { get_string().assign(str); return *this; }
		quick_string& assign(const string_type& str, size_type pos, size_type n) { get_string().assign(str, pos, n); return *this; }
		quick_string& assign(const charT* s, size_type n) { get_string().assign(s, n); return *this; }
		quick_string& assign(const charT* s) { get_string().assign(s); return *this; }
		quick_string& assign(size_type n, charT c) { get_string().assign(n, c); return *this; }
		template<class InputIterator>
		quick_string& assign(InputIterator first, InputIterator last) { get_string().assign(first, last); return *this; }
		quick_string& insert(size_type pos1, const string_type& str) { get_string().insert(pos1, str); return *this; }
		quick_string& insert(size_type pos1, const string_type& str, size_type pos2, size_type n) { get_string().insert(pos1, str, pos2, n); return *this; }
		quick_string& insert(size_type pos, const charT* s, size_type n) { get_string().insert(pos, s, n); return *this; }
		quick_string& insert(size_type pos, const charT* s) { get_string().insert(pos, s); return *this; }
		quick_string& insert(size_type pos, size_type n, charT c) { get_string().insert(pos, n, c); return *this; }
		iterator insert(const_iterator p, charT c) { get_string().insert(p, c); return *this; }
		void insert(const_iterator p, size_type n, charT c) { get_string().insert(p, n, c); return *this; }
		template<class InputIterator>
		void insert(const_iterator p, InputIterator first, InputIterator last) { get_string().insert(p, first, last); return *this; }
		quick_string& erase(size_type pos = 0, size_type n = npos) { get_string().erase(pos, n); return *this; }
		iterator erase(iterator p) { return get_string().erase(p); } 
		iterator erase(iterator first, iterator last) { return get_string().erase(first, last); }
		quick_string& replace(size_type pos1, size_type n1, const string_type& str) { get_string().replace(pos1, n1, str); return *this; }
		quick_string& replace(size_type pos1, size_type n1, const string_type& str, size_type pos2, size_type n2) { get_string().replace(pos1, n1, str, pos2, n2); return *this; }
		quick_string& replace(size_type pos, size_type n1, const charT* s, size_type n2) { get_string().replace(pos, n1, s, n2); return *this; }
		quick_string& replace(size_type pos, size_type n1, const charT* s) { get_string().replace(pos, n1, s); return *this; }
		quick_string& replace(size_type pos, size_type n1, size_type n2, charT c) { get_string().replace(pos, n1, n2, c); return *this; }
		quick_string& replace(iterator i1, iterator i2, const string_type& str) { get_string().replace(i1, i2, str); return *this; }
		quick_string& replace(iterator i1, iterator i2, const charT* s, size_type n) { get_string().replace(i1, i2, s, n); return *this; }
		quick_string& replace(iterator i1, iterator i2, const charT* s) { get_string().replace(i1, i2, s); return *this; }
		quick_string& replace(iterator i1, iterator i2, size_type n, charT c) { get_string().replace(i1, i2, n, c); return *this; }
		template<class InputIterator>
		quick_string& replace(iterator i1, iterator i2, InputIterator j1, InputIterator j2) { get_string().replace(i1, i2, j1, j2); return *this; }
		size_type copy(charT* s, size_type n, size_type pos = 0) const 
		{
			if (is_view())
				return get_view_string().copy(s, n, pos);
			else
				return get_string().copy(s, n, pos);
		}
		void swap(quick_string& str)
		{
			std::swap(iContents, str.iContents);
		}
		// string_type operations:
		const charT* c_str() const // explicit
		{ 
			return get_string().c_str(); 
		} 
		const charT* data() const 
		{ 
			if (is_view())
				return get_view_string().data();
			else
				return get_string().data(); 
		}
		allocator_type get_allocator() const 
		{ 
			if (is_view())
				return static_cast<view_contents_type&>(iContents).second;
			else
				return get_string().get_allocator(); 
		}
		size_type find (const string_type& str, size_type pos = 0) const 
		{ 
			if (is_view())
				return get_view_string().find(str, pos);
			else
				return get_string().find(str, pos);
		}
		size_type find (const charT* s, size_type pos, size_type n) const
		{ 
			if (is_view())
				return get_view_string().find(s, pos, n);
			else
				return get_string().find(s, pos, n);
		}
		size_type find (const charT* s, size_type pos = 0) const
		{ 
			if (is_view())
				return get_view_string().find(s, pos);
			else
				return get_string().find(s, pos);
		}
		size_type find (charT c, size_type pos = 0) const
		{ 
			if (is_view())
				return get_view_string().find(c, pos);
			else
				return get_string().find(c, pos);
		}
		size_type rfind(const string_type& str, size_type pos = npos) const
		{ 
			if (is_view())
				return get_view_string().rfind(str, pos);
			else
				return get_string().rfind(str, pos);
		}
		size_type rfind(const charT* s, size_type pos, size_type n) const
		{ 
			if (is_view())
				return get_view_string().rfind(s, pos, n);
			else
				return get_string().rfind(s, pos, n);
		}
		size_type rfind(const charT* s, size_type pos = npos) const
		{ 
			if (is_view())
				return get_view_string().rfind(s, pos);
			else
				return get_string().rfind(s, pos);
		}
		size_type rfind(charT c, size_type pos = npos) const
		{ 
			if (is_view())
				return get_view_string().rfind(c, pos);
			else
				return get_string().rfind(c, pos);
		}
		size_type find_first_of(const string_type& str, size_type pos = 0) const
		{ 
			if (is_view())
				return get_view_string().find_first_of(str, pos);
			else
				return get_string().find_first_of(str, pos);
		}
		size_type find_first_of(const charT* s, size_type pos, size_type n) const
		{ 
			if (is_view())
				return get_view_string().find_first_of(s, pos, n);
			else
				return get_string().find_first_of(s, pos, n);
		}
		size_type find_first_of(const charT* s, size_type pos = 0) const
		{ 
			if (is_view())
				return get_view_string().find_first_of(s, pos);
			else
				return get_string().find_first_of(s, pos);
		}
		size_type find_first_of(charT c, size_type pos = 0) const
		{ 
			if (is_view())
				return get_view_string().find_first_of(c, pos);
			else
				return get_string().find_first_of(c, pos);
		}
		size_type find_last_of (const string_type& str, size_type pos = npos) const
		{
			if (is_view())
				return get_view_string().find_last_of(str, pos);
			else
				return get_string().find_last_of(str, pos);
		}
		size_type find_last_of (const charT* s, size_type pos, size_type n) const
		{
			if (is_view())
				return get_view_string().find_last_of(s, pos, n);
			else
				return get_string().find_last_of(s, pos, n);
		}
		size_type find_last_of (const charT* s, size_type pos = npos) const
		{
			if (is_view())
				return get_view_string().find_last_of(s, pos);
			else
				return get_string().find_last_of(s, pos);
		}
		size_type find_last_of (charT c, size_type pos = npos) const
		{
			if (is_view())
				return get_view_string().find_last_of(c, pos);
			else
				return get_string().find_last_of(c, pos);
		}
		size_type find_first_not_of(const string_type& str, size_type pos = 0) const
		{ 
			if (is_view())
				return get_view_string().find_first_not_of(str, pos);
			else
				return get_string().find_first_not_of(str, pos);
		}
		size_type find_first_not_of(const charT* s, size_type pos, size_type n) const
		{ 
			if (is_view())
				return get_view_string().find_first_not_of(s, pos, n);
			else
				return get_string().find_first_not_of(s, pos, n);
		}
		size_type find_first_not_of(const charT* s, size_type pos = 0) const
		{ 
			if (is_view())
				return get_view_string().find_first_not_of(s, pos);
			else
				return get_string().find_first_not_of(s, pos);
		}
		size_type find_first_not_of(charT c, size_type pos = 0) const
		{ 
			if (is_view())
				return get_view_string().find_first_not_of(c, pos);
			else
				return get_string().find_first_not_of(c, pos);
		}
		size_type find_last_not_of (const string_type& str, size_type pos = npos) const
		{
			if (is_view())
				return get_view_string().find_last_of(str, pos);
			else
				return get_string().find_last_of(str, pos);
		}
		size_type find_last_not_of (const charT* s, size_type pos, size_type n) const
		{
			if (is_view())
				return get_view_string().find_last_of(s, pos, n);
			else
				return get_string().find_last_of(s, pos, n);
		}
		size_type find_last_not_of (const charT* s, size_type pos = npos) const
		{
			if (is_view())
				return get_view_string().find_last_of(s, pos);
			else
				return get_string().find_last_of(s, pos);
		}
		size_type find_last_not_of (charT c, size_type pos = npos) const
		{
			if (is_view())
				return get_view_string().find_last_of(c, pos);
			else
				return get_string().find_last_of(c, pos);
		}
		string_type substr(size_type pos = 0, size_type n = npos) const
		{
			if (is_view())
				return string_type{ get_view_string().substr(pos, n) };
			else
				return get_string().substr(pos, n);
		}
		int compare(const quick_string& str) const
		{
			if (is_view())
				return get_view_string().compare(str);
			else
				return get_string().compare(str);
		}
		int compare(const string_type& str) const
		{
			if (is_view())
				return get_view_string().compare(str);
			else
				return get_string().compare(str);
		}
		int compare(size_type pos1, size_type n1, const string_type& str) const
		{
			return quick_string(*this, pos1, n1).compare(str);
		}
		int compare(size_type pos1, size_type n1, const string_type& str, size_type pos2, size_type n2) const
		{
			return quick_string(*this, pos1, n1).compare(quick_string(quick_string(str.begin(), str.end()), pos2, n2));
		}
		int compare(const charT* s) const
		{
			return compare(quick_string(s));
		}
		int compare(size_type pos1, size_type n1, const charT* s) const
		{
			return quick_string(*this, pos1, n1).compare(quick_string(s));
		}
		int compare(size_type pos1, size_type n1, const charT* s, size_type n2) const
		{
			return quick_string(*this, pos1, n1).compare(quick_string(s, n2));
		}
		// quick_string specific operations
	public:
		bool is_view() const 
		{ 
			return iContents.is<view_contents_type>(); 
		}
		operator string_type&()
		{
			if (!is_view())
				return iContents;
			else
			{
				iContents = string_type(get_view_string().begin(), get_view_string().end());
				return iContents;
			}
		}
		operator const string_type&() const
		{
			if (!is_view())
				return iContents;
			else
			{
				iContents = string_type(get_view_string().begin(), get_view_string().end());
				return iContents;
			}
		}
		operator string_view_type&()
		{
			if (is_view())
				return static_variant_cast<view_contents_type&>(iContents).first;
			else
				throw std::logic_error("neolib::quick_string::string_view_type");
		}
		operator const string_view_type&() const
		{
			if (is_view())
				return static_variant_cast<const view_contents_type&>(iContents).first;
			else
				throw std::logic_error("neolib::quick_string::string_view_type");
		}
	private:
		const string_type& get_string() const
		{
			if (is_view())
				iContents = string_type(get_view_string().begin(), get_view_string().end());
			return iContents;
		}
		string_type& get_string()
		{ 
			if (is_view())
				iContents = string_type(get_view_string().begin(), get_view_string().end());
			return static_cast<string_type&>(*this);
		}
		const string_view_type& get_view_string() const
		{
			return static_variant_cast<const view_contents_type&>(iContents).first;
		}
		string_view_type& get_view_string()
		{ 
			return static_variant_cast<view_contents_type&>(iContents).first;
		}
	private:
		mutable contents_type iContents;
	};

	template <typename charT, typename Traits, typename Alloc>
	const typename quick_string<charT, Traits, Alloc>::size_type quick_string<charT, Traits, Alloc>::npos = quick_string<charT, Traits, Alloc>::string_type::npos;


template<class charT,
	class Traits,
	class Alloc> inline
	quick_string<charT, Traits, Alloc> operator+(
		const quick_string<charT, Traits, Alloc>& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// return quick_string + quick_string
	return (quick_string<charT, Traits, Alloc>(_Left) += _Right);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	quick_string<charT, Traits, Alloc> operator+(
		const charT *_Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// return NTCS + quick_string
	return (quick_string<charT, Traits, Alloc>(_Left) += _Right);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	quick_string<charT, Traits, Alloc> operator+(
		const charT _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// return character + quick_string
	return (quick_string<charT, Traits, Alloc>(1, _Left) += _Right);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	quick_string<charT, Traits, Alloc> operator+(
		const quick_string<charT, Traits, Alloc>& _Left,
		const charT *_Right)
	{	// return quick_string + NTCS
	return (quick_string<charT, Traits, Alloc>(_Left) += _Right);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	quick_string<charT, Traits, Alloc> operator+(
		const quick_string<charT, Traits, Alloc>& _Left,
		const charT _Right)
	{	// return quick_string + character
	return (quick_string<charT, Traits, Alloc>(_Left) += _Right);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator==(
		const quick_string<charT, Traits, Alloc>& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test for quick_string equality
	return (_Left.compare(_Right) == 0);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator==(
		const quick_string<charT, Traits, Alloc>& _Left,
		const typename quick_string<charT, Traits, Alloc>::string_type& _Right)
	{	// test for quick_string equality
	return (_Left.compare(_Right) == 0);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator==(
		const typename quick_string<charT, Traits, Alloc>::string_type& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test for quick_string equality
	return (quick_string<charT, Traits, Alloc>(_Left.begin(), _Left.end()).compare(_Right) == 0);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator==(
		const charT * _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test for NTCS vs. quick_string equality
	return (_Right.compare(_Left) == 0);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator==(
		const quick_string<charT, Traits, Alloc>& _Left,
		const charT *_Right)
	{	// test for quick_string vs. NTCS equality
	return (_Left.compare(_Right) == 0);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator!=(
		const quick_string<charT, Traits, Alloc>& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test for quick_string inequality
	return (!(_Left == _Right));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator!=(
		const quick_string<charT, Traits, Alloc>& _Left,
		const typename quick_string<charT, Traits, Alloc>::string_type& _Right)
	{	// test for quick_string inequality
	return (!(_Left == _Right));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator!=(
	const typename quick_string<charT, Traits, Alloc>::string_type& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test for quick_string inequality
	return (!(_Left == _Right));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator!=(
		const charT *_Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test for NTCS vs. quick_string inequality
	return (!(_Left == _Right));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator!=(
		const quick_string<charT, Traits, Alloc>& _Left,
		const charT *_Right)
	{	// test for quick_string vs. NTCS inequality
	return (!(_Left == _Right));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator<(
		const quick_string<charT, Traits, Alloc>& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if quick_string < quick_string
	return (_Left.compare(_Right) < 0);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator<(
		const quick_string<charT, Traits, Alloc>& _Left,
		const typename quick_string<charT, Traits, Alloc>::string_type& _Right)
	{	// test if quick_string < quick_string
	return (_Left.compare(_Right) < 0);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator<(
	const typename quick_string<charT, Traits, Alloc>::string_type& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if quick_string < quick_string
	return (quick_string<charT, Traits, Alloc>(_Left.begin(), _Left.end()).compare(_Right) < 0);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator<(
		const charT * _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if NTCS < quick_string
	return (_Right.compare(_Left) > 0);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator<(
		const quick_string<charT, Traits, Alloc>& _Left,
		const charT *_Right)
	{	// test if quick_string < NTCS
	return (_Left.compare(_Right) < 0);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator>(
		const quick_string<charT, Traits, Alloc>& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if quick_string > quick_string
	return (_Right < _Left);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator>(
		const quick_string<charT, Traits, Alloc>& _Left,
		const typename quick_string<charT, Traits, Alloc>::string_type& _Right)
	{	// test if quick_string > quick_string
	return (_Right < _Left);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator>(
	const typename quick_string<charT, Traits, Alloc>::string_type& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if quick_string > quick_string
	return (_Right < _Left);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator>(
		const charT * _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if NTCS > quick_string
	return (_Right < _Left);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator>(
		const quick_string<charT, Traits, Alloc>& _Left,
		const charT *_Right)
	{	// test if quick_string > NTCS
	return (_Right < _Left);
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator<=(
		const quick_string<charT, Traits, Alloc>& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if quick_string <= quick_string
	return (!(_Right < _Left));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator<=(
		const quick_string<charT, Traits, Alloc>& _Left,
		const typename quick_string<charT, Traits, Alloc>::string_type& _Right)
	{	// test if quick_string <= quick_string
	return (!(_Right < _Left));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator<=(
	const typename quick_string<charT, Traits, Alloc>::string_type& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if quick_string <= quick_string
	return (!(_Right < _Left));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator<=(
		const charT * _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if NTCS <= quick_string
	return (!(_Right < _Left));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator<=(
		const quick_string<charT, Traits, Alloc>& _Left,
		const charT *_Right)
	{	// test if quick_string <= NTCS
	return (!(_Right < _Left));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator>=(
		const quick_string<charT, Traits, Alloc>& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if quick_string >= quick_string
	return (!(_Left < _Right));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator>=(
		const quick_string<charT, Traits, Alloc>& _Left,
		const typename quick_string<charT, Traits, Alloc>::string_type& _Right)
	{	// test if quick_string >= quick_string
	return (!(_Left < _Right));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator>=(
		const typename quick_string<charT, Traits, Alloc>::string_type& _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if quick_string >= quick_string
	return (!(_Left < _Right));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator>=(
		const charT * _Left,
		const quick_string<charT, Traits, Alloc>& _Right)
	{	// test if NTCS >= quick_string
	return (!(_Left < _Right));
	}

template<class charT,
	class Traits,
	class Alloc> inline
	bool operator>=(
		const quick_string<charT, Traits, Alloc>& _Left,
		const charT *_Right)
	{	// test if quick_string >= NTCS
	return (!(_Left < _Right));
	}

template <typename charT>
class basic_character_map
{
public:
	basic_character_map(const std::basic_string<charT>& Characters) : 
		iMap()
	{
		for (typename std::basic_string<charT>::const_iterator i = Characters.begin(); i != Characters.end(); ++i)
			iMap[static_cast<typename std::make_unsigned<charT>::type>(*i)] = true;
	}
public:
	bool find(charT Character) const
	{
		return iMap[static_cast<typename std::make_unsigned<charT>::type>(Character)];
	}
private:
	std::array<bool, 256> iMap;
};

template <>
class basic_character_map<wchar_t>
{
public:
	basic_character_map(const std::basic_string<wchar_t>& Characters)
	{
		for (std::basic_string<wchar_t>::const_iterator i = Characters.begin(); i != Characters.end(); ++i)
			iMap.insert(*i);
	}
public:
	bool find(wchar_t Character) const
	{
		return iMap.find(Character) != iMap.end();
	}
private:
	std::set<wchar_t> iMap;
};

template <typename Elem, typename Traits, typename Alloc>
inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const quick_string<Elem, Traits, Alloc>& aString)
{
	aStream << std::basic_string<Elem, Traits>{ aString.data(), aString.size() };
	return aStream;
}
}