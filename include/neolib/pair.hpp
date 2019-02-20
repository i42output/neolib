// pair.hpp - v1.0
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
#include <utility>
#include "i_pair.hpp"

namespace neolib
{
    template <typename T1, typename T2, typename ConcreteType1 = T1, typename ConcreteType2 = T2>
    class pair : public i_pair<T1, T2>, public std::pair<ConcreteType1, ConcreteType2>
    {
    public:
        typedef T1 first_abstract_type;
        typedef T2 second_abstract_type;
        typedef ConcreteType1 first_concrete_type;
        typedef ConcreteType2 second_concrete_type;
    private:
        typedef std::pair<ConcreteType1, ConcreteType2> concrete_base;
    public:
        pair() : concrete_base(first_concrete_type(), second_concrete_type()) {}
        pair(const i_pair<T1, T2>& aPair) : concrete_base(aPair.first(), aPair.second()) {}
        pair(const concrete_base& aPair) : concrete_base(aPair) {}
        pair(const first_abstract_type& aFirst, const second_abstract_type& aSecond) : concrete_base(aFirst, aSecond) {}
    public:
        virtual const first_abstract_type& first() const { return concrete_base::first; }
        virtual first_abstract_type& first() { return concrete_base::first; }
        virtual const second_abstract_type& second() const { return concrete_base::second; }
        virtual second_abstract_type& second() { return concrete_base::second; }
    };
}
