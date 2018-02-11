// visitor.hpp - v1.0
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

namespace neolib
{
	template <unsigned int N>
	struct unused_visitee {};

	template<typename T1, typename T2 = unused_visitee<2>, typename T3 = unused_visitee<3>, typename T4 = unused_visitee<4>,
		typename T5 = unused_visitee<5>, typename T6 = unused_visitee<6>, typename T7 = unused_visitee<7>, 
		typename T8 = unused_visitee<8>, typename T9 = unused_visitee<9>, typename T10 = unused_visitee<10> >
	class visitor
	{
	public:
		virtual ~visitor() {}
	public:
		virtual void visit(const T1& aVisitee) const {}
		virtual void visit(T1& aVisitee) { visit(const_cast<const T1&>(aVisitee)); }
		virtual void visit(const T2& aVisitee) const {}
		virtual void visit(T2& aVisitee) { visit(const_cast<const T2&>(aVisitee)); }
		virtual void visit(const T3& aVisitee) const {}
		virtual void visit(T3& aVisitee) { visit(const_cast<const T3&>(aVisitee)); }
		virtual void visit(const T4& aVisitee) const {}
		virtual void visit(T4& aVisitee) { visit(const_cast<const T4&>(aVisitee)); }
		virtual void visit(const T5& aVisitee) const {}
		virtual void visit(T5& aVisitee) { visit(const_cast<const T5&>(aVisitee)); }
		virtual void visit(const T6& aVisitee) const {}
		virtual void visit(T6& aVisitee) { visit(const_cast<const T6&>(aVisitee)); }
		virtual void visit(const T7& aVisitee) const {}
		virtual void visit(T7& aVisitee) { visit(const_cast<const T7&>(aVisitee)); }
		virtual void visit(const T8& aVisitee) const {}
		virtual void visit(T8& aVisitee) { visit(const_cast<const T8&>(aVisitee)); }
		virtual void visit(const T9& aVisitee) const {}
		virtual void visit(T9& aVisitee) { visit(const_cast<const T9&>(aVisitee)); }
		virtual void visit(const T10& aVisitee) const {}
		virtual void visit(T10& aVisitee) { visit(const_cast<const T10&>(aVisitee)); }
	};

	template<typename T1, typename T2 = unused_visitee<2>, typename T3 = unused_visitee<3>, typename T4 = unused_visitee<4>,
		typename T5 = unused_visitee<5>, typename T6 = unused_visitee<6>, typename T7 = unused_visitee<7>, 
		typename T8 = unused_visitee<8>, typename T9 = unused_visitee<9>, typename T10 = unused_visitee<10> >
	class visitee
	{
	public:
		typedef visitor<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10> visitor_type;
	public:
		virtual void accept(const visitor_type& aVisitor) const = 0;
		virtual void accept(visitor_type& aVisitor) { accept(const_cast<const visitor_type&>(aVisitor)); }
	};
}
