// utility.hpp - v1.0
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

/* WARNING: The classes present here are not a substitute for any equivalent std:: 
 * classes available on your platform which you should be using instead.  They exist here
 * either for technical reasons or for when there is no standard library available.
  */

#pragma once

#include <neolib/neolib.hpp>
#include <array>

namespace neolib
{
    template <typename T1, typename T2>
    struct pair
    {
        typedef T1 first_type;
        typedef T2 second_type;
        T1 first;
        T2 second;
        pair() : first(T1()), second(T2()) {}
        pair(const T1& x, const T2& y) : first(x), second(y) {}
        template <typename U, typename V> pair(const pair<U, V>& p) : first(p.first), second(p.second) {}
        template <typename U, typename V> pair<T1, T2>& operator=(const pair<U, V>& p) { first = p.first; second = p.second; return *this; }
        pair operator-() const { return make_pair(-first, -second); }
    };

    template <typename T1, typename T2>
    inline pair<T1, T2> make_pair(T1 x, T2 y)
    {
        return pair<T1, T2>(x, y);
    }

    template <typename T1, typename T2>
    struct minmax : pair<T1, T2>
    {
        typedef pair<T1, T2> base_type;
        minmax() {}
        minmax(const T1& x, const T2& y) : pair<T1, T2>(x, y) {}
        template <typename U, typename V> minmax(const minmax<U, V>& m) : pair<T1, T2>(m.first, m.second) {}
        template <typename U, typename V> minmax<T1, T2>& operator=(const minmax<U, V>& m) { base_type::first = m.first; base_type::second = m.second; return *this; }
        minmax operator-() const { minmax ret(-base_type::second, -base_type::first); return ret; }
    };
}
