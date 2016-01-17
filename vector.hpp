// vector.hpp - v1.0
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
#include "reference_counted.hpp"
#include "i_vector.hpp"
#include "container_iterator.hpp"

namespace neolib
{
	template <typename T, typename ConcreteType = T>
	class vector : public reference_counted<i_vector<T> >
	{
		// types
	private:
		typedef reference_counted<i_vector<T> > base;
	public:
		typedef T value_type;
		typedef ConcreteType concrete_type;
		typedef std::vector<concrete_type> container_type;
		typedef typename i_vector<T>::size_type size_type;
	protected:
		typedef container::random_access_const_iterator<T, typename container_type::const_iterator> container_const_iterator;
		typedef container::random_access_iterator<T, typename container_type::iterator, typename container_type::const_iterator> container_iterator;
		typedef typename base::abstract_const_iterator abstract_const_iterator;
		typedef typename base::abstract_iterator abstract_iterator;
		// construction
	public:
		vector() : iEndConstIterator(), iEndIterator() {}
		vector(const vector& aOther) : iVector(aOther.begin(), aOther.end()), iEndConstIterator(), iEndIterator() {}
		vector(const i_vector<T>& aOther) : iVector(aOther.begin(), aOther.end()), iEndConstIterator(), iEndIterator() {}
		vector(const container_type& aOtherContainer) : iVector(aOtherContainer), iEndConstIterator(), iEndIterator() {}
		template <typename InputIter>
		vector(InputIter aFirst, InputIter aLast) : iVector(aFirst, aLast), iEndConstIterator(), iEndIterator() {}
		vector& operator=(const vector& aOther) { assign(aOther); return *this; }
		vector& operator=(const i_vector<T>& aOther) { assign(aOther); return *this; }
		// operations
	public:
		container_type& container() { return iVector; }
		const container_type& container() const { return iVector; }
		// implementation
	public:
		// from i_container
		virtual size_type size() const { return iVector.size(); }
		virtual size_type max_size() const { return iVector.max_size(); }
		virtual void clear() { reset_cache(); iVector.clear(); }
		virtual void assign(const i_container& aOther) { if (&aOther == this) return; reset_cache(); iVector.assign(aOther.begin(), aOther.end()); }
	private:
		// from i_container
		virtual abstract_const_iterator* do_begin() const { populate_cache(); return new container_const_iterator(iVector.begin()); }
		virtual abstract_const_iterator* do_end() const { populate_cache(); return iEndConstIterator.wrapped_iterator(); }
		virtual abstract_iterator* do_begin() { populate_cache(); return new container_iterator(iVector.begin()); }
		virtual abstract_iterator* do_end() { populate_cache(); return iEndIterator.wrapped_iterator(); }
		virtual abstract_iterator* do_erase(const abstract_const_iterator& aPosition) { reset_cache();  return new container_iterator(iVector.erase(static_cast<const container_const_iterator&>(aPosition))); }
		virtual abstract_iterator* do_erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) { reset_cache(); return new container_iterator(iVector.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); }
	public:
		// from i_sequence_container
		virtual size_type capacity() const { return iVector.size(); }
		virtual void reserve(size_type aCapacity) { reset_cache(); iVector.reserve(aCapacity); }
		virtual void resize(size_type aSize, const value_type& aValue) { reset_cache(); iVector.resize(aSize, aValue); }
		virtual void push_back(const value_type& aValue) { reset_cache(); iVector.push_back(aValue); }
		virtual void pop_back() { reset_cache(); iVector.pop_back(); }
		virtual const value_type& back() const { return iVector.back(); }
		virtual value_type& back() { return iVector.back(); }
	private:
		// from i_sequence_container
		virtual abstract_iterator* do_insert(const abstract_const_iterator& aPosition, const value_type& aValue) { reset_cache(); return new container_iterator(iVector.insert(static_cast<const container_const_iterator&>(aPosition), aValue)); }
	public:
		// from i_vector
		virtual const value_type& operator[](size_type aIndex) const { return iVector[aIndex]; }
		virtual value_type& operator[](size_type aIndex) { return iVector[aIndex]; }
	private:
		void reset_cache() const { iEndConstIterator = const_iterator(); iEndIterator = iterator(); }
		void populate_cache() const 
		{ 
			if (iEndConstIterator.wrapped_iterator() == 0) 
				iEndConstIterator = const_iterator(new container_const_iterator(iVector.end())); 
		}
		void populate_cache()
		{
			if (iEndConstIterator.wrapped_iterator() == 0)
				iEndConstIterator = const_iterator(new container_const_iterator(iVector.end()));
			if (iEndIterator.wrapped_iterator() == 0)
				iEndIterator = iterator(new container_iterator(iVector.end()));
		}
		// attributes
	private:
		std::vector<ConcreteType> iVector;
		mutable const_iterator iEndConstIterator;
		mutable iterator iEndIterator;
	};
}
