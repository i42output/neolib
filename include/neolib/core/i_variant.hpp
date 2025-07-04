// i_variant.hpp
/*
 *  Copyright (c) 2021 Leigh Johnston.
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
#include <functional>
#include <tuple>
#include <optional>
#include <variant>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/variadic.hpp>

namespace neolib
{
    template <typename... Types>
    class i_variant : public i_reference_counted
    {
    public:
        using abstract_type = i_variant;
    public:
        virtual ~i_variant() = default;
    public:
        virtual std::size_t index() const = 0;
    public:
        virtual void const* ptr() const = 0;
        virtual void* ptr() = 0;
    private:
        virtual i_variant& assign(std::size_t aIndex, void const* aPtr) = 0;
    public:
        i_variant& operator=(i_variant const& aOther)
        {
            return assign(aOther.index(), aOther.ptr());
        }
        template <typename T>
        std::enable_if_t<!std::is_base_of_v<i_variant, T>, i_variant>& operator=(T const& aValue)
        {
            return assign(variadic::index_v<abstract_t<T>, Types...> + 1, &aValue);
        }
        template <typename T>
        bool holds_alternative() const
        {
            return index() == variadic::index_v<abstract_t<T>, Types...> + 1;
        }
        template <typename T>
        T const* get_if() const
        {
            if (holds_alternative<T>())
                return static_cast<T const*>(ptr());
            return nullptr;
        }
        template <typename T>
        T* get_if()
        {
            if (holds_alternative<T>())
                return static_cast<T*>(ptr());
            return nullptr;
        }
        template <typename T>
        T const& get() const
        {
            if (holds_alternative<T>())
                return *get_if<T>();
            throw std::bad_variant_access();
        }
        template <typename T>
        T& get()
        {
            if (holds_alternative<T>())
                return *get_if<T>();
            throw std::bad_variant_access();
        }
    };

    namespace detail
    {
        // Compute a single return type R = common_type_t<invoke_result_t<Visitor, Types&>...>
        template <typename Visitor, typename... Types>
        struct visit_traits
        {
            using result_type = std::common_type_t<
                std::invoke_result_t<Visitor, Types&>...>;
        };

        // Non-const dispatch
        template <std::size_t I, typename Visitor, typename... Types>
        auto visit_impl(Visitor&& vis, i_variant<Types...>& v)
            -> typename visit_traits<Visitor, Types...>::result_type
        {
            constexpr std::size_t N = sizeof...(Types);
            using traits = visit_traits<Visitor, Types...>;
            using R = typename traits::result_type;

            if constexpr (I < N)
            {
                if (v.index() == I + 1)
                {
                    using T = std::tuple_element_t<I, std::tuple<Types...>>;
                    if constexpr (std::is_void_v<R>)
                    {
                        std::forward<Visitor>(vis)(v.template get<T>());
                        return;    // void
                    }
                    else
                    {
                        return std::forward<Visitor>(vis)(v.template get<T>());
                    }
                }
                else
                {
                    return visit_impl<I + 1>(std::forward<Visitor>(vis), v);
                }
            }
            else
            {
                throw std::bad_variant_access{};
            }
        }

        // Const dispatch
        template <std::size_t I, typename Visitor, typename... Types>
        auto visit_impl(Visitor&& vis, i_variant<Types...> const& v)
            -> typename visit_traits<Visitor, Types...>::result_type
        {
            constexpr std::size_t N = sizeof...(Types);
            using traits = visit_traits<Visitor, Types...>;
            using R = typename traits::result_type;

            if constexpr (I < N)
            {
                if (v.index() == I + 1)
                {
                    using T = std::tuple_element_t<I, std::tuple<Types...>>;
                    if constexpr (std::is_void_v<R>)
                    {
                        std::forward<Visitor>(vis)(v.template get<T>());
                        return;
                    }
                    else
                    {
                        return std::forward<Visitor>(vis)(v.template get<T>());
                    }
                }
                else
                {
                    return visit_impl<I + 1>(std::forward<Visitor>(vis), v);
                }
            }
            else
            {
                throw std::bad_variant_access{};
            }
        }
    }

    // Public API: non-const
    template <typename Visitor, typename... Types>
    auto visit(Visitor&& vis, i_variant<Types...>& v)
        -> typename detail::visit_traits<Visitor, Types...>::result_type
    {
        return detail::visit_impl<0>(std::forward<Visitor>(vis), v);
    }

    // Public API: const
    template <typename Visitor, typename... Types>
    auto visit(Visitor&& vis, i_variant<Types...> const& v)
        -> typename detail::visit_traits<Visitor, Types...>::result_type
    {
        return detail::visit_impl<0>(std::forward<Visitor>(vis), v);
    }
}
