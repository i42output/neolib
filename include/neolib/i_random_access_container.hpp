// i_random_access_container.hpp
/*
 *  Copyright (c) 2019 Leigh Johnston.
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
#include <neolib/i_sequence_container.hpp>

namespace neolib
{
    template <typename T, bool DefaultComparisonOperators = true>
    class i_random_access_container : public i_sequence_container<T, i_random_access_const_iterator<T>, i_random_access_iterator<T>, DefaultComparisonOperators>
    {
        typedef i_random_access_container<T, DefaultComparisonOperators> self_type;
        typedef i_sequence_container<T, i_random_access_const_iterator<T>, i_random_access_iterator<T>, DefaultComparisonOperators> base_type;
    public:
        typedef self_type abstract_type;
    public:
        using typename base_type::value_type;
        using typename base_type::size_type;
        typedef const value_type* const_fast_iterator;
        typedef value_type* fast_iterator;
    public:
        using base_type::size;
    public:
        virtual const value_type* cdata() const = 0;
        virtual const value_type* data() const = 0;
        virtual value_type* data() = 0;
    public:
        const value_type& operator[](size_type aIndex) const { return data()[aIndex]; }
        value_type& operator[](size_type aIndex) { return data()[aIndex]; }
    public:
        const_fast_iterator cfbegin() const { return cdata(); }
        const_fast_iterator cfend() const { return cdata() + size(); }
        const_fast_iterator fbegin() const { return data(); }
        const_fast_iterator fend() const { return data() + size(); }
        fast_iterator fbegin() { return data(); }
        fast_iterator fend() { return data() + size(); }
    };
}
