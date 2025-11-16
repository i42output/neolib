// json.hpp
/*
 *  NoFussJSON v1.0
 *
 *  Copyright (c) 2018, 2020 Leigh Johnston.
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
#include <cstddef>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <type_traits>
#include <istream>
#include <ostream>
#include <utility>
#include <memory>
#include <exception>
#include <optional>
#include <variant>
#include <boost/functional/hash.hpp>
#include <neolib/core/allocator.hpp>
#include <neolib/core/quick_string.hpp>

namespace neolib
{
    struct json_error : std::runtime_error { json_error(const std::string& aReason) : std::runtime_error{ aReason } {} };
    struct json_path_not_found : std::runtime_error { json_path_not_found(const std::string& aPath) : std::runtime_error{ "JSON path not found: " + aPath } {} };
    struct json_no_owning_node : std::logic_error { json_no_owning_node() : std::logic_error{ "JSON no owning node" } {} };

    enum class json_syntax
    {
        Standard,
        StandardNoKeywords,
        Relaxed,
        Functional
    };

    enum class json_encoding
    {
        Utf8,
        Utf16LE,
        Utf16BE,
        Utf32LE,
        Utf32BE
    };

    namespace json_detail
    {
        template <std::size_t CharSize>
        struct default_encoding_helper;
        template <> struct default_encoding_helper<1> { static const json_encoding DEFAULT_ENCODING = json_encoding::Utf8; };
        template <> struct default_encoding_helper<2> { static const json_encoding DEFAULT_ENCODING = json_encoding::Utf16LE; };
        template <> struct default_encoding_helper<4> { static const json_encoding DEFAULT_ENCODING = json_encoding::Utf32LE; };

        template <typename CharT>
        struct default_encoding    { static const json_encoding DEFAULT_ENCODING = default_encoding_helper<sizeof(CharT)>::DEFAULT_ENCODING; };
    }

    enum class json_type
    {
        Unknown,
        Object,
        Array,
        Double,
        Int64,
        Uint64,
        Int,
        Uint,
        String,
        Bool,
        Null,
        Keyword
    };

    inline std::string to_string(json_type aType)
    {
        switch (aType)
        {
        default:
        case json_type::Unknown:
            return "Unknown";
        case json_type::Object:
            return "Object";
        case json_type::Array:
            return "Array";
        case json_type::Double:
            return "Double";
        case json_type::Int64:
            return "Int64";
        case json_type::Uint64:
            return "Uint64";
        case json_type::Int:
            return "Int";
        case json_type::Uint:
            return "Uint";
        case json_type::String:
            return "String";
        case json_type::Bool:
            return "Bool";
        case json_type::Null:
            return "Null";
        case json_type::Keyword:
            return "Keyword";
        }
    }

    struct json_document_source_location
    {
        uint32_t line;
        uint32_t column;
    };

    namespace json_detail
    {
        template <typename T>
        class basic_json_node
        {
            template <json_syntax Syntax, typename Alloc , typename CharT, typename Traits, typename CharAlloc>
            friend class basic_json_value;
        public:
            using json_value = T;
            using value_type = typename json_value::value_type;
        private:
            using value_allocator = typename json_value::value_allocator;
        public:
            basic_json_node() : 
                iParent{ nullptr },
                iPrevious{ nullptr }, 
                iNext{ nullptr },
                iFirstChild{ nullptr },
                iLastChild{ nullptr }
            {
            }
            basic_json_node(json_value& aParent) :
                iParent{ &aParent },
                iPrevious{ nullptr },
                iNext{ nullptr },
                iFirstChild{ nullptr },
                iLastChild{ nullptr }
            {
            }
            ~basic_json_node()
            {
                while (iLastChild != nullptr)
                    destruct_child(iLastChild);
            }
        public:
            json_value* buy_child(json_value& aParent, const value_type& aValue)
            {
                return construct_child(allocate_child(), aParent, aValue);
            }
            json_value* buy_child(json_value& aParent, value_type&& aValue)
            {
                return construct_child(allocate_child(), aParent, std::move(aValue));
            }
        public:
            void unlink()
            {
                if (iPrevious != nullptr)
                    iPrevious->iNode.iNext = iNext;
                if (iNext != nullptr)
                    iNext->iNode.iPrevious = iPrevious;
            }
            bool has_parent() const
            {
                return iParent != nullptr;
            }
            const json_value* parent() const
            {
                return iParent;
            }
            json_value* parent()
            {
                return iParent;
            }
            bool has_children() const
            {
                return iFirstChild != nullptr;
            }
            const json_value* first_child() const
            {
                return iFirstChild;
            }
            json_value* first_child()
            {
                return iFirstChild;
            }
            const json_value* last_child() const
            {
                return iLastChild;
            }
            json_value* last_child()
            {
                return iLastChild;
            }
            bool is_last_sibling() const
            {
                return iNext == nullptr;
            }
            const json_value* previous_sibling() const
            {
                return iPrevious;
            }
            json_value* previous_sibling()
            {
                return iPrevious;
            }
            void set_previous_sibling(json_value* aPrevious)
            {
                iPrevious = aPrevious;
            }
            const json_value* next_sibling() const
            {
                return iNext;
            }
            json_value* next_sibling()
            {
                return iNext;
            }
            void set_next_sibling(json_value* aNext)
            {
                iNext = aNext;
            }
            const json_value* next_parent_sibling() const
            {
                auto tryParent = iParent;
                if (tryParent == nullptr)
                    return nullptr;
                while (tryParent->has_parent() && tryParent->is_last_sibling())
                    tryParent = &tryParent->parent();
                return tryParent->next_sibling();
            }
            json_value* next_parent_sibling()
            {
                return const_cast<json_value*>(to_const(*this).next_parent_sibling());
            }
        private:
            json_value* allocate_child()
            {
                return std::allocator_traits<value_allocator>::allocate(allocator(), 1);
            }
            void deallocate_child(json_value* aAddress)
            {
                std::allocator_traits<value_allocator>::deallocate(allocator(), aAddress, 1);
            }
            template <typename... Args>
            json_value* construct_child(json_value* aAddress, json_value& aParent, Args&&... aArguments)
            {
                std::allocator_traits<value_allocator>::construct(allocator(), aAddress, aParent, std::forward<Args>(aArguments)...);
                json_value& child = *aAddress;
                if (iLastChild == nullptr)
                {
                    iFirstChild = &child;
                    iLastChild = &child;
                }
                else
                {
                    iLastChild->iNode.iNext = &child;
                    child.iNode.iPrevious = iLastChild;
                    iLastChild = &child;
                }
                return &child;
            }
            void destruct_child(json_value* aAddress)
            {
                json_value& child = *aAddress;
                if (child.iNode.iPrevious != nullptr)
                    child.iNode.iPrevious->iNode.iNext = child.iNode.iNext;
                if (child.iNode.iNext != nullptr)
                    child.iNode.iNext->iNode.iPrevious = child.iNode.iPrevious;
                if (iLastChild == &child)
                    iLastChild = child.iNode.iPrevious;
                if (iFirstChild == &child)
                    iFirstChild = child.iNode.iNext;
                child.~json_value();
                deallocate_child(aAddress);
            }
        private:
            static value_allocator& allocator()
            {
                static value_allocator sAllocator;
                return sAllocator;
            }
            json_value* iParent;
            json_value* iPrevious;
            json_value* iNext;
            json_value* iFirstChild;
            json_value* iLastChild;
        };
    }

    template <typename T>
    class basic_json_object
    {
        template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
        friend class basic_json;
        template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
        friend class basic_json_value;
    public:
        using json_value = T;
        using value_type = typename json_value::value_type;
        using json_string = typename json_value::json_string;
    private:
        using allocator_type = typename json_value::value_allocator;
        using dictionary_type = std::unordered_multimap<
            json_string, 
            json_value*, 
            boost::hash<json_string>, 
            std::equal_to<json_string>, 
            typename std::allocator_traits<allocator_type>::template rebind_alloc<std::pair<const json_string, json_value*>>>;
    public:
        basic_json_object() :
            iContents{ nullptr }
        {
        }
        basic_json_object(json_value& aOwner) :
            iContents{ &aOwner }
        {
        }
    public:
        bool has(const json_string& aKey) const
        {
            return cache().find(aKey) != cache().end();
        }
        const json_value& at(const json_string& aKey) const
        {
            auto existing = cache().find(aKey);
            if (existing != cache().end())
                return *existing->second;
            throw std::out_of_range("neolib::basic_json_object::at: key not found");
        }
        json_value& at(const json_string& aKey)
        {
            return const_cast<json_value&>(to_const(*this).at(aKey));
        }
        template <typename U>
        std::enable_if_t<!std::is_arithmetic_v<U>, const U&> at_or(const json_string& aKey, const U& aDefault) const
        {
            if (has(aKey))
                return at(aKey).template as<U>();
            return aDefault;
        }
        template <typename U>
        std::enable_if_t<!std::is_arithmetic_v<U>, U&> at_or(const json_string& aKey, U& aDefault)
        {
            if (has(aKey))
                return at(aKey).template as<U>();
            return aDefault;
        }
        template <typename U>
        std::enable_if_t<std::is_arithmetic_v<U>, U> at_or(const json_string& aKey, const U& aDefault) const
        {
            if (has(aKey))
                return at(aKey).template as<U>();
            return aDefault;
        }
        template <typename U>
        std::enable_if_t<std::is_arithmetic_v<U>, U&> at_or(const json_string& aKey, U& aDefault)
        {
            if (has(aKey))
                return at(aKey).template as<U>();
            return aDefault;
        }
        json_value& operator[](const json_string& aKey)
        {
            auto existing = cache().find(aKey);
            if (existing != cache().end())
                return *existing->second;
            auto& newChild = contents().emplace_back(value_type{});
            newChild.set_name(aKey.to_std_string());
            cache().emplace(newChild.name(), &newChild);
            return newChild;
        }
    public:
        json_value& contents() const
        {
            if (iContents == nullptr)
                throw json_no_owning_node();
            return *iContents;
        }
        void set_contents(json_value& aOwner)
        {
            iContents = &aOwner;
        }
    private:
        const dictionary_type& cache() const
        {
            if (iLazyDictionary != nullptr)
                return *iLazyDictionary;
            iLazyDictionary = std::make_unique<dictionary_type>(); // todo: use allocator_type
            for (auto i = contents().begin(); i != contents().end(); ++i)
                iLazyDictionary->emplace(i.value().name(), &i.value());
            return *iLazyDictionary;
        }
        dictionary_type& cache()
        {
            return const_cast<dictionary_type&>(to_const(*this).cache());
        }
    private:
        json_value* iContents;
        mutable std::unique_ptr<dictionary_type> iLazyDictionary;
    };

    template <typename T>
    class basic_json_array
    {
        template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
        friend class basic_json;
        template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
        friend class basic_json_value;
    public:
        using json_value = T;
        using value_type = typename json_value::value_type;
        using json_string = typename json_value::json_string;
    private:
        using allocator_type = typename json_value::value_allocator;
        using array_type = std::vector<json_value*>;
    public:
        basic_json_array() :
            iContents{ nullptr }
        {
        }
        basic_json_array(json_value& aOwner) :
            iContents{ &aOwner }
        {
        }
    public:
        typename array_type::const_iterator cbegin() const
        {
            return cache().begin();
        }
        typename array_type::const_iterator cend() const
        {
            return cache().end();
        }
        typename array_type::const_iterator begin() const
        {
            return cache().begin();
        }
        typename array_type::const_iterator end() const
        {
            return cache().end();
        }
        typename array_type::iterator begin()
        {
            return cache().begin();
        }
        typename array_type::iterator end()
        {
            return cache().end();
        }
        const json_value& operator[](std::size_t aIndex) const
        {
            return *cache().at(aIndex);
        }
    public:
        json_value& push_back(const value_type& aValue)
        {
            auto& newChild = contents().emplace_back(aValue);
            cache().emplace_back(&newChild);
            return back();
        }
        json_value& push_back(value_type&& aValue)
        {
            auto& newChild = contents().emplace_back(std::move(aValue));
            cache().emplace_back(&newChild);
            return back();
        }
        json_value& back()
        {
            return *cache().back();
        }
        json_value& operator[](std::size_t aIndex)
        {
            return *cache().at(aIndex);
        }
    public:
        json_value& contents() const
        {
            if (iContents == nullptr)
                throw json_no_owning_node();
            return *iContents;
        }
        void set_contents(json_value& aOwner)
        {
            iContents = &aOwner;
        }
    private:
        const array_type& cache() const
        {
            if (iLazyArray != nullptr)
                return *iLazyArray;
            iLazyArray = std::make_unique<array_type>(); // todo: use allocator_type
            for (auto& e : contents())
                iLazyArray->emplace_back(&e);
            return *iLazyArray;
        }
        array_type& cache()
        {
            return const_cast<array_type&>(to_const(*this).cache());
        }
    private:
        json_value* iContents;
        mutable std::unique_ptr<array_type> iLazyArray;
    };

    template <typename T>
    struct basic_json_keyword
    {
    public:
        using json_value = T;
        using json_string = typename json_value::json_string;
    public:
        json_string text;
    };

    template <typename T>
    struct basic_json_null
    {
    public:
        using json_value = T;
    public:
        bool operator==(std::nullptr_t) const { return true; }
        bool operator!=(std::nullptr_t) const { return false; }
    };

    template <json_syntax Syntax = json_syntax::Standard, typename Alloc = std::allocator<json_type>, typename CharT = char, typename Traits = std::char_traits<CharT>, typename CharAlloc = std::allocator<CharT>>
    class basic_json;
        
    template <json_syntax Syntax = json_syntax::Standard, typename Alloc = std::allocator<json_type>, typename CharT = char, typename Traits = std::char_traits<CharT>, typename CharAlloc = std::allocator<CharT>>
    class basic_json_value
    {
        friend class basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>;
        template <typename T>
        friend class json_detail::basic_json_node;
    public:
        struct no_name : std::logic_error { no_name() : std::logic_error("neolib::basic_json_value::no_name") {} };
        struct bad_conversion : std::runtime_error { bad_conversion() : std::runtime_error("neolib::basic_json_value::bad_conversion") {} };
    public:
        static constexpr json_syntax syntax = Syntax;
        using allocator_type = Alloc;
        using character_type = CharT;
        using character_traits_type = Traits;
        using character_allocator_type = CharAlloc;
    public:
        using node_value_type = basic_json_value;
    public:
        using value_allocator = typename std::allocator_traits<allocator_type>::template rebind_alloc<basic_json_value>;
    public:
        using const_pointer = const basic_json_value*;
        using pointer = basic_json_value*;
        using const_reference = const basic_json_value&;
        using reference = basic_json_value&;
    private:
        template <typename IteratorTraits>
        class iterator_base;
    public:
        class const_iterator;
        class iterator;
    public:
        using json_string = basic_quick_string<character_type, character_traits_type, character_allocator_type>;
        using json_object = basic_json_object<basic_json_value>;
        using json_array = basic_json_array<basic_json_value>;
        using json_double = double;
        using json_int64 = std::int64_t;
        using json_uint64 = std::uint64_t;
        using json_int = std::int32_t;
        using json_uint = std::uint32_t;
        using json_bool = bool;
        using json_null = basic_json_null<basic_json_value>;
        using json_keyword = basic_json_keyword<basic_json_value>;
    public:
        using name_t = std::variant<std::monostate, json_string, json_keyword>;
    public:
        using optional_json_string = std::optional<json_string>;
    public:
        using value_type = std::variant<std::monostate, json_object, json_array, json_double, json_int64, json_uint64, json_int, json_uint, json_string, json_bool, json_null, json_keyword>;
    private:
        using node_type = json_detail::basic_json_node<basic_json_value>;
    public:
        basic_json_value() :
            iNode{}, iValue{}, iDocumentSourceLocation{}
        {
        }
        basic_json_value(reference aParent, const value_type& aValue) :
            iNode{ aParent }, iValue{ aValue }, iDocumentSourceLocation{}
        {
            update_contents();
        }
        basic_json_value(reference aParent, value_type&& aValue) :
            iNode{ aParent }, iValue{ std::move(aValue) }, iDocumentSourceLocation{}
        {
            update_contents();
        }
        ~basic_json_value()
        {
            clear(true);
        }
    public:
        basic_json_value(const basic_json_value&) = delete;
        basic_json_value(basic_json_value&&) = delete;
    public:
        template <typename T>
        std::enable_if_t<!std::is_arithmetic_v<T>, const T&> as() const
        {
            return std::get<T>(iValue);
        }
        template <typename T>
        std::enable_if_t<!std::is_arithmetic_v<T>, T&> as()
        {
            return std::get<T>(iValue);
        }
        template <typename T>
        std::enable_if_t<std::is_arithmetic_v<T>, T> as() const
        {
            T result = {};
            std::visit([&result](auto&& v)
            {
                using Lhs = std::decay_t<T>;
                using Rhs = std::decay_t<decltype(v)>;
                if constexpr (std::is_convertible_v<Lhs, Rhs>)
                    result = static_cast<T>(v);
                else
                    throw bad_conversion();
            }, iValue);
            return result;
        }
        template <typename T>
        std::enable_if_t<std::is_arithmetic_v<T>, T&> as()
        {
            return std::get<T>(iValue);
        }
        const value_type& operator*() const
        {
            return iValue;
        }
        value_type& operator*()
        {
            return iValue;
        }
        reference operator=(const value_type& aValue)
        {
            clear();
            iValue = aValue;
            update_contents();
            return *this;
        }
        reference operator=(value_type&& aValue)
        {
            clear();
            iValue = std::move(aValue);
            update_contents();
            return *this;
        }
    public:
        json_type type() const
        {
            return static_cast<json_type>(iValue.index());
        }
        bool is_composite() const
        {
            return type() == json_type::Object || type() == json_type::Array;
        }
        bool is_empty_composite() const
        {
            return is_composite() && !iNode.has_children();
        }
        bool is_populated_composite() const
        {
            return is_composite() && iNode.has_children();
        }
        const json_string& text() const
        {
            if (type() != json_type::Keyword)
                return as<json_string>();
            else
                return as<json_keyword>().text;
        }
        bool has_name() const
        {
            return !std::holds_alternative<std::monostate>(iName);
        }
        bool name_is_keyword() const
        {
            return std::holds_alternative<json_keyword>(iName);
        }
        const json_string& name() const
        {
            if (!name_is_keyword())
                return std::get<json_string>(iName);
            else
                return std::get<json_keyword>(iName).text;
        }
        void set_name(const json_string& aName)
        {
            iName = aName;
        }
        void set_name(const json_keyword& aName)
        {
            iName = aName;
        }
    public:
        bool is_root() const
        {
            return !has_parent();
        }
        bool has_parent() const
        {
            return iNode.has_parent();
        }
        const_reference parent() const
        {
            return *iNode.parent();
        }
        reference parent()
        {
            return *iNode.parent();
        }
        bool has_children() const
        {
            return iNode.has_children();
        }
        const_pointer first_child() const
        {
            return iNode.first_child();
        }
        pointer first_child()
        {
            return iNode.first_child();
        }
        const_pointer last_child() const
        {
            return iNode.last_child();
        }
        pointer last_child()
        {
            return iNode.last_child();
        }
        bool is_last_sibling() const
        {
            return iNode.is_last_sibling();
        }
        const_pointer next_sibling() const
        {
            return iNode.next_sibling();
        }
        pointer next_sibling()
        {
            return iNode.next_sibling();
        }
        const_pointer next_parent_sibling() const
        {
            return iNode.next_parent_sibling();
        }
        pointer next_parent_sibling()
        {
            return iNode.next_parent_sibling();
        }
    public:
        const_iterator cbegin() const
        {
            return begin();
        }
        const_iterator cend() const
        {
            return end();
        }
        const_iterator begin() const
        {
            return first_child();
        }
        const_iterator end() const
        {
            return nullptr;
        }
        iterator begin()
        {
            return first_child();
        }
        iterator end()
        {
            return nullptr;
        }
    public:
        template <typename Visitor>
        void visit(Visitor&& aVisitor, bool aRecurse = true) const
        {
            std::visit([&aVisitor](auto&& arg) 
            { 
                if constexpr(!std::is_same_v<typename std::remove_cv<typename std::remove_reference<decltype(arg)>::type>::type, std::monostate>)
                    aVisitor(std::forward<decltype(arg)>(arg)); 
            }, iValue);
            switch (type())
            {
            case json_type::Object:
            case json_type::Array:
                if (aRecurse)
                    for (auto& v : *this)
                        v.visit(std::forward<Visitor>(aVisitor));
                break;
            }
        }
        template <typename Visitor>
        void visit(Visitor&& aVisitor, bool aRecurse = true)
        {
            std::visit([&aVisitor](auto&& arg)
            {
                if constexpr(!std::is_same_v<typename std::remove_cv<typename std::remove_reference<decltype(arg)>::type>::type, std::monostate>)
                    aVisitor(std::forward<decltype(arg)>(arg));
            }, iValue);
            switch (type())
            {
            case json_type::Object:
            case json_type::Array:
                if (aRecurse)
                    for (auto i = begin(); i != end(); ++i)
                        i.value().visit(std::forward<Visitor>(aVisitor));
                break;
            default:
                break;
            }
        }
    public:
        bool empty() const
        {
            return begin() == end();
        }
        std::size_t size() const
        {
            return std::distance(begin(), end()); // todo: this is O(n); have a size member instead for O(1)?
        }
        void clear(bool aUnlink = false)
        {
            auto previous = iNode.previous_sibling();
            auto next = iNode.next_sibling();
            if (has_parent())
            {
                auto& p = parent();
                iNode.~node_type();
                new(&iNode) node_type{ p };
            }
            else
            {
                iNode.~node_type();
                new(&iNode) node_type{};
            }
            iNode.set_previous_sibling(previous);
            iNode.set_next_sibling(next);
            if (aUnlink)
            {
                iNode.unlink();
            }
        }
        template <typename... Args>
        reference emplace_back(Args&&... aArguments)
        {
            return *buy_child(std::forward<Args>(aArguments)...);
        }
        template <typename T>
        void push_back(T&& aValue)
        {
            buy_child(std::forward<T>(aValue));
        }
        void pop_back()
        {
            iNode.destruct_child(last_child());
        }
    public:
        const json_document_source_location& document_source_location() const
        {
            return iDocumentSourceLocation;
        }
        void set_document_source_location(const json_document_source_location& aDocumentSourceLocation)
        {
            iDocumentSourceLocation = aDocumentSourceLocation;
        }
    private:
        template <typename... Args>
        pointer buy_child(Args&&... aArguments)
        {
            return iNode.buy_child(*this, std::forward<Args>(aArguments)...);
        }
        void update_contents()
        {
            if (type() == json_type::Object)
                std::get<json_object>(iValue).set_contents(*this);
            else if (type() == json_type::Array)
                std::get<json_array>(iValue).set_contents(*this);
        }
    private:
        node_type iNode;
        name_t iName;
        value_type iValue;
        json_document_source_location iDocumentSourceLocation;
    };

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    class basic_json
    {
    public:
        static constexpr json_syntax syntax = Syntax;
        using allocator_type = Alloc;
        using character_type = CharT;
        using character_traits_type = Traits;
        using character_allocator_type = CharAlloc;
        using json_value = basic_json_value<syntax, allocator_type, character_type, character_traits_type, character_allocator_type>;
        using optional_json_value = std::optional<json_value>;
        using json_object = typename json_value::json_object;
        using json_array = typename json_value::json_array;
        using json_double = typename json_value::json_double;
        using json_int64 = typename json_value::json_int64;
        using json_uint64 = typename json_value::json_uint64;
        using json_int = typename json_value::json_int;
        using json_uint = typename json_value::json_uint;
        using json_string = typename json_value::json_string;
        using json_bool = typename json_value::json_bool;
        using json_null = typename json_value::json_null;
        using json_keyword = typename json_value::json_keyword;
    public:
        using value_type = typename json_value::value_type;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
    private:
        template <typename IteratorTraits>
        class iterator_base;
    public:
        class const_iterator;
        class iterator;
    private:
        using string_type = std::basic_string<CharT, Traits, CharAlloc>;
        struct element
        {
            enum type_e
            {
                Unknown,
                String,
                Number,
                Keyword,
                EscapedUnicode,
                Name
            };
            type_e type;
            type_e auxType;
            character_type* start;
            character_type* auxStart;
            typename json_value::name_t name;
        };
    public:
        basic_json();
        basic_json(const std::string& aPath, bool aValidateUtf = false);
        template <typename Elem, typename ElemTraits>
        basic_json(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf = false);
    public:
        void clear();
        bool read(const std::string& aPath, bool aValidateUtf = false);
        template <typename Elem, typename ElemTraits>
        bool read(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf = false);
        bool write(const std::string& aPath, const string_type& aIndent = string_type(2, character_type{' '}));
        template <typename Elem, typename ElemTraits>
        bool write(std::basic_ostream<Elem, ElemTraits>& aOutput, const string_type& aIndent = string_type(2, character_type{' '}));
    public:
        json_encoding encoding() const;
        const json_string& document() const;
        const string_type& error_text() const;
    public:
        bool has_root() const;
        const json_value& croot() const;
        const json_value& root() const;
        json_value& root();
        const json_value& at(const json_string& aPath) const;
        json_value& at(const json_string& aPath);
        template <typename Visitor>
        void visit(Visitor&& aVisitor) const;
        template <typename Visitor>
        void visit(Visitor&& aVisitor);
        const_iterator cbegin() const;
        const_iterator cend() const;
        const_iterator begin() const;
        const_iterator end() const;
        iterator begin();
        iterator end();
    public:
        static string_type to_error_text(const json_document_source_location& aDocumentSourceLocation, const string_type& aExtraInfo = {});
        static string_type to_error_text(const json_value& aNode, const string_type& aExtraInfo = {});
    private:
        json_string& document();
        string_type to_error_text(const string_type& aExtraInfo = {}) const;
    private:
        template <typename Elem, typename ElemTraits>
        bool do_read(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf = false);
        bool do_parse();
        json_type context() const;
        template <typename T>
        json_value* buy_value(element& aCurrentElement, T&& aValue);
        void create_parse_error(const string_type& aExtraInfo = {}) const;
    private:
        json_encoding iEncoding;
        json_string iDocumentText;
        json_document_source_location iCursor;
        mutable string_type iErrorText;
        mutable optional_json_value iRoot;
        std::vector<json_value*> iCompositeValueStack;
        std::optional<char16_t> iUtf16HighSurrogate;
    };

    using json = basic_json<json_syntax::Standard>;
    using json_value = json::json_value;
    using json_object = json::json_object;
    using json_array = json::json_array;
    using json_double = json::json_double;
    using json_int64 = json::json_int64;
    using json_uint64 = json::json_uint64;
    using json_int = json::json_int;
    using json_uint = json::json_uint;
    using json_string = json::json_string;
    using json_bool = json::json_bool;
    using json_null = json::json_null;
    using json_keyword = json::json_keyword;
    
    using fast_json = basic_json<json_syntax::Standard, neolib::fast_pool_allocator<json_type>>;
    using fast_json_value = fast_json::json_value;
    using fast_json_object = fast_json::json_object;
    using fast_json_array = fast_json::json_array;
    using fast_json_double = fast_json::json_double;
    using fast_json_int64 = fast_json::json_int64;
    using fast_json_uint64 = fast_json::json_uint64;
    using fast_json_int = fast_json::json_int;
    using fast_json_uint = fast_json::json_uint;
    using fast_json_string = fast_json::json_string;
    using fast_json_bool = fast_json::json_bool;
    using fast_json_null = fast_json::json_null;
    using fast_json_keyword = fast_json::json_keyword;

    using rjson = basic_json<json_syntax::Relaxed>;
    using rjson_value = rjson::json_value;
    using rjson_object = rjson::json_object;
    using rjson_array = rjson::json_array;
    using rjson_double = rjson::json_double;
    using rjson_int64 = rjson::json_int64;
    using rjson_uint64 = rjson::json_uint64;
    using rjson_int = rjson::json_int;
    using rjson_uint = rjson::json_uint;
    using rjson_string = rjson::json_string;
    using rjson_bool = rjson::json_bool;
    using rjson_null = rjson::json_null;
    using rjson_keyword = rjson::json_keyword;

    using fast_rjson = basic_json<json_syntax::Relaxed, neolib::fast_pool_allocator<json_type>>;
    using fast_rjson_value = fast_rjson::json_value;
    using fast_rjson_object = fast_rjson::json_object;
    using fast_rjson_array = fast_rjson::json_array;
    using fast_rjson_double = fast_rjson::json_double;
    using fast_rjson_int64 = fast_rjson::json_int64;
    using fast_rjson_uint64 = fast_rjson::json_uint64;
    using fast_rjson_int = fast_rjson::json_int;
    using fast_rjson_uint = fast_rjson::json_uint;
    using fast_rjson_string = fast_rjson::json_string;
    using fast_rjson_bool = fast_rjson::json_bool;
    using fast_rjson_null = fast_rjson::json_null;
    using fast_rjson_keyword = fast_rjson::json_keyword;

    using fjson = basic_json<json_syntax::Functional>;
    using fjson_value = fjson::json_value;
    using fjson_object = fjson::json_object;
    using fjson_array = fjson::json_array;
    using fjson_double = fjson::json_double;
    using fjson_int64 = fjson::json_int64;
    using fjson_uint64 = fjson::json_uint64;
    using fjson_int = fjson::json_int;
    using fjson_uint = fjson::json_uint;
    using fjson_string = fjson::json_string;
    using fjson_bool = fjson::json_bool;
    using fjson_null = fjson::json_null;
    using fjson_keyword = fjson::json_keyword;

    using fast_fjson = basic_json<json_syntax::Functional, neolib::fast_pool_allocator<json_type>>;
    using fast_fjson_value = fast_fjson::json_value;
    using fast_fjson_object = fast_fjson::json_object;
    using fast_fjson_array = fast_fjson::json_array;
    using fast_fjson_double = fast_fjson::json_double;
    using fast_fjson_int64 = fast_fjson::json_int64;
    using fast_fjson_uint64 = fast_fjson::json_uint64;
    using fast_fjson_int = fast_fjson::json_int;
    using fast_fjson_uint = fast_fjson::json_uint;
    using fast_fjson_string = fast_fjson::json_string;
    using fast_fjson_bool = fast_fjson::json_bool;
    using fast_fjson_null = fast_fjson::json_null;
    using fast_fjson_keyword = fast_fjson::json_keyword;
}

#include <neolib/file/json.inl>

