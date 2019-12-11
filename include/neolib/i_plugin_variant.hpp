// i_plugin_variant.hpp - v1.0
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
#include <neolib/variadic.hpp>
#include <neolib/reference_counted.hpp>
#include <neolib/variant.hpp>
#include <neolib/i_enum.hpp>

namespace neolib
{
    namespace detail
    {
        namespace i_plugin_variant
        {
            template <typename Visitor, typename Variant>
            using funky_visit_t = std::function<void(const Visitor&, Variant&)>;
            template <typename Visitor, typename Variant>
            using funky_visit_list_t = std::vector<funky_visit_t<Visitor, Variant>>;
            template <typename Visitor, typename Variant>
            std::size_t funky_gen_visit(funky_visit_list_t<Visitor, Variant>& aList)
            {
                return aList.size();
            }
            template <typename Visitor, typename Variant, typename T, typename... Types>
            std::size_t funky_gen_visit(funky_visit_list_t<Visitor, Variant>& aList)
            {
                aList.push_back(
                    [](const Visitor& aVisitor, Variant& aThis)
                    {
                        typedef std::remove_const_t<std::remove_reference_t<T>> type;
                        aVisitor(aThis.get<type>());
                    });
                return funky_gen_visit<Visitor, Variant, Types...>(aList);
            }
        }
    }

    struct bad_variant_access : std::invalid_argument { bad_variant_access() : std::invalid_argument{ "neolib::bad_variant_access" } {} };
    struct variant_type_not_equality_comparable : std::invalid_argument { variant_type_not_equality_comparable() : std::invalid_argument{ "neolib::variant_type_not_equality_comparable" } {} };
    struct variant_type_not_less_than_comparable : std::invalid_argument { variant_type_not_less_than_comparable() : std::invalid_argument{ "neolib::variant_type_not_less_than_comparable" } {} };
    struct variant_type_not_convertible : std::invalid_argument { variant_type_not_convertible() : std::invalid_argument{ "neolib::variant_type_not_convertible" } {} };

    template <typename Id, typename... Types>
    class i_plugin_variant : public i_reference_counted
    {
        typedef i_plugin_variant<Id, Types...> self_type;
        template <typename, typename...>
        friend class plugin_variant;
        // exceptions
    public:
        // types
    public:
        typedef self_type abstract_type;
        typedef Id id_t;
        typedef std::size_t index_type;
        // construction/assignment
    public:
        ref_ptr<self_type> clone() const
        {
            return do_clone();
        }
        self_type& operator=(const self_type& aOther)
        {
            return do_assign(aOther.which(), aOther.data());
        }
        self_type& operator=(self_type&& aOther)
        {
            auto& result = do_move_assign(aOther.which(), aOther.data());
            aOther.clear();
            return result;
        }
        template <typename T>
        std::enable_if_t<!std::is_base_of_v<self_type, T>, self_type>& operator=(const T& aArgument)
        {
            return do_assign(static_cast<id_t>(variadic::index_v<T, Types...>), &aArgument);
        }
        template <typename T>
        std::enable_if_t<!std::is_base_of_v<self_type, std::remove_reference_t<T>>, self_type>& operator=(T&& aArgument)
        {
            return do_move_assign(static_cast<id_t>(variadic::no_reference_index_v<T, Types...>), &aArgument);
        }
        self_type& operator=(const none_t)
        {
            clear();
            return *this;
        }
        // comparison
    public:
        virtual bool operator==(const self_type& aRhs) const = 0;
        virtual bool operator<(const self_type& aRhs) const = 0;
        bool operator!=(const self_type& aRhs) const
        {
            return !(*this == aRhs);
        }
        bool operator==(const none_t) const
        {
            return empty();
        }
        bool operator!=(const none_t) const
        {
            return !empty();
        }
        // state
    public:
        virtual void clear() = 0;
        virtual id_t which() const = 0;
        virtual bool empty() const = 0;
        template <typename T>
        const T& get() const
        {
            if (which() == static_cast<id_t>(variadic::index_v<T, Types...>))
                return *static_cast<const T*>(data());
            throw bad_variant_access();
        }
        template <typename T>
        T& get()
        {
            if (which() == static_cast<id_t>(variadic::index_v<T, Types...>))
                return *static_cast<T*>(data());
            throw bad_variant_access();
        }
        // meta
    public:
        virtual const typename i_enum_t<Id>::enumerators_t& ids() const = 0;
        std::string which_as_string() const
        {
            ids().enumerators().find(which())->second.to_std_string();
        }
        // implementation
    private:
        virtual std::size_t index() const = 0;
        virtual const void* data() const = 0;
        virtual void* data() = 0;
        virtual self_type* do_clone() const = 0;
        virtual self_type& do_assign(id_t aType, const void* aData) = 0;
        virtual self_type& do_move_assign(id_t aType, void* aData) = 0;
    };

    namespace variant_visitors
    {
        template <typename Visitor, typename Id, typename... Types>
        inline void visit(Visitor aVisitor, const i_plugin_variant<Id, Types...>& aVariant)
        {
            typedef const i_plugin_variant<Id, Types...> variant_type;
            static detail::i_plugin_variant::funky_visit_list_t<Visitor, variant_type> funks;
            static auto const n = detail::i_plugin_variant::funky_gen_visit<Visitor, variant_type, Types...>(funks);
            if (static_cast<std::size_t>(aVariant.which()) < n)
                funks[static_cast<std::size_t>(aVariant.which())](aVisitor, aVariant);
            else
                throw std::bad_variant_access();
        }

        template <typename Visitor, typename Id, typename... Types>
        inline void visit(Visitor aVisitor, i_plugin_variant<Id, Types...>& aVariant)
        {
            typedef i_plugin_variant<Id, Types...> variant_type;
            static detail::i_plugin_variant::funky_visit_list_t<Visitor, variant_type> funks;
            static auto const n = detail::i_plugin_variant::funky_gen_visit<Visitor, variant_type, Types...>(funks);
            if (static_cast<std::size_t>(aVariant.which()) < n)
                funks[static_cast<std::size_t>(aVariant.which())](aVisitor, aVariant);
            else
                throw std::bad_variant_access();
        }
    }

    template <typename T, typename Id, typename... Types>
    inline T get_as(const i_plugin_variant<Id, Types...>& aVariant)
    {
        T result;
        variant_visitors::visit([&result](auto&& v)
        {
            if constexpr (std::is_convertible_v<decltype(v), T>)
                result = static_cast<T>(v);
            else
                throw variant_type_not_convertible();
        }, aVariant);
        return result;
    }
}

namespace std
{
    using neolib::variant_visitors::visit;
}

