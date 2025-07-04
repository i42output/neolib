// pair.hpp
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
#include <neolib/core/i_pair.hpp>

namespace neolib
{
    template <typename T1, typename T2>
    class pair : public i_pair<abstract_t<T1>, abstract_t<T2>>, public std::pair<T1, T2>
    {
        typedef pair<T1, T2> self_type;
        typedef i_pair<abstract_t<T1>, abstract_t<T2>> base_type;
    public:
        using typename base_type::abstract_type;
        typedef abstract_t<T1> first_abstract_type;
        typedef abstract_t<T2> second_abstract_type;
        typedef T1 first_type;
        typedef T2 second_type;
    private:
        typedef std::pair<first_type, second_type> concrete_type;
    public:
        pair() : concrete_type{ first_type{}, second_type{} } {}
        pair(const abstract_type& aPair) : concrete_type{ first_type{ aPair.first() }, second_type{ aPair.second() } } {}
        pair(const concrete_type& aPair) : concrete_type{ aPair } {}
        template <typename T3, typename T4>
        pair(T3&& aFirst, T4&& aSecond) : concrete_type{ std::forward<T3>(aFirst), std::forward<T4>(aSecond) } {}
    public:
        self_type& operator=(const self_type& aOther) { return assign(aOther); }
        abstract_type& operator=(const abstract_type& aOther) final { return assign(aOther); }
    public:
        const first_type& first() const final { return concrete_type::first; }
        first_type& first() final { return concrete_type::first; }
        const second_type& second() const final { return concrete_type::second; }
        second_type& second() final { return concrete_type::second; }
    public:
        self_type& assign(const abstract_type& aOther)
        {
            if constexpr (!std::is_const_v<first_type> && !std::is_const_v<second_type>)
            {
                concrete_type::operator=(concrete_type{ first_type{ aOther.first() }, second_type{ aOther.second() } });
                return *this;
            }
            else
                throw std::logic_error("neolib::pair isn't assignable (const value_type)");
        }
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

    template <typename T1, typename T2>
    inline pair<std::decay_t<T1>, std::decay_t<T2>> make_pair(T1&& aFirst, T2&& aSecond)
    {
        return pair<std::decay_t<T1>, std::decay_t<T2>>{ std::forward<T1>(aFirst), std::forward<T2>(aSecond) };
    }
}

namespace std
{
    template<typename First, typename Second>
    struct tuple_size<neolib::pair<First, Second>>
        : std::integral_constant<std::size_t, 2> {};

    template<std::size_t I, typename First, typename Second>
    struct tuple_element<I, neolib::pair<First, Second>>;

    template<typename First, typename Second>
    struct tuple_element<0, neolib::pair<First, Second>> {
        using type = First;
    };

    template<typename First, typename Second>
    struct tuple_element<1, neolib::pair<First, Second>> {
        using type = Second;
    };
}

namespace neolib
{
    template<std::size_t I, typename First, typename Second>
    constexpr auto& get(pair<First, Second>& p) noexcept {
        if constexpr (I == 0) return p.first();
        else                  return p.second();
    }

    template<std::size_t I, typename First, typename Second>
    constexpr auto& get(pair<First, Second> const& p) noexcept {
        if constexpr (I == 0) return p.first();
        else                  return p.second();
    }

    template<std::size_t I, typename First, typename Second>
    constexpr auto&& get(pair<First, Second>&& p) noexcept {
        if constexpr (I == 0) return std::move(p.first());
        else                  return std::move(p.second());
    }
}
