// zip_iterator.hpp
/*
 *  Copyright (c) 2018 Leigh Johnston.
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
#include <tuple>
#include <iterator>
#include <functional>

namespace neolib
{
    template <typename... Iterators>
    struct zip_iterator_traits
    {
        typedef std::tuple<Iterators...> iterators;
        typedef std::ptrdiff_t difference_type;
        typedef std::tuple<typename Iterators::value_type...> value_type;
        typedef std::tuple<typename Iterators::pointer...> pointer;
        typedef std::tuple<typename Iterators::reference...> reference;
        typedef typename std::iterator_traits<typename std::tuple_element<0, iterators>::type>::iterator_category iterator_category;
    };

    template <typename... Iterators>
    class zip_iterator
    {
    private:
        typedef zip_iterator<Iterators...> self_type;
        typedef zip_iterator_traits<Iterators...> traits;
        typedef typename traits::iterators iterators;
    public:
        typedef typename traits::difference_type difference_type;
        typedef typename traits::value_type value_type;
        typedef typename traits::pointer pointer;
        typedef typename traits::reference reference;
        typedef typename traits::iterator_category iterator_category;
    private:
        template <size_t... Is>
        struct index_sequence;
    public:
        template <typename... Iterators2>
        zip_iterator(const Iterators2&... iterators) : iContents{ iterators... }
        {
        }
    public:
        reference operator*()
        {
            return reference_helper(std::make_index_sequence<sizeof...(Iterators)>());
        }
        pointer operator->()
        {
            pointer result;
            set_pointer<0>(result);
            return result;
        }
        self_type& operator++()
        {
            increment<0>();
            return *this;
        }
        self_type operator++(int)
        {
            self_type old{ *this };
            increment<0>();
            return old;
        }
        self_type& operator--()
        {
            decrement<0>();
            return *this;
        }
        self_type operator--(int)
        {
            self_type old{ *this };
            decrement<0>();
            return old;
        }
        difference_type operator-(const self_type& aOther) const
        {
            return std::get<0>(contents()) - std::get<0>(aOther.contents());
        }
        self_type operator+(difference_type aAmount) const
        {
            self_type result{ *this };
            result.add<0>(aAmount);
            return result;
        }
        self_type operator-(difference_type aAmount) const
        {
            self_type result{ *this };
            result.subtract<0>(aAmount);
            return result;
        }
        bool operator<(const self_type& aOther) const
        {
            return std::get<0>(contents()) < std::get<0>(aOther.contents());
        }
        bool operator==(const self_type& aOther) const
        {
            return std::get<0>(contents()) == std::get<0>(aOther.contents());
        }
        bool operator!=(const self_type& aOther) const
        {
            return std::get<0>(contents()) != std::get<0>(aOther.contents());
        }
    public:
        const iterators& contents() const
        {
            return iContents;
        }
    private:
        template <std::size_t Index>
        void increment()
        {
            ++std::get<Index>(contents());
            if constexpr (Index < std::tuple_size<iterators>::value - 1)
                increment<Index + 1>();
        }
        template <std::size_t Index>
        void decrement()
        {
            --std::get<Index>(contents());
            if constexpr (Index < std::tuple_size<iterators>::value - 1)
                decrement<Index + 1>();
        }
        template <std::size_t Index>
        void add(difference_type aAmount)
        {
            std::get<Index>(contents()) += aAmount;
            if constexpr (Index < std::tuple_size<iterators>::value - 1)
                add<Index + 1>(aAmount);
        }
        template <std::size_t Index>
        void subtract(difference_type aAmount)
        {
            std::get<Index>(contents()) -= aAmount;
            if constexpr (Index < std::tuple_size<iterators>::value - 1)
                subtract<Index + 1>(aAmount);
        }
        template <std::size_t... Is>
        reference reference_helper(std::index_sequence<Is...>) 
        {
            return reference_helper_2(*std::get<Is>(contents())...);
        }
        template <typename... References>
        reference reference_helper_2(References&... aReferences)
        {
            return reference{ aReferences... };
        }
        template <std::size_t Index>
        void set_pointer(pointer& aResult)
        {
            std::get<Index>(aResult) = &std::get<Index>(contents());
            if constexpr (Index < std::tuple_size<iterators>::value - 1)
                set_pointer<Index + 1>(aResult);
        }
        iterators& contents()
        {
            return iContents;
        }
    private:
        iterators iContents;
    };

    template <typename... Iterators>
    inline zip_iterator<Iterators...> make_zip_iterator(Iterators&&... iterators)
    {
        return zip_iterator<Iterators...>(std::forward<Iterators>(iterators)...);
    }
}

namespace std
{
    template <typename... Iterators>
    struct iterator_traits<neolib::zip_iterator<Iterators...>>
    {
        typedef typename neolib::zip_iterator_traits<Iterators...>::difference_type difference_type;
        typedef typename neolib::zip_iterator_traits<Iterators...>::value_type value_type;
        typedef typename neolib::zip_iterator_traits<Iterators...>::pointer pointer;
        typedef typename neolib::zip_iterator_traits<Iterators...>::reference reference;
        typedef typename neolib::zip_iterator_traits<Iterators...>::iterator_category iterator_category;
    };
    
    template <typename... Iterators>
    inline void iter_swap(neolib::zip_iterator<Iterators...> a, neolib::zip_iterator<Iterators...> b)
    {
        auto temp = *a;
        *a = *b;
        *b = temp;
    }
}
