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
#include <map>
#include <list>
#include <string>
#include <type_traits>
#include <istream>
#include <ostream>
#include <utility>
#include <memory>
#include <exception>
#include <boost/optional.hpp>
#include <boost/pool/pool_alloc.hpp>
#include "variant.hpp"
#include "quick_string.hpp"

namespace neolib
{
	namespace json_detail
	{
		enum class state;
	}

	enum class json_type
	{
		Unknown,
		Object,
		Array,
		Number,
		String,
		True,
		False,
		Null,
		Keyword
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
	private:
		typedef typename allocator_type::template rebind<self_type>::other value_allocator;
	public:
		typedef double json_number;
		typedef basic_quick_string<character_type, character_traits_type, character_allocator_type> json_string;
		typedef std::true_type json_true;
		typedef std::false_type json_false;
		typedef std::nullptr_t json_null;
		typedef std::multiset<self_type, std::less<self_type>, value_allocator> json_object;
		typedef std::list<self_type, value_allocator> json_array;
		typedef struct { json_string text; } json_keyword;
	public:
		typedef boost::optional<json_string> optional_json_string;
	public:
		class i_visitor
		{
		public:
			virtual void visit(const json_number& aNumber) = 0;
			virtual void visit(const json_string& aString) = 0;
			virtual void visit(const json_object& aObject) = 0;
			virtual void visit(const json_array& aArray) = 0;
			virtual void visit(json_true) = 0;
			virtual void visit(json_false) = 0;
			virtual void visit(json_null) = 0;
			virtual void visit(const json_keyword& aKeyword) {}
			virtual void visit(const json_string& aName, const json_number& aNumber) = 0;
			virtual void visit(const json_string& aName, const json_string& aString) = 0;
			virtual void visit(const json_string& aName, const json_object& aObject) = 0;
			virtual void visit(const json_string& aName, const json_array& aArray) = 0;
			virtual void visit(const json_string& aName, json_true) = 0;
			virtual void visit(const json_string& aName, json_false) = 0;
			virtual void visit(const json_string& aName, json_null) = 0;
			virtual void visit(const json_string& aName, const json_keyword& aKeyword) {}
		};
	private:
		typedef variant<json_object, json_array, json_number, json_string, json_true, json_false, json_null, json_keyword> value_type;
	public:
		basic_json_value() :
			iValue{}
		{
		}
		basic_json_value(const value_type& aValue) :
			iValue{ aValue }
		{
		}
		basic_json_value(value_type&& aValue) :
			iValue{ std::move(aValue) }
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
	public:
		bool operator<(const self_type& aRhs) const
		{
			return name() < aRhs.name();
		}
	public:
		bool has_name() const
		{
			return iName != boost::none;
		}
		const json_string& name() const
		{
			if (has_name())
				return *iName;
			throw no_name();
		}
		json_type type() const
		{
			return static_cast<json_type>(iValue.which());
		}
		bool is_composite() const
		{
			return type() == json_type::Object || type() == json_type::Array;
		}
	public:
		void accept(i_visitor& aVisitor) const
		{
			switch(type())
			{
			case json_type::Object:
				if (has_name())
					aVisitor.visit(name(), static_variant_cast<const json_object&>(iValue));
				else
					aVisitor.visit(static_variant_cast<const json_object&>(iValue));
				for (const auto& e : static_variant_cast<const json_object&>(iValue))
					e.accept(aVisitor);
				break;
			case json_type::Array:
				if (has_name())
					aVisitor.visit(name(), static_variant_cast<const json_array&>(iValue));
				else
					aVisitor.visit(static_variant_cast<const json_array&>(iValue));
				for (const auto& e : static_variant_cast<const json_array&>(iValue))
					e.accept(aVisitor);
				break;
			case json_type::Number:
				if (has_name())
					aVisitor.visit(name(), static_variant_cast<const json_number&>(iValue));
				else
					aVisitor.visit(static_variant_cast<const json_number&>(iValue));
				break;
			case json_type::String:
				if (has_name())
					aVisitor.visit(name(), static_variant_cast<const json_string&>(iValue));
				else
					aVisitor.visit(static_variant_cast<const json_string&>(iValue));
				break;
			case json_type::True:
				if (has_name())
					aVisitor.visit(name(), json_true{});
				else
					aVisitor.visit(json_true{});
				break;
			case json_type::False:
				if (has_name())
					aVisitor.visit(name(), json_false{});
				else
					aVisitor.visit(json_false{});
				break;
			case json_type::Null:
				if (has_name())
					aVisitor.visit(name(), json_null{});
				else
					aVisitor.visit(json_null{});
				break;
			}
		}
	private:
		optional_json_string iName;
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
		typedef boost::optional<value> optional_value;
		typedef typename value::json_object json_object;
		typedef typename value::json_array json_array;
		typedef typename value::json_number json_number;
		typedef typename value::json_string json_string;
		typedef typename value::json_true json_true;
		typedef typename value::json_false json_false;
		typedef typename value::json_null json_null;
		typedef typename value::json_keyword json_keyword;
	public:
		typedef typename value::i_visitor i_visitor;
		class default_visitor : public i_visitor
		{
		public:
			void visit(const json_number& aNumber) override {}
			void visit(const json_string& aString) override {}
			void visit(const json_object& aObject) override {}
			void visit(const json_array& aArray) override {}
			void visit(json_true) override {}
			void visit(json_false) override {}
			void visit(json_null) override {}
			void visit(const json_string& aName, const json_number& aNumber) override {}
			void visit(const json_string& aName, const json_string& aString) override {}
			void visit(const json_string& aName, const json_object& aObject) override {}
			void visit(const json_string& aName, const json_array& aArray) override {}
			void visit(const json_string& aName, json_true) override {}
			void visit(const json_string& aName, json_false) override {}
			void visit(const json_string& aName, json_null) override {}
		};
	private:
		typedef std::basic_string<CharT, Traits, CharAlloc> string;
		struct element
		{
			value* value;
			enum type_e
			{
				Unknown,
				String,
				Number,
				Keyword,
			} type;
			const char* start;
		};
	public:
		basic_json();
		basic_json(const std::string& aPath, bool aValidateUtf8 = false);
		template <typename Elem, typename ElemTraits>
		basic_json(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf8 = false);
	public:
		void clear();
		bool read(const std::string& aPath, bool aValidateUtf8 = false);
		template <typename Elem, typename ElemTraits>
		bool read(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf8 = false);
		bool write(const std::string& aPath);
		template <typename Elem, typename ElemTraits>
		bool write(std::basic_ostream<Elem, ElemTraits>& aOutput);
	public:
		const json_string& document() const;
		const string& error_text() const;
		bool has_root() const;
		const value& root() const;
		value& root();
		void accept(i_visitor& aVisitor);
	private:
		json_string& document();
	private:
		template <typename Elem, typename ElemTraits>
		bool do_read(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf8 = false);
		json_detail::state change_state(json_detail::state aCurrentState, json_detail::state aNextState, const character_type* aNextCh, element& aCurrentElement);
		void create_parse_error(const character_type* aDocumentPos);
	private:
		json_string iDocumentText;
		string iErrorText;
		optional_value iRoot;
		std::vector<value*> iCompositeValueStack;
	};

	typedef basic_json<> json;
	typedef json::value json_value;
	typedef json::json_object json_object;
	typedef json::json_array json_array;
	typedef json::json_number json_number;
	typedef json::json_string json_string;
	typedef json::json_true json_true;
	typedef json::json_false json_false;
	typedef json::json_null json_null;
	typedef json::json_keyword json_keyword;
}

#include "json.inl"

