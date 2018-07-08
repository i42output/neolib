// json.hpp
/*
 *  NoFussJSON v1.0
 *
 *  Copyright (c) 2018 Leigh Johnston.
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

#include "neolib.hpp"
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
#include <neolib/allocator.hpp>
#include <neolib/variant.hpp>
#include <neolib/quick_string.hpp>

namespace neolib
{
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
		enum class state;

		template <std::size_t CharSize>
		struct default_encoding_helper;
		template <> struct default_encoding_helper<1> { static const json_encoding DEFAULT_ENCODING = json_encoding::Utf8; };
		template <> struct default_encoding_helper<2> { static const json_encoding DEFAULT_ENCODING = json_encoding::Utf16LE; };
		template <> struct default_encoding_helper<4> { static const json_encoding DEFAULT_ENCODING = json_encoding::Utf32LE; };

		template <typename CharT>
		struct default_encoding	{ static const json_encoding DEFAULT_ENCODING = default_encoding_helper<sizeof(CharT)>::DEFAULT_ENCODING; };
	}

	enum class json_type
	{
		Unknown,
		Object,
		Array,
		Number,
		String,
		Bool,
		Null,
		Keyword
	};

	namespace json_detail
	{
		template <typename T>
		class basic_json_node
		{
			template <typename Alloc , typename CharT, typename Traits, typename CharAlloc>
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
			json_value* buy_child(json_value& aParent)
			{
				construct_child(allocate_child(), aParent);
			}
			json_value* buy_child(json_value& aParent, const value_type& aValue)
			{
				construct_child(allocate_child(), aParent, aValue);
			}
			json_value* buy_child(json_value& aParent, value_type&& aValue)
			{
				construct_child(allocate_child(), aParent, std::move(aValue));
			}
		private:
			json_value* allocate_child()
			{
				auto address = iAllocator.allocate(1);
			}
			void deallocate_child(json_value* aAddress)
			{
				iAllocator.deallocate(aAddress);
			}
			template <typename... Args>
			void construct_child(json_value* aAddress, Args&&... aArguments)
			{
				iAllocator.construct(aAddress, std::forward<Args>(aArguments)...);
				json_value& child = *aAddress;
				if (iFirstChild == nullptr)
				{
					iFirstChild = &child;
					iLastChild = &child;
				}
			}
			void destruct_child(json_value* aAddress)
			{
				aAddress->~json_value();
			}
		private:
			value_allocator iAllocator;
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
		template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
		friend class basic_json_value;
	public:
		typedef T json_value;
		typedef typename json_value::value_type value_type;
		typedef typename json_value::json_string json_string;
	private:
		typedef typename json_value::value_allocator allocator_type;
		typedef std::unordered_multimap<json_string, json_value*, std::hash<json_string>, std::equal_to<json_string>, typename allocator_type:: template rebind<std::pair<const json_string, json_value*>>::other> dictionary_type;
	public:
		basic_json_object(json_value& aOwner) :
			iOwner{ aOwner }
		{
		}
	private:
		json_value & iOwner;
		std::unique_ptr<dictionary_type> iLazyDictionary;
	};

	template <typename T>
	class basic_json_array
	{
		template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
		friend class basic_json_value;
	public:
		typedef T json_value;
		typedef typename json_value::value_type value_type;
		typedef typename json_value::json_string json_string;
	private:
		typedef typename json_value::value_allocator allocator_type;
		typedef std::vector<json_value*> array_type;
	public:
		basic_json_array(json_value& aOwner) :
			iOwner{ aOwner }
		{
		}
	private:
		json_value & iOwner;
		std::unique_ptr<array_type> iLazyArray;
	};

	template <typename Alloc = std::allocator<json_type>, typename CharT = char, typename Traits = std::char_traits<CharT>, typename CharAlloc = std::allocator<CharT>>
	class basic_json_value
	{
	public:
		struct no_name : std::logic_error { no_name() : std::logic_error("neolib::basic_json_value::no_name") {} };
	public:
		typedef Alloc allocator_type;
		typedef CharT character_type;
		typedef Traits character_traits_type;
		typedef CharAlloc character_allocator_type;
		typedef basic_json_value<allocator_type, character_type, character_traits_type, character_allocator_type> self_type;
	public:
		typedef typename allocator_type::template rebind<self_type>::other value_allocator;
	public:
		typedef basic_quick_string<character_type, character_traits_type, character_allocator_type> json_string;
		typedef basic_json_object<self_type> json_object;
		typedef basic_json_array<self_type> json_array;
		typedef double json_number;
		typedef bool json_bool;
		typedef std::nullptr_t json_null;
		typedef struct { json_string text; } json_keyword;
	public:
		typedef std::optional<json_string> optional_json_string;
	public:
		typedef variant<json_object, json_array, json_number, json_string, json_bool, json_null, json_keyword> value_type;
		class i_visitor
		{
		public:
			virtual void visit(const json_number& aNumber) = 0;
			virtual void visit(const json_string& aString) = 0;
			virtual void visit(const json_object& aObject) = 0;
			virtual void visit(const json_array& aArray) = 0;
			virtual void visit(const json_bool& aBool) = 0;
			virtual void visit(const json_null&) = 0;
			virtual void visit(const json_keyword& aKeyword) {}
			virtual void visit(json_number& aNumber) { visit(const_cast<const json_number&>(aNumber)); }
			virtual void visit(json_string& aString) { visit(const_cast<const json_string&>(aString)); }
			virtual void visit(json_object& aObject) { visit(const_cast<const json_object&>(aObject)); }
			virtual void visit(json_array& aArray) { visit(const_cast<const json_array&>(aArray)); }
			virtual void visit(json_bool& aBool) { visit(const_cast<const json_bool&>(aBool)); }
			virtual void visit(json_null& aNull) { visit(const_cast<const json_null&>(aNull)); }
			virtual void visit(json_keyword& aKeyword) { visit(const_cast<const json_keyword&>(aKeyword)); }
		};
	private:
		typedef json_detail::basic_json_node<basic_json_value> node_type;
		typedef variant<typename json_object::iterator, typename json_array::iterator> parent_container_iterator;
	public:
		basic_json_value() :
			iNode{}, iValue {}
		{
		}
		basic_json_value(basic_json_value& aParent, const value_type& aValue) :
			iNode{ &aParent }, iValue {	aValue }
		{
		}
		basic_json_value(basic_json_value& aParent, value_type&& aValue) :
			iNode{ &aParent }, iValue{ std::move(aValue) }
		{
		}
	public:
		template <typename T>
		const T& as() const
		{
			return static_variant_cast<const T&>(iValue);
		}
		template <typename T>
		T& as()
		{
			return static_variant_cast<T&>(iValue);
		}
		const value_type& operator*() const
		{
			return iValue;
		}
		value_type& operator*()
		{
			return iValue;
		}
		basic_json_value& operator=(const value_type& aValue)
		{
			iValue = aValue;
			return *this;
		}
		basic_json_value& operator=(value_type&& aValue)
		{
			iValue = std::move(aValue);
			return *this;
		}
	public:
		json_type type() const
		{
			return static_cast<json_type>(iValue.which());
		}
		bool is_composite() const
		{
			return type() == json_type::Object || type() == json_type::Array;
		}
		bool is_empty_composite() const
		{
			return (type() == json_type::Object && as<json_object>().empty()) || (type() == json_type::Array && as<json_array>().empty());
		}
		bool is_populated_composite() const
		{
			return (type() == json_type::Object && !as<json_object>().empty()) || (type() == json_type::Array && !as<json_array>().empty());
		}
		bool has_name() const
		{
			return has_parent() && parent().type() == json_type::Object;
		}
		const json_string& name() const
		{
			return (*static_variant_cast<typename json_object::iterator>(parent_pos())).first;
		}
	public:
		bool has_parent() const
		{
			return iParent != nullptr;
		}
		const basic_json_value& parent() const
		{
			return *iParent;
		}
		basic_json_value& parent()
		{
			return *iParent;
		}
		const parent_container_iterator& parent_pos() const
		{
			return iParentPos;
		}
		void set_parent_pos(const parent_container_iterator& aParentPos)
		{
			iParentPos = aParentPos;
		}
		const basic_json_value* first_child() const
		{
			if (type() == json_type::Array)
			{
				auto& a = as<json_array>();
				if (!a.empty())
					return &*a.begin();
				else
					return nullptr;
			}
			else if (type() == json_type::Object)
			{
				auto& o = as<json_object>();
				if (!o.empty())
					return &(*o.begin()).second;
				else
					return nullptr;
			}
			else
				return nullptr;
		}
		basic_json_value* first_child()
		{
			return const_cast<basic_json_value*>(const_cast<const basic_json_value*>(this)->first_child());
		}
		bool is_last_sibling() const
		{
			if (!has_parent())
				return true;
			if (iParent->type() == json_type::Array)
			{
				auto& a = iParent->as<json_array>();
				return static_variant_cast<json_array::iterator>(parent_pos()) == std::prev(a.end());
			}
			else if (iParent->type() == json_type::Object)
			{
				auto& o = iParent->as<json_object>();
				return static_variant_cast<json_object::iterator>(parent_pos()) == std::prev(o.end());
			}
			return true;
		}
		const basic_json_value* next_sibling() const
		{
			if (is_last_sibling())
				return nullptr;
			else if (parent().type() == json_type::Array)
			{
				auto& a = parent().as<json_array>();
				return &*std::next(static_variant_cast<json_array::iterator>(parent_pos()));
			}
			else if (parent().type() == json_type::Object)
			{
				auto& o = parent().as<json_object>();
				return &(*std::next(static_variant_cast<json_object::iterator>(parent_pos()))).second;
			}
			return nullptr;
		}
		basic_json_value* next_sibling()
		{
			return const_cast<basic_json_value*>(const_cast<const basic_json_value*>(this)->next_sibling());
		}
		const basic_json_value* next_parent_sibling() const
		{
			auto tryParent = iParent;
			if (tryParent == nullptr)
				return nullptr;
			while (tryParent->has_parent() && tryParent->is_last_sibling())
				tryParent = &tryParent->parent();
			if (tryParent->is_last_sibling())
				return nullptr;
			else if (tryParent->parent().type() == json_type::Array)
			{
				auto& a = tryParent->parent().as<json_array>();
				return &*std::next(static_variant_cast<json_array::iterator>(tryParent->parent_pos()));
			}
			else if (tryParent->parent().type() == json_type::Object)
			{
				auto& o = tryParent->parent().as<json_object>();
				return &(*std::next(static_variant_cast<json_object::iterator>(tryParent->parent_pos()))).second;
			}
			return nullptr;
		}
		basic_json_value* next_parent_sibling()
		{
			return const_cast<basic_json_value*>(const_cast<const basic_json_value*>(this)->next_parent_sibling());
		}
	public:
		void accept(i_visitor& aVisitor) const
		{
			switch(type())
			{
			case json_type::Object:
				aVisitor.visit(static_variant_cast<const json_object&>(iValue));
				for (const auto& e : static_variant_cast<const json_object&>(iValue))
					e.accept(aVisitor);
				break;
			case json_type::Array:
				aVisitor.visit(static_variant_cast<const json_array&>(iValue));
				for (const auto& e : static_variant_cast<const json_array&>(iValue))
					e.second.accept(aVisitor);
				break;
			case json_type::Number:
				aVisitor.visit(static_variant_cast<const json_number&>(iValue));
				break;
			case json_type::String:
				aVisitor.visit(static_variant_cast<const json_string&>(iValue));
				break;
			case json_type::Bool:
				aVisitor.visit(static_variant_cast<const json_bool&>(iValue));
				break;;
			case json_type::Null:
				aVisitor.visit(static_variant_cast<const json_null&>(iValue));
				break;
			case json_type::Keyword:
				aVisitor.visit(static_variant_cast<const json_keyword&>(iValue));
				break;
			}
		}
		void accept(i_visitor& aVisitor)
		{
			switch (type())
			{
			case json_type::Object:
				aVisitor.visit(static_variant_cast<json_object&>(iValue));
				for (auto& e : static_variant_cast<json_object&>(iValue))
					e.second.accept(aVisitor);
				break;
			case json_type::Array:
				aVisitor.visit(static_variant_cast<json_array&>(iValue));
				for (auto& e : static_variant_cast<json_array&>(iValue))
					e.accept(aVisitor);
				break;
			case json_type::Number:
				aVisitor.visit(static_variant_cast<json_number&>(iValue));
				break;
			case json_type::String:
				aVisitor.visit(static_variant_cast<json_string&>(iValue));
				break;
			case json_type::Bool:
				aVisitor.visit(static_variant_cast<json_bool&>(iValue));
				break;
			case json_type::Null:
				aVisitor.visit(static_variant_cast<json_null&>(iValue));
				break;
			case json_type::Keyword:
				aVisitor.visit(static_variant_cast<json_keyword&>(iValue));
				break;
			}
		}
	private:
		node_type iNode;
		value_type iValue;
	};

	template <typename Alloc = std::allocator<json_type>, typename CharT = char, typename Traits = std::char_traits<CharT>, typename CharAlloc = std::allocator<CharT>>
	class basic_json
	{
	public:
		struct json_error : std::runtime_error { json_error(const std::string& aReason) : std::runtime_error(aReason) {} };
		struct no_root : std::logic_error { no_root() : std::logic_error("neolib::basic_json::no_root") {} };
	public:
		typedef Alloc allocator_type;
		typedef CharT character_type;
		typedef Traits character_traits_type;
		typedef CharAlloc character_allocator_type;
		typedef basic_json<Alloc, CharT, Traits, CharAlloc> self_type;
		typedef basic_json_value<allocator_type, character_type, character_traits_type, character_allocator_type> value;
		typedef std::optional<value> optional_value;
		typedef typename value::json_object json_object;
		typedef typename value::json_array json_array;
		typedef typename value::json_number json_number;
		typedef typename value::json_string json_string;
		typedef typename value::json_bool json_bool;
		typedef typename value::json_null json_null;
		typedef typename value::json_keyword json_keyword;
	public:
		typedef typename value::value_type value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
	public:
		typedef typename value::i_visitor i_visitor;
		class default_visitor : public i_visitor
		{
		public:
			void visit(const json_number& aNumber) override {}
			void visit(const json_string& aString) override {}
			void visit(const json_object& aObject) override {}
			void visit(const json_array& aArray) override {}
			void visit(const json_bool& aBool) override {}
			void visit(const json_null&) override {}
		};
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
			std::optional<json_string> name;
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
		const value& root() const;
		value& root();
		void accept(i_visitor& aVisitor);
		const_iterator cbegin() const;
		const_iterator cend() const;
		const_iterator begin() const;
		const_iterator end() const;
		iterator begin();
		iterator end();
	private:
		json_string& document();
	private:
		template <typename Elem, typename ElemTraits>
		bool do_read(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf = false);
		json_detail::state change_state(json_detail::state aCurrentState, json_detail::state aNextState, character_type* aNextInputCh, character_type*& aNextOutputCh, element& aCurrentElement);
		json_type context() const;
		template <typename T>
		value* buy_value(element& aCurrentElement, T&& aValue);
		void create_parse_error(const character_type* aDocumentPos, const string_type& aExtraInfo = {});
	private:
		json_encoding iEncoding;
		json_string iDocumentText;
		string_type iErrorText;
		optional_value iRoot;
		std::vector<value*> iCompositeValueStack;
		std::optional<char16_t> iUtf16HighSurrogate;
	};

	typedef basic_json<> json;
	typedef json::value json_value;
	typedef json::json_object json_object;
	typedef json::json_array json_array;
	typedef json::json_number json_number;
	typedef json::json_string json_string;
	typedef json::json_bool json_bool;
	typedef json::json_null json_null;
	typedef json::json_keyword json_keyword;

	typedef basic_json<neolib::fast_pool_allocator<json_type>> fast_json;
	typedef fast_json::value fast_json_value;
	typedef fast_json::json_object fast_json_object;
	typedef fast_json::json_array fast_json_array;
	typedef fast_json::json_number fast_json_number;
	typedef fast_json::json_string fast_json_string;
	typedef fast_json::json_bool fast_json_bool;
	typedef fast_json::json_null fast_json_null;
	typedef fast_json::json_keyword fast_json_keyword;
}

#include "json.inl"

