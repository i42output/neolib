// i_pair.hpp
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

namespace neolib
{
    template <typename T1, typename T2>
    class i_pair
    {
        typedef i_pair<T1, T2> self_type;
    public:
        typedef self_type abstract_type;
        typedef T1 first_type;
        typedef T2 second_type;
    public:
        virtual self_type& operator=(const self_type& aRhs) = 0;
    public:
        virtual const first_type& first() const = 0;
        virtual first_type& first() = 0;
        virtual const second_type& second() const = 0;
        virtual second_type& second() = 0;
    public:
        friend void swap(self_type& a, self_type& b)
        {
            using std::swap;
            swap(a.first(), b.first());
            swap(a.second(), b.second());
        }
    public:
        constexpr bool operator==(const self_type& that) const noexcept
        {
            return first() == that.first() && second() == that.second();
        }
        constexpr std::partial_ordering operator<=>(const self_type& that) const noexcept
        {
            if (*this == that)
                return std::partial_ordering::equivalent;
            else if (std::forward_as_tuple(first(), second()) < std::forward_as_tuple(that.first(), that.second()))
                return std::partial_ordering::less;
            else
                return std::partial_ordering::greater;
        }
    };
}

namespace std 
{
    template<typename First, typename Second>
    struct tuple_size<neolib::i_pair<First, Second>>
        : std::integral_constant<std::size_t, 2> {};

    template<std::size_t I, typename First, typename Second>
    struct tuple_element<I, neolib::i_pair<First, Second>>;

    template<typename First, typename Second>
    struct tuple_element<0, neolib::i_pair<First, Second>> {
        using type = First;
    };

    template<typename First, typename Second>
    struct tuple_element<1, neolib::i_pair<First, Second>> {
        using type = Second;
    };
}

namespace neolib
{
    template<std::size_t I, typename First, typename Second>
    constexpr auto& get(i_pair<First, Second>& p) noexcept {
        if constexpr (I == 0) return p.first();
        else                  return p.second();
    }

    template<std::size_t I, typename First, typename Second>
    constexpr auto& get(i_pair<First, Second> const& p) noexcept {
        if constexpr (I == 0) return p.first();
        else                  return p.second();
    }

    template<std::size_t I, typename First, typename Second>
    constexpr auto&& get(i_pair<First, Second>&& p) noexcept {
        if constexpr (I == 0) return std::move(p.first());
        else                  return std::move(p.second());
    }
}
