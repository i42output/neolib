// plugin_variant.hpp
/*
 *  Copyright (c) 2019, 2020 Leigh Johnston.
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
#include <boost/type_traits.hpp>
#include <neolib/core/enum.hpp>
#include <neolib/core/variant.hpp>
#include <neolib/plugin/i_plugin_variant.hpp>

namespace neolib
{
    namespace detail
    {
        namespace plugin_variant
        {
            template <typename V>
            using funky_assign_t = std::function<void(V&, const void*)>;
            template <typename V>
            using funky_assign_list_t = std::vector<funky_assign_t<V>> ;
            template <typename V>
            std::size_t funky_gen_assign(funky_assign_list_t<V>& aList)
            {
                return aList.size();
            }
            template <typename V, typename T, typename... Types>
            std::size_t funky_gen_assign(funky_assign_list_t<V>& aList)
            {
                aList.push_back(
                    [](V& aThis, const void* aData)
                    {
                        typedef std::decay_t<T> type;
                        typedef abstract_t<type> assign_type;
                        aThis = *static_cast<const assign_type*>(aData);
                    });
                return funky_gen_assign<V, Types...>(aList);
            }

            template <typename V>
            using funky_move_assign_t = std::function<void(V&, void*)>;
            template <typename V>
            using funky_move_assign_list_t = std::vector<funky_move_assign_t<V>> ;
            template <typename V>
            std::size_t funky_gen_move_assign(funky_move_assign_list_t<V>& aList)
            {
                return aList.size();
            }
            template <typename V, typename T, typename... Types>
            std::size_t funky_gen_move_assign(funky_move_assign_list_t<V>& aList)
            {
                aList.push_back(
                    [](V& aThis, void* aData)
                    {
                        typedef std::decay_t<T> type;
                        typedef abstract_t<type> move_assign_type;
                        aThis = static_cast<move_assign_type&&>(*static_cast<move_assign_type*>(aData));
                    });
                return funky_gen_move_assign<V, Types...>(aList);
            }
        }
    }

    template <typename Id, typename... Types>
    class plugin_variant :
        public reference_counted<i_plugin_variant<Id, abstract_t<Types>...>>,
        public std::variant<std::monostate, Types...>
    {
        typedef reference_counted<i_plugin_variant<Id, abstract_t<Types>...>> base_type;
        // types
    public:
        typedef i_plugin_variant<Id, abstract_t<Types>...> abstract_type;
        using typename base_type::id_t;
        using typename base_type::index_type;
        typedef std::variant<std::monostate, Types...> variant_type;
        // construction/assignment
    public:
        using variant_type::variant_type;
        plugin_variant(const plugin_variant& aOther)
        {
            if (!aOther.empty())
                do_assign(aOther.which(), aOther.data());
        }
        plugin_variant(plugin_variant&& aOther) noexcept
        {
            if (!aOther.empty())
                do_move_assign(aOther.which(), aOther.data());
        }
        plugin_variant(const abstract_type& aOther)
        {
            if (!aOther.empty())
                do_assign(aOther.which(), aOther.data());
        }
        plugin_variant(abstract_type&& aOther) noexcept
        {
            if (!aOther.empty())
                do_move_assign(aOther.which(), aOther.data());
        }
        plugin_variant& operator=(const plugin_variant& aOther)
        {
            variant_type::operator=(aOther);
            return *this;
        }
        plugin_variant& operator=(plugin_variant&& aOther) noexcept
        {
            variant_type::operator=(std::move(aOther));
            return *this;
        }
        plugin_variant& operator=(none_t)
        {
            variant_type::operator=(std::monostate{});
            return *this;
        }
        using abstract_type::operator=;
        // comparison
    public:
        bool operator==(none_t) const
        {
            return std::holds_alternative<std::monostate>(*this);
        }
        bool operator==(const abstract_type& that) const final
        {
            if (index() != that.index())
                return false;
            bool result = false;
            std::visit([this, &that, &result](auto&& v)
            {
                typedef std::decay_t<decltype(v)> type;
                typedef abstract_t<type> comparison_type;
                if constexpr (boost::has_equal_to<comparison_type, comparison_type>::value)
                    result = (*static_cast<const comparison_type*>(data()) == *static_cast<const comparison_type*>(that.data()));
                else
                    throw variant_type_not_equality_comparable();
            }, as_std_variant());
            return result;
        }
        bool operator<(const abstract_type& that) const final
        {
            if (index() != that.index())
                return index() < that.index();
            if (index() == 0u)
                return false;
            bool result = false;
            std::visit([this, &that, &result](auto&& v)
            {
                typedef std::decay_t<decltype(v)> type;
                typedef abstract_t<type> comparison_type;
                if constexpr (boost::has_less<comparison_type, comparison_type>::value)
                    result = (*static_cast<const comparison_type*>(data()) < *static_cast<const comparison_type*>(that.data()));
                else
                    throw variant_type_not_less_than_comparable();
            },  as_std_variant());
            return result;
        }
        std::partial_ordering operator<=>(const plugin_variant& that) const
        {
            if (*this == that)
                return std::partial_ordering::equivalent;
            else if (*this < that)
                return std::partial_ordering::less;
            else
                return std::partial_ordering::greater;
        }
        // state
    public:
        void clear() final
        {
            variant_type::operator=(std::monostate{});
        }
        id_t which() const final
        {
            if (!empty())
                return static_cast<id_t>(index() - 1u);
            throw bad_variant_access();
        }
        bool empty() const final
        {
            return std::holds_alternative<std::monostate>(*this);
        }
        // meta
    public:
        const variant_type& as_std_variant() const
        {
            return *this;
        }
        variant_type& as_std_variant()
        {
            return *this;
        }
        const typename i_enum_t<Id>::enumerators_t& ids() const final
        {
            return iEnum.enumerators();
        }
        std::string which_as_string() const
        {
            return ids().enumerators().find(which())->second.to_std_string();
        }
        // implementation
    private:
        std::size_t index() const final
        {
            return variant_type::index();
        }
        const void* data() const final
        {
            const void* result = nullptr;
            std::visit([&result](auto&& v) { result = &v; }, as_std_variant());
            return result;
        }
        void* data() final
        {
            void* result = nullptr;
            std::visit([&result](auto&& v) { result = &v; }, as_std_variant());
            return result;
        }
        abstract_type* do_clone() const final
        {
            return new plugin_variant{ *this };
        }
        abstract_type& do_assign(id_t aType, const void* aData) final
        {
            static detail::plugin_variant::funky_assign_list_t<variant_type> funks;
            static auto const n = detail::plugin_variant::funky_gen_assign<variant_type, Types...>(funks);
            if (static_cast<std::size_t>(aType) < n)
                funks[static_cast<std::size_t>(aType)](*this, aData);
            else
                throw std::bad_variant_access();
            return *this;
        }
        abstract_type& do_move_assign(id_t aType, void* aData) final
        {
            static detail::plugin_variant::funky_move_assign_list_t<variant_type> funks;
            static auto const n = detail::plugin_variant::funky_gen_move_assign<variant_type, Types...>(funks);
            if (static_cast<std::size_t>(aType) < n)
                funks[static_cast<std::size_t>(aType)](*this, aData);
            else
                throw std::bad_variant_access();
            return *this;
        }
        // state
    private:
        const enum_t<id_t> iEnum;
    };
}

namespace std
{
    template <typename Visitor, typename Id, typename... Types>
    auto visit(Visitor&& vis, const neolib::plugin_variant<Id, Types...>& var)
    {
        return std::visit(std::forward<Visitor>(vis), var.as_std_variant());
    }

    template <typename Visitor, typename Id, typename... Types>
    auto visit(Visitor&& vis, neolib::plugin_variant<Id, Types...>& var)
    {
        return std::visit(std::forward<Visitor>(vis), var.as_std_variant());
    }
}

