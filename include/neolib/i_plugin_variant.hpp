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
#include <neolib/i_enum.hpp>

namespace neolib
{
    template <typename Id, typename... Types>
    class i_plugin_variant : public i_reference_counted
    {
        typedef i_plugin_variant<Id, Types...> self_type;
        template <typename, typename...>
        friend class plugin_variant;
        // exceptions
    public:
        struct bad_variant_access : std::invalid_argument { bad_variant_access() : std::invalid_argument{ "neolib::i_plugin_variant::bad_variant_access" } {} };
        struct type_not_less_than_comparable : std::invalid_argument { type_not_less_than_comparable() : std::invalid_argument{ "neolib::i_plugin_variant::type_not_less_than_comparable" } {} };
        // types
    public:
        typedef Id id_t;
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
        self_type& operator=(const T& aArgument)
        {
            return do_assign(static_cast<id_t>(variadic::index<T, Types...>::value), &aArgument);
        }
        template <typename T>
        self_type& operator=(T&& aArgument)
        {
            return do_move_assign(static_cast<id_t>(variadic::index<T, Types...>::value), &aArgument);
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
        const T& value() const
        {
            if (which() == static_cast<id_t>(variadic::index<T, Types...>::value))
                return *static_cast<const T*>(data());
            throw bad_variant_access();
        }
        template <typename T>
        T& value()
        {
            if (which() == static_cast<id_t>(variadic::index<T, Types...>::value))
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
}

