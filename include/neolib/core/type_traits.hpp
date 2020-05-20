// type_traits.hpp
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
#include <istream>
#include <ostream>

namespace neolib
{
    namespace type_traits_detail
    {
        // This type traits is type streamable code is a modified version of http://stackoverflow.com/questions/4434569/is-it-possible-to-use-sfinae-templates-to-check-if-an-operator-exists

        typedef char yes;
        typedef char(&no)[2];

        struct anyx { template <class T> anyx(const T &); };

        no operator << (const anyx &, const anyx &);
        no operator >> (const anyx &, const anyx &);

        template <class T> yes check(T const&);
        no check(no);

    }

    template <typename T, typename StreamType = std::istream>
    struct has_loading_support {
        static StreamType & stream;
        static T & x;
        static constexpr bool value = sizeof(type_traits_detail::check(stream >> x)) == sizeof(type_traits_detail::yes);
    };

    template <typename T, typename StreamType = std::ostream>
    struct has_saving_support {
        static StreamType & stream;
        static T & x;
        static constexpr bool value = sizeof(type_traits_detail::check(stream << x)) == sizeof(type_traits_detail::yes);
    };

    template <typename T, typename LoadingStreamType = std::istream, typename SavingStreamType = std::ostream>
    struct has_stream_operators {
        static constexpr bool can_load = has_loading_support<LoadingStreamType, T>::value;
        static constexpr bool can_save = has_saving_support<SavingStreamType, T>::value;
        static constexpr bool value = can_load && can_save;
    };

    template <typename ConstType, typename NonConstType, bool IsConst>
    struct const_selector;
    template <typename ConstType, typename NonConstType>
    struct const_selector<ConstType, NonConstType, true> { typedef ConstType type; };
    template <typename ConstType, typename NonConstType>
    struct const_selector<ConstType, NonConstType, false> { typedef NonConstType type; };

    template<typename ConstType, typename NonConstType, typename Pointer>
    struct const_selector_from_pointer : const_selector<ConstType, NonConstType, std::is_const<typename std::remove_pointer<Pointer>::type>::value> {};
}
