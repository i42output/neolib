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

#include <neolib/neolib.hpp>
#include <utility>
#include "i_pair.hpp"

namespace neolib
{
    template <typename T1, typename T2>
    class pair : public i_pair<abstract_t<T1>, abstract_t<T2>>, public std::pair<T1, T2>
    {
    public:
        typedef i_pair<abstract_t<T1>, abstract_t<T2>> abstract_type;
        typedef abstract_t<T1> first_abstract_type;
        typedef abstract_t<T2> second_abstract_type;
        typedef T1 first_type;
        typedef T2 second_type;
    private:
        typedef std::pair<first_type, second_type> concrete_base;
    public:
        pair() : concrete_base{ first_type{}, second_type{} } {}
        pair(const abstract_type& aPair) : concrete_base{ first_type{ aPair.first() }, second_type{ aPair.second() } } {}
        pair(const concrete_base& aPair) : concrete_base{ aPair } {}
        pair(const first_abstract_type& aFirst, const second_abstract_type& aSecond) : concrete_base{ first_type{ aFirst }, second_type{ aSecond } } {}
        pair(const first_type& aFirst, const second_type& aSecond) : concrete_base{ aFirst, aSecond } {}
    public:
        const first_type& first() const override { return concrete_base::first; }
        first_type& first() override { return concrete_base::first; }
        const second_type& second() const override { return concrete_base::second; }
        second_type& second() override { return concrete_base::second; }
    };

    template <typename T1, typename T2>
    inline pair<T1, T2> make_pair(T1&& aFirst, T2&& aSecond)
    {
        return pair{ std::forward<T1>(aFirst), std::forward<T2>(aSecond) };
    }

    namespace detail
    {
        template<typename T1, typename T2>
        struct abstract_type<std::pair<T1, T2>> { typedef typename abstract_type<T2>::type type; };
    }
}
