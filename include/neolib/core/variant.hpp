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
    template <typename... Types>
    class variant;
}

namespace std
{
    template <typename Type>
    struct variant_size<neolib::variant<Type>> : variant_size<std::variant<std::monostate, Type>> {};

    template <typename... Types>
    struct variant_size<neolib::variant<Types...>> : variant_size<std::variant<std::monostate, Types...>> {};

    template <typename Visitor, typename... Types>
    inline constexpr decltype(auto) visit(Visitor&& vis, neolib::variant<Types...>&& var)
    {
        return visit(std::forward<Visitor>(vis), static_cast<std::variant<std::monostate, Types...>&&>(std::move(var)));
    }
    
    template <typename Visitor, typename... Types>
    inline constexpr decltype(auto) visit(Visitor&& vis, neolib::variant<Types...> const& var)
    {
        return visit(std::forward<Visitor>(vis), static_cast<std::variant<std::monostate, Types...> const&>(var));
    }
    
    template <typename Visitor, typename... Types>
    inline constexpr decltype(auto) visit(Visitor&& vis, neolib::variant<Types...>& var)
    {
        return visit(std::forward<Visitor>(vis), static_cast<std::variant<std::monostate, Types...>&>(var));
    }

    template <typename T, typename... Types>
    inline constexpr bool holds_alternative(neolib::variant<Types...> const& var) noexcept
    {
        return holds_alternative<T>(static_cast<std::variant<std::monostate, Types...> const&>(var));
    }

    template<typename T, typename... Types>
    inline constexpr T& get(neolib::variant<Types...>& var)
    {
        return get<T>(static_cast<std::variant<std::monostate, Types...>&>(var));
    }

    template<typename T, typename... Types>
    inline constexpr T&& get(neolib::variant<Types...>&& var)
    {
        return get<T>(static_cast<std::variant<std::monostate, Types...> &&>(std::move(var)));
    }

    template<typename T, typename... Types>
    inline constexpr const T& get(const neolib::variant<Types...>& var)
    {
        return get<T>(static_cast<std::variant<std::monostate, Types...> const&>(var));
    }

    template<typename T, typename... Types>
    inline constexpr const T&& get(const neolib::variant<Types...>&& var)
    {
        return get<T>(static_cast<const std::variant<std::monostate, Types...>&&>(std::move(var)));
    }
}

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
    class variant : public reference_counted<i_variant<abstract_t<Types>...>>, public std::variant<std::monostate, Types...>
    {
        typedef variant<Types...> self_type;
        // types
    public:
        typedef i_variant<abstract_t<Types>...> abstract_type;
        typedef std::variant<std::monostate, Types...> std_type;
        // construction
    public:
        using std_type::std_type;
        variant(abstract_type const& aOther) :
            std_type{ static_cast<self_type const&>(aOther) } // todo: not plugin-safe
        {
        }
        variant(abstract_type&& aOther) :
            std_type{ static_cast<self_type&&>(aOther) } // todo: not plugin-safe
        {
        }
        template <typename T, typename = std::enable_if_t<!std::is_base_of_v<T, self_type>, sfinae>>
        variant(T const& aValue, std::enable_if_t<std::is_abstract_v<T>, sfinae> = {}) :
            std_type{ static_cast<from_abstract_t<T, Types...> const&>(aValue) }
        {
        }
        template <typename T, typename = std::enable_if_t<!std::is_base_of_v<T, self_type>, sfinae>>
        variant(T&& aValue, std::enable_if_t<std::is_abstract_v<T>, sfinae> = {}) :
            std_type{ static_cast<from_abstract_t<T, Types...>&&>(aValue) }
        {
        }
        // assignment
    public:
        using std_type::operator=;
        // meta
    public:
        std::size_t index() const override
        {
            return std_type::index();
        }
        // impl
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
    inline bool operator==(variant<Types...> const& lhs, variant<Types...> const& rhs)
    {
        return static_cast<std::variant<std::monostate, Types...> const&>(lhs) == static_cast<std::variant<std::monostate, Types...> const&>(rhs);
    }

    template <typename... Types>
    inline bool operator!=(variant<Types...> const& lhs, variant<Types...> const& rhs)
    {
        return static_cast<std::variant<std::monostate, Types...> const&>(lhs) != static_cast<std::variant<std::monostate, Types...> const&>(rhs);
    }

    template <typename... Types>
    inline bool operator<(variant<Types...> const& lhs, variant<Types...> const& rhs)
    {
        return static_cast<std::variant<std::monostate, Types...> const&>(lhs) < static_cast<std::variant<std::monostate, Types...> const&>(rhs);
    }

    template <typename... Types>
    inline bool operator<=(variant<Types...> const& lhs, variant<Types...> const& rhs)
    {
        return static_cast<std::variant<std::monostate, Types...> const&>(lhs) <= static_cast<std::variant<std::monostate, Types...> const&>(rhs);
    }

    template <typename... Types>
    inline bool operator>(variant<Types...> const& lhs, variant<Types...> const& rhs)
    {
        return static_cast<std::variant<std::monostate, Types...> const&>(lhs) > static_cast<std::variant<std::monostate, Types...> const&>(rhs);
    }

    template <typename... Types>
    inline bool operator>=(variant<Types...> const& lhs, variant<Types...> const& rhs)
    {
        return static_cast<std::variant<std::monostate, Types...> const&>(lhs) >= static_cast<std::variant<std::monostate, Types...> const&>(rhs);
    }

    template <typename... Types>
    inline bool operator==(variant<Types...> const& var, neolib::none_t)
    {
        return std::holds_alternative<std::monostate>(var);
    }

    template <typename... Types>
    inline bool operator==(neolib::none_t, variant<Types...> const& var)
    {
        return std::holds_alternative<std::monostate>(var);
    }

    template <typename... Types>
    inline bool operator!=(variant<Types...> const& var, neolib::none_t)
    {
        return !std::holds_alternative<std::monostate>(var);
    }

    template <typename... Types>
    inline bool operator!=(neolib::none_t, variant<Types...> const& var)
    {
        return !std::holds_alternative<std::monostate>(var);
    }

    // Deprecated, use std::get.
    template <typename T, typename Variant>
    inline auto& static_variant_cast(const Variant& var)
    {
        return std::get<std::decay_t<T>>(var);
    }

    // Deprecated, use std::get.
    template <typename T, typename Variant>
    inline auto& static_variant_cast(Variant& var)
    { 
        return std::get<std::decay_t<T>>(var);
    }

    struct bad_numeric_variant_cast : std::logic_error { bad_numeric_variant_cast() : std::logic_error{ "neolib::bad_numeric_variant_cast" } {} };

    template <typename T, typename Variant>
    inline T static_numeric_variant_cast(const Variant& var)
    {
        typedef T result_type;
        std::optional<result_type> result;
        visit([&result](auto&& source) 
        { 
            typedef std::remove_cv_t<std::remove_reference_t<decltype(source)>> source_type;
            if constexpr (std::is_arithmetic_v<source_type>)
                result = static_cast<result_type>(source); 
        }, var);
        if (result != std::nullopt)
            return *result;
        throw bad_numeric_variant_cast();
    }

    template <typename T, typename Variant>
    inline T static_numeric_variant_cast(Variant& var)
    {
        typedef T result_type;
        typedef std::remove_cv_t<std::remove_reference_t<result_type>> alternative_type;
        visit([&var](auto&& source)
        { 
            typedef std::remove_cv_t<std::remove_reference_t<decltype(source)>> source_type;
            if constexpr (std::is_arithmetic_v<source_type> && !std::is_same_v<alternative_type, source_type>)
                var = static_cast<alternative_type>(source);
        }, var);
        return static_variant_cast<T>(var);
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
}

// Deprecated, use std::get.
using neolib::static_variant_cast;
