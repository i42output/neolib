// variant.hpp
/*
 *  Copyright (c) 2007, 2020 Leigh Johnston.
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
#include <type_traits>
#include <optional>
#include <variant>
#include <neolib/core/variadic.hpp>
#include <neolib/core/i_variant.hpp>

namespace neolib
{
    struct none_t : std::monostate {};
    const none_t none;

    template <typename AbstractT, typename Type>
    constexpr bool is_variant_convertible_v = std::is_base_of_v<AbstractT, Type> && std::is_abstract_v<AbstractT>;
    template <bool, typename AbstractT, typename... Rest>
    struct from_abstract;
    template <typename AbstractT, typename... Types>
    struct from_abstract_next;
    template <typename AbstractT, typename Type>
    struct from_abstract_next<AbstractT, Type> { typedef typename from_abstract<is_variant_convertible_v<AbstractT, Type>, AbstractT, Type>::result_type result_type; };
    template <typename AbstractT, typename Type, typename Rest>
    struct from_abstract_next<AbstractT, Type, Rest> { typedef typename from_abstract<is_variant_convertible_v<AbstractT, Type>, AbstractT, Type, Rest>::result_type result_type; };
    template <typename AbstractT, typename Type, typename... Rest>
    struct from_abstract_next<AbstractT, Type, Rest...> { typedef typename from_abstract<is_variant_convertible_v<AbstractT, Type>, AbstractT, Type, Rest...>::result_type result_type; };
    template <typename AbstractT, typename Type>
    struct from_abstract<true, AbstractT, Type> { typedef Type result_type; };
    template <typename AbstractT, typename Type, typename... Rest>
    struct from_abstract<true, AbstractT, Type, Rest...> { typedef Type result_type; };
    template <typename AbstractT, typename Type>
    struct from_abstract<false, AbstractT, Type> {};
    template <typename AbstractT, typename Type, typename... Rest>
    struct from_abstract<false, AbstractT, Type, Rest...> { typedef typename from_abstract_next<AbstractT, Rest...>::result_type result_type; };
    template <typename AbstractT, typename... Type>
    using from_abstract_t = typename from_abstract_next<AbstractT, Type...>::result_type;
        
    template <typename... Types>
    class variant : public i_variant<abstract_t<Types>...>, public std::variant<std::monostate, Types...>
    {
        typedef variant<Types...> self_type;
        typedef std::variant<std::monostate, Types...> base_type;
    public:
        typedef self_type abstract_type; // todo
    public:
        using base_type::base_type;
        template <typename T>
        variant(T const& aValue,  std::enable_if_t<std::is_abstract_v<T>, sfinae> = {}) :
            base_type{ static_cast<from_abstract_t<T, Types...> const&>(aValue) }
        {
        }
        template <typename T>
        variant(T&& aValue,  std::enable_if_t<std::is_abstract_v<T>, sfinae> = {}) :
            base_type{ static_cast<from_abstract_t<T, Types...>&&>(aValue) }
        {
        }
    public:
        using base_type::operator=;
    public:
        std::size_t index() const override
        {
            return base_type::index();
        }
    private:
        void const* ptr() const override
        {
            void const* result = nullptr;
            std::visit([&](auto&& arg)
            {
                result = &arg;
            }, *this);
            return result;
        }
        void* ptr() override
        {
            return const_cast<void*>(to_const(*this).ptr());
        }
    };

    template <typename... Types>
    inline bool operator==(const variant<Types...>& v, none_t)
    {
        return std::holds_alternative<std::monostate>(v);
    }

    template <typename... Types>
    inline bool operator!=(const variant<Types...>& v, none_t)
    {
        return !std::holds_alternative<std::monostate>(v);
    }

    // Deprecated, use std::get.
    template <typename T, typename Variant>
    inline auto& static_variant_cast(const Variant& aVariant)
    {
        return std::get<std::decay_t<T>>(aVariant);
    }

    // Deprecated, use std::get.
    template <typename T, typename Variant>
    inline auto& static_variant_cast(Variant& aVariant)
    { 
        return std::get<std::decay_t<T>>(aVariant);
    }

    struct bad_numeric_variant_cast : std::logic_error { bad_numeric_variant_cast() : std::logic_error{ "neolib::bad_numeric_variant_cast" } {} };

    template <typename T, typename Variant>
    inline T static_numeric_variant_cast(const Variant& aVariant)
    {
        typedef T result_type;
        std::optional<result_type> result;
        visit([&result](auto&& source) 
        { 
            typedef std::remove_cv_t<std::remove_reference_t<decltype(source)>> source_type;
            if constexpr (std::is_arithmetic_v<source_type>)
                result = static_cast<result_type>(source); 
        }, aVariant);
        if (result != std::nullopt)
            return *result;
        throw bad_numeric_variant_cast();
    }

    template <typename T, typename Variant>
    inline T static_numeric_variant_cast(Variant& aVariant)
    {
        typedef T result_type;
        typedef std::remove_cv_t<std::remove_reference_t<result_type>> alternative_type;
        visit([&aVariant](auto&& source)
        { 
            typedef std::remove_cv_t<std::remove_reference_t<decltype(source)>> source_type;
            if constexpr (std::is_arithmetic_v<source_type> && !std::is_same_v<alternative_type, source_type>)
                aVariant = static_cast<alternative_type>(source);
        }, aVariant);
        return static_variant_cast<T>(aVariant);
    }

    template<typename T, typename Variant, std::size_t index = 0>
    constexpr std::size_t variant_index_of()
    {
        if constexpr (index == std::variant_size_v<Variant>)
            return index - 1;
        else if constexpr (std::is_same_v<std::variant_alternative_t<index, Variant>, T>)
            return index - 1;
        else
            return variant_index_of<T, Variant, index + 1>();
    }

    namespace detail
    {
        template <typename T>
        struct is_neolib_variant {};
        template <typename Type>
        struct is_neolib_variant<std::variant<Type>> { static constexpr bool value = false; };
        template <typename... Types>
        struct is_neolib_variant<std::variant<Types...>> { static constexpr bool value = false; };
        template <typename Type>
        struct is_neolib_variant<variant<Type>> { static constexpr bool value = true; };
        template <typename... Types>
        struct is_neolib_variant<variant<Types...>> { static constexpr bool value = true; };
        template <typename... Types>
        constexpr bool is_neolib_variant_v = is_neolib_variant<Types...>::value;

        template <typename Variant, typename... Rest>
        struct any_neolib_variants { static constexpr bool value = is_neolib_variant_v<Variant> || any_neolib_variants<Rest...>::value; };
        template <typename Variant>
        struct any_neolib_variants<Variant> { static constexpr bool value = is_neolib_variant_v<Variant>; };
        template <typename... Variants>
        constexpr bool any_neolib_variants_v = any_neolib_variants<Variants...>::value;

        template <typename Variant, bool IsNeolibVariant, bool IsRvalueReference, bool IsConstReference>
        struct from_neolib_variant { typedef Variant type; };
        template <typename Variant>
        struct from_neolib_variant<Variant, true, true, false> { typedef typename Variant::base_type&& type; };
        template <typename Variant>
        struct from_neolib_variant<Variant, true, true, true> { typedef typename Variant::base_type&& type; };
        template <typename Variant>
        struct from_neolib_variant<Variant, true, false, true> { typedef typename Variant::base_type const& type; };
        template <typename Variant>
        struct from_neolib_variant<Variant, true, false, false> { typedef typename Variant::base_type& type; };
        template <typename Variant>
        using from_neolib_variant_t = typename from_neolib_variant<Variant, is_neolib_variant_v<Variant>, std::is_rvalue_reference_v<Variant>, std::is_const_v<std::remove_reference<Variant>>>::type;
    }
}

namespace std
{
    template <typename Type>
    struct variant_size<neolib::variant<Type>> : variant_size<std::variant<std::monostate, Type>> {};

    template <typename... Types>
    struct variant_size<neolib::variant<Types...>> : variant_size<std::variant<std::monostate, Types...>> {};

    template <typename R, typename Visitor, typename... Variants>
    constexpr R visit(Visitor&& vis, Variants&&... vars, std::enable_if_t<neolib::detail::any_neolib_variants_v<Variants...>, neolib::sfinae> = {})
    {
        return visit(vis, static_cast<neolib::detail::from_neolib_variant_t<Variants>>(vars)...);
    }
}

// Deprecated, use std::get.
using neolib::static_variant_cast;
