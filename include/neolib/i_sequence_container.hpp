// i_sequence_container.hpp - v1.0
/*
 *  Copyright (c) 2007-present, Leigh Johnston.
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
#include "i_container.hpp"

namespace neolib
{
	template <typename T, typename ConstIteratorType, typename IteratorType, bool DefaultComparisonOperators = true>
	class i_sequence_container : public i_container<T, ConstIteratorType, IteratorType, DefaultComparisonOperators>
	{
	private:
		typedef i_container<T, ConstIteratorType, IteratorType, DefaultComparisonOperators> base;
	public:
		typedef typename base::value_type value_type;
		typedef typename base::size_type size_type;
		typedef typename base::const_iterator const_iterator;
		typedef typename base::iterator iterator;
		typedef typename base::abstract_const_iterator abstract_const_iterator;
		typedef typename base::abstract_iterator abstract_iterator;
	public:
		virtual size_type capacity() const = 0;
		virtual void reserve(size_type aCapacity) = 0;
		virtual void resize(size_type aSize, const value_type& aValue) = 0;
		iterator insert(const abstract_iterator& aPosition, const value_type& aValue) { return do_insert(const_iterator(aPosition), aValue); }
		iterator insert(const abstract_const_iterator& aPosition, const value_type& aValue) { return do_insert(aPosition, aValue); }
		virtual void push_back(const value_type& aValue) = 0;
		virtual void pop_back() = 0;
		virtual const value_type& back() const = 0;
		virtual value_type& back() = 0;
	private:
		virtual abstract_iterator* do_insert(const abstract_const_iterator& aPosition, const value_type& aValue) = 0;
	};
}
