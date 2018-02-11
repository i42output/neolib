// i_container.hpp - v1.0
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
#include <algorithm>
#include "i_reference_counted.hpp"
#include "i_iterator.hpp"

namespace neolib
{
	template <typename T, typename ConstIteratorType, typename IteratorType, bool DefaultComparisonOperators = true>
	class i_container : public i_reference_counted
	{
	public:
		typedef T value_type;
		typedef size_t size_type;
		typedef typename ConstIteratorType abstract_const_iterator;
		typedef typename IteratorType abstract_iterator;
	public:
		typedef typename abstract_const_iterator::iterator_wrapper const_iterator;
		typedef typename abstract_iterator::iterator_wrapper iterator;
	public:
		virtual size_type size() const = 0;
		virtual size_type max_size() const = 0;
		bool empty() const { return size() == 0; }
		const_iterator begin() const { return do_begin(); }
		const_iterator end() const { return do_end(); }
		iterator begin() { return do_begin(); }
		iterator end() { return do_end(); }
		iterator erase(const abstract_iterator& aPosition) { return do_erase(const_iterator(aPosition)); }
		iterator erase(const abstract_const_iterator& aPosition) { return do_erase(aPosition); }
		iterator erase(const abstract_iterator& aFirst, const abstract_iterator& aLast) { return do_erase(const_iterator(aFirst), const_iterator(aLast)); }
		iterator erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) { return do_erase(aFirst, aLast); }
		virtual void clear() = 0;
		virtual void assign(const i_container& aRhs) = 0;
	public:
		i_container& operator=(const i_container& aRhs)
		{
			assign(aRhs);
			return *this;
		}
		template<bool Enable = DefaultComparisonOperators, typename T = bool>
		typename std::enable_if<Enable, T>::type operator==(const i_container& aRhs) const
		{
			return size() == aRhs.size() && std::equal(begin(), end(), aRhs.begin());
		}
		template<bool Enable = DefaultComparisonOperators, typename T = bool>
		typename std::enable_if<Enable, T>::type operator!=(const i_container& aRhs) const
		{
			return size() != aRhs.size() && !std::equal(begin(), end(), aRhs.begin());
		}
		template<bool Enable = DefaultComparisonOperators, typename T = bool>
		typename std::enable_if<Enable, T>::type operator<(const i_container& aRhs) const
		{
			return std::lexicographical_compare(begin(), end(), aRhs.begin(), aRhs.end());
		}
	private:
		virtual abstract_const_iterator* do_begin() const = 0;
		virtual abstract_const_iterator* do_end() const = 0;
		virtual abstract_iterator* do_begin() = 0;
		virtual abstract_iterator* do_end() = 0;
		virtual abstract_iterator* do_erase(const abstract_const_iterator& aPosition) = 0;
		virtual abstract_iterator* do_erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) = 0;
	};
}
