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
            typedef basic_json_node<T> self_type;
            template <json_syntax Syntax, typename Alloc , typename CharT, typename Traits, typename CharAlloc>
            friend class basic_json_value;
        public:
            typedef T json_value;
            typedef typename json_value::value_type value_type;
        private:
            typedef typename json_value::value_allocator value_allocator;
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
            const json_value* next_sibling() const
            {
                return iNext;
            }
            json_value* next_sibling()
            {
                return iNext;
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
        typedef basic_json_object<T> self_type;
        template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
        friend class basic_json;
        template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
        friend class basic_json_value;
    public:
        typedef T json_value;
        typedef typename json_value::value_type value_type;
        typedef typename json_value::json_string json_string;
    private:
        typedef typename json_value::value_allocator allocator_type;
        typedef std::unordered_multimap<
            json_string, 
            json_value*, 
            boost::hash<json_string>, 
            std::equal_to<json_string>, 
            typename std::allocator_traits<allocator_type>::template rebind_alloc<std::pair<const json_string, json_value*>>> dictionary_type;
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
        typedef basic_json_array<T> self_type;
        template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
        friend class basic_json;
        template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
        friend class basic_json_value;
    public:
        typedef T json_value;
        typedef typename json_value::value_type value_type;
        typedef typename json_value::json_string json_string;
    private:
        typedef typename json_value::value_allocator allocator_type;
        typedef std::vector<json_value*> array_type;
    public:
        basic_json_array() :
            iContents{ nullptr }
        {
        }
        basic_json_array(json_value& aOwner) :
            iContents{ aOwner }
        {
        }
    public:
        json_value& push_back(const value_type& aValue)
        {
            auto& newChild = contents().emplace_back(aValue);
            cache().emplace_back(&newChild);
            return newChild;
        }
        json_value& operator[](std::size_t aIndex)
        {
            return *cache().at(aIndex);
        }
    public:
        json_value& owner() const
        {
            return *iContents;
        }
        const json_value& contents() const
        {
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
            for (auto & e : contents())
                cache().emplace_back(&e);
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
        typedef T json_value;
        typedef typename json_value::json_string json_string;
    public:
        json_string text;
    };

    template <typename T>
    struct basic_json_null
    {
    public:
        typedef T json_value;
    public:
        bool operator==(std::nullptr_t) const { return true; }
        bool operator!=(std::nullptr_t) const { return false; }
    };

    template <json_syntax Syntax = json_syntax::Standard, typename Alloc = std::allocator<json_type>, typename CharT = char, typename Traits = std::char_traits<CharT>, typename CharAlloc = std::allocator<CharT>>
    class basic_json;
        
    template <json_syntax Syntax = json_syntax::Standard, typename Alloc = std::allocator<json_type>, typename CharT = char, typename Traits = std::char_traits<CharT>, typename CharAlloc = std::allocator<CharT>>
    class basic_json_value
    {
        typedef basic_json_value<Syntax, Alloc, CharT, Traits, CharAlloc> self_type;
        friend class basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>;
        template <typename T>
        friend class json_detail::basic_json_node;
    public:
        struct no_name : std::logic_error { no_name() : std::logic_error("neolib::basic_json_value::no_name") {} };
    public:
        static constexpr json_syntax syntax = Syntax;
        typedef Alloc allocator_type;
        typedef CharT character_type;
        typedef Traits character_traits_type;
        typedef CharAlloc character_allocator_type;
    public:
        typedef self_type node_value_type;
    public:
        typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<self_type> value_allocator;
    public:
        typedef const self_type* const_pointer;
        typedef self_type* pointer;
        typedef const self_type& const_reference;
        typedef self_type& reference;
    private:
        template <typename IteratorTraits>
        class iterator_base;
    public:
        class const_iterator;
        class iterator;
    public:
        typedef basic_quick_string<character_type, character_traits_type, character_allocator_type> json_string;
        typedef basic_json_object<self_type> json_object;
        typedef basic_json_array<self_type> json_array;
        typedef double json_double;
        typedef int64_t json_int64;
        typedef uint64_t json_uint64;
        typedef int32_t json_int;
        typedef uint32_t json_uint;
        typedef bool json_bool;
        typedef basic_json_null<self_type> json_null;
        typedef basic_json_keyword<self_type> json_keyword;
    public:
        typedef std::variant<std::monostate, json_string, json_keyword> name_t;
    public:
        typedef std::optional<json_string> optional_json_string;
    public:
        typedef std::variant<std::monostate, json_object, json_array, json_double, json_int64, json_uint64, json_int, json_uint, json_string, json_bool, json_null, json_keyword> value_type;
    private:
        typedef json_detail::basic_json_node<basic_json_value> node_type;
    public:
        basic_json_value() :
            iNode{}, iValue{}, iDocumentSourceLocation{}
        {
        }
        basic_json_value(reference aParent, const value_type& aValue) :
            iNode{ aParent }, iValue{ aValue }, iDocumentSourceLocation{}
        {
        }
        basic_json_value(reference aParent, value_type&& aValue) :
            iNode{ aParent }, iValue{ std::move(aValue) }, iDocumentSourceLocation{}
        {
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
            return std::get<T>(iValue);
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
            iValue = aValue;
            update_contents();
            return *this;
        }
        reference operator=(value_type&& aValue)
        {
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
        void clear()
        {
            iNode.~node_type();
            new(&iNode) node_type();
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

    struct json_error : std::runtime_error { json_error(const std::string& aReason) : std::runtime_error(aReason) {} };
    struct json_path_not_found : std::runtime_error { json_path_not_found(const std::string& aPath) : std::runtime_error("JSON path not found: " + aPath) {} };

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    class basic_json
    {
        typedef basic_json<Syntax, Alloc, CharT, Traits, CharAlloc> self_type;
    public:
        static constexpr json_syntax syntax = Syntax;
        typedef Alloc allocator_type;
        typedef CharT character_type;
        typedef Traits character_traits_type;
        typedef CharAlloc character_allocator_type;
        typedef basic_json_value<syntax, allocator_type, character_type, character_traits_type, character_allocator_type> json_value;
        typedef std::optional<json_value> optional_json_value;
        typedef typename json_value::json_object json_object;
        typedef typename json_value::json_array json_array;
        typedef typename json_value::json_double json_double;
        typedef typename json_value::json_int64 json_int64;
        typedef typename json_value::json_uint64 json_uint64;
        typedef typename json_value::json_int json_int;
        typedef typename json_value::json_uint json_uint;
        typedef typename json_value::json_string json_string;
        typedef typename json_value::json_bool json_bool;
        typedef typename json_value::json_null json_null;
        typedef typename json_value::json_keyword json_keyword;
    public:
        typedef typename json_value::value_type value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
    private:
        template <typename IteratorTraits>
        class iterator_base;
    public:
        class const_iterator;
        class iterator;
    private:
        typedef std::basic_string<CharT, Traits, CharAlloc> string_type;
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

    typedef basic_json<json_syntax::Standard> json;
    typedef json::json_value json_value;
    typedef json::json_object json_object;
    typedef json::json_array json_array;
    typedef json::json_double json_double;
    typedef json::json_int64 json_int64;
    typedef json::json_uint64 json_uint64;
    typedef json::json_int json_int;
    typedef json::json_uint json_uint;
    typedef json::json_string json_string;
    typedef json::json_bool json_bool;
    typedef json::json_null json_null;
    typedef json::json_keyword json_keyword;

    typedef basic_json<json_syntax::Standard, neolib::fast_pool_allocator<json_type>> fast_json;
    typedef fast_json::json_value fast_json_value;
    typedef fast_json::json_object fast_json_object;
    typedef fast_json::json_array fast_json_array;
    typedef fast_json::json_double fast_json_double;
    typedef fast_json::json_int64 fast_json_int64;
    typedef fast_json::json_uint64 fast_json_uint64;
    typedef fast_json::json_int fast_json_int;
    typedef fast_json::json_uint fast_json_uint;
    typedef fast_json::json_string fast_json_string;
    typedef fast_json::json_bool fast_json_bool;
    typedef fast_json::json_null fast_json_null;
    typedef fast_json::json_keyword fast_json_keyword;

    typedef basic_json<json_syntax::Relaxed> rjson;
    typedef rjson::json_value rjson_value;
    typedef rjson::json_object rjson_object;
    typedef rjson::json_array rjson_array;
    typedef rjson::json_double rjson_double;
    typedef rjson::json_int64 rjson_int64;
    typedef rjson::json_uint64 rjson_uint64;
    typedef rjson::json_int rjson_int;
    typedef rjson::json_uint rjson_uint;
    typedef rjson::json_string rjson_string;
    typedef rjson::json_bool rjson_bool;
    typedef rjson::json_null rjson_null;
    typedef rjson::json_keyword rjson_keyword;

    typedef basic_json<json_syntax::Relaxed, neolib::fast_pool_allocator<json_type>> fast_rjson;
    typedef fast_rjson::json_value fast_rjson_value;
    typedef fast_rjson::json_object fast_rjson_object;
    typedef fast_rjson::json_array fast_rjson_array;
    typedef fast_rjson::json_double fast_rjson_double;
    typedef fast_rjson::json_int64 fast_rjson_int64;
    typedef fast_rjson::json_uint64 fast_rjson_uint64;
    typedef fast_rjson::json_int fast_rjson_int;
    typedef fast_rjson::json_uint fast_rjson_uint;
    typedef fast_rjson::json_string fast_rjson_string;
    typedef fast_rjson::json_bool fast_rjson_bool;
    typedef fast_rjson::json_null fast_rjson_null;
    typedef fast_rjson::json_keyword fast_rjson_keyword; 

    typedef basic_json<json_syntax::Functional> fjson;
    typedef fjson::json_value fjson_value;
    typedef fjson::json_object fjson_object;
    typedef fjson::json_array fjson_array;
    typedef fjson::json_double fjson_double;
    typedef fjson::json_int64 fjson_int64;
    typedef fjson::json_uint64 fjson_uint64;
    typedef fjson::json_int fjson_int;
    typedef fjson::json_uint fjson_uint;
    typedef fjson::json_string fjson_string;
    typedef fjson::json_bool fjson_bool;
    typedef fjson::json_null fjson_null;
    typedef fjson::json_keyword fjson_keyword;

    typedef basic_json<json_syntax::Functional, neolib::fast_pool_allocator<json_type>> fast_fjson;
    typedef fast_fjson::json_value fast_fjson_value;
    typedef fast_fjson::json_object fast_fjson_object;
    typedef fast_fjson::json_array fast_fjson_array;
    typedef fast_fjson::json_double fast_fjson_double;
    typedef fast_fjson::json_int64 fast_fjson_int64;
    typedef fast_fjson::json_uint64 fast_fjson_uint64;
    typedef fast_fjson::json_int fast_fjson_int;
    typedef fast_fjson::json_uint fast_fjson_uint;
    typedef fast_fjson::json_string fast_fjson_string;
    typedef fast_fjson::json_bool fast_fjson_bool;
    typedef fast_fjson::json_null fast_fjson_null;
    typedef fast_fjson::json_keyword fast_fjson_keyword;
}

#include <neolib/file/json.inl>

