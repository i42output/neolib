// plugin_variant.hpp
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
#include <type_traits>
#include <boost/type_traits.hpp>
#include <neolib/enum.hpp>
#include <neolib/variant.hpp>
#include <neolib/i_plugin_variant.hpp>

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
                        typedef std::remove_const_t<std::remove_reference_t<T>> type;
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
                        typedef std::remove_const_t<std::remove_reference_t<T>> type;
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
        public variant<Types...>
    {
        typedef plugin_variant<Id, Types...> self_type;
        typedef reference_counted<i_plugin_variant<Id, abstract_t<Types>...>> base_type;
        // types
    public:
        typedef i_plugin_variant<Id, abstract_t<Types>...> abstract_type;
        using typename base_type::id_t;
        using typename base_type::index_type;
        typedef variant<Types...> variant_type;
        // construction/assignment
    public:
        using variant_type::variant_type;
        plugin_variant(const self_type& aOther)
        {
            do_assign(aOther.which(), aOther.data());
        }
        plugin_variant(self_type&& aOther)
        {
            do_move_assign(aOther.which(), aOther.data());
        }
        plugin_variant(const abstract_type& aOther)
        {
            do_assign(aOther.which(), aOther.data());
        }
        plugin_variant(abstract_type&& aOther)
        {
            do_move_assign(aOther.which(), aOther.data());
        }
        self_type& operator=(const self_type& aOther)
        {
            return static_cast<self_type&>(abstract_type::operator=(aOther));
        }
        self_type& operator=(self_type&& aOther)
        {
            return static_cast<self_type&>(abstract_type::operator=(aOther));
        }
        using abstract_type::operator=;
        // comparison
    public:
        using variant_type::operator==;
        using variant_type::operator!=;
        bool operator==(const abstract_type& aRhs) const override
        {
            if (index() != index())
                return false;
            bool result = false;
            visit([this, &aRhs, &result](auto&& v)
            {
                typedef std::remove_const_t<std::remove_reference_t<decltype(v)>> type;
                typedef abstract_t<type> comparison_type;
                if constexpr (boost::has_equal_to<comparison_type, comparison_type>::value)
                    result = (*static_cast<const comparison_type*>(data()) == *static_cast<const comparison_type*>(aRhs.data()));
                else
                    throw variant_type_not_equality_comparable();
            }, *this);
            return result;
        }
        bool operator!=(const abstract_type& aRhs) const
        {
            return !(*this == aRhs);
        }
        bool operator<(const abstract_type& aRhs) const override
        {
            if (index() != aRhs.index())
                return index() < aRhs.index();
            if (index() == 0u)
                return false;
            bool result = false;
            visit([this, &aRhs, &result](auto&& v)
            {
                typedef std::remove_const_t<std::remove_reference_t<decltype(v)>> type;
                typedef abstract_t<type> comparison_type;
                if constexpr (boost::has_less<comparison_type, comparison_type>::value)
                    result = (*static_cast<const comparison_type*>(data()) < *static_cast<const comparison_type*>(aRhs.data()));
                else
                    throw variant_type_not_less_than_comparable();
            }, *this);
            return result;
        }
        // state
    public:
        void clear() override
        {
            variant_type::operator=(none);
        }
        id_t which() const override
        {
            if (!empty())
                return static_cast<id_t>(index() - 1u);
            throw bad_variant_access();
        }
        bool empty() const override
        {
            return variant_type::operator==(none);
        }
        // meta
    public:
        const typename i_enum_t<Id>::enumerators_t& ids() const override
        {
            return iEnum.enumerators();
        }
        std::string which_as_string() const
        {
            ids().enumerators().find(which())->second.to_std_string();
        }
        // implementation
    private:
        std::size_t index() const override
        {
            return variant_type::index();
        }
        const void* data() const override
        {
            const void* result = nullptr;
            visit([&result](auto&& v) { result = &v; }, *this);
            return result;
        }
        void* data() override
        {
            void* result = nullptr;
            visit([&result](auto&& v) { result = &v; }, *this);
            return result;
        }
        abstract_type* do_clone() const override
        {
            return new self_type{ *this };
        }
        abstract_type& do_assign(id_t aType, const void* aData) override
        {
            static detail::plugin_variant::funky_assign_list_t<variant_type> funks;
            static auto const n = detail::plugin_variant::funky_gen_assign<variant_type, Types...>(funks);
            if (static_cast<std::size_t>(aType) < n)
                funks[static_cast<std::size_t>(aType)](*this, aData);
            else
                throw std::bad_variant_access();
            return *this;
        }
        abstract_type& do_move_assign(id_t aType, void* aData) override
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

    namespace variant_visitors
    {
        template <typename Visitor, typename Id, typename... Types>
        auto visit(Visitor&& vis, const neolib::plugin_variant<Id, Types...>& var)
        {
            return std::visit(std::forward<Visitor>(vis), var.for_visitor());
        }

        template <typename Visitor, typename Id, typename... Types>
        auto visit(Visitor&& vis, neolib::plugin_variant<Id, Types...>& var)
        {
            return std::visit(std::forward<Visitor>(vis), var.for_visitor());
        }
    }

    using namespace variant_visitors;
}

namespace std
{
    template <typename Id, typename... Types>
    struct variant_size<neolib::plugin_variant<Id, Types...>>
        : std::integral_constant<std::size_t, sizeof...(Types)> { };

    template <size_t I, typename Id, class... Types>
    struct variant_alternative<I, neolib::plugin_variant<Id, Types...>>
        { typedef typename std::variant_alternative<I, typename neolib::plugin_variant<Id, Types...>::variant_type>::type type; };

    using neolib::variant_visitors::visit;
}