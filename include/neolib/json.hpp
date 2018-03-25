// json.hpp
/*
 *  NoFussJSON v1.0
 *
 *  Copyright (c) 2018-present, Leigh Johnston.
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
	enum class json_type
	{
		Object,
		Array,
		Number,
		String,
		True,
		False,
		Null
	};

	template <typename Alloc = std::allocator<json_type>, typename CharT = char, typename Traits = std::char_traits<CharT>, typename CharAlloc = std::allocator<CharT>>
	class basic_json_value
	{
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
	public:
		typedef boost::optional<json_string> optional_json_string;
	public:
		class i_visitor
		{
		public:
			virtual void visit(const optional_json_string& aName, const json_number& aNumber) = 0;
			virtual void visit(const optional_json_string& aName, const json_string& aString) = 0;
			virtual void visit(const optional_json_string& aName, const json_object& aObject) = 0;
			virtual void visit(const optional_json_string& aName, const json_array& aArray) = 0;
			virtual void visit(const optional_json_string& aName, const json_true&) = 0;
			virtual void visit(const optional_json_string& aName, const json_false&) = 0;
			virtual void visit(const optional_json_string& aName, const json_null&) = 0;
		};
	private:
		typedef variant<json_object, json_array, json_number, json_string, json_true, json_false, json_null> value_type;
	public:
		struct no_name : std::logic_error { no_name() : std::logic_error("neolib::basic_json_value::no_name") {} };
	public:
		basic_json_value(const value_type& aValue) :
			iValue{ aValue }
		{
		}
		basic_json_value(value_type&& aValue) :
			iValue{ std::move(aValue) }
		{
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
			return static_cast<json_type>(iValue.which() - 1);
		}
	public:
		void accept(i_visitor& aVisitor)
		{
			switch(type())
			{
			case Object:
				aVisitor.visit(iName, static_variant_cast<const json_object&>(iValue));
				for (const auto& e : static_variant_cast<const json_object&>(iValue))
					e.accept(aVisitor);
				break;
			case Array:
				aVisitor.visit(iName, static_variant_cast<const json_array&>(iValue));
				for (const auto& e : static_variant_cast<const json_array&>(iValue))
					e.accept(aVisitor);
				break;
			case Number:
				aVisitor.visit(iName, static_variant_cast<const json_number&>(iValue));
				break;
			case String:
				aVisitor.visit(iName, static_variant_cast<const json_string&>(iValue));
				break;
			case True:
				aVisitor.visit(iName, json_true{});
				break;
			case False:
				aVisitor.visit(iName, json_false{});
				break;
			case Null:
				aVisitor.visit(iName, json_null{});
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
	public:
		typedef Alloc allocator_type;
		typedef CharT character_type;
		typedef Traits character_traits_type;
		typedef CharAlloc character_allocator_type;
		typedef basic_json_value<allocator_type, character_type, character_traits_type, character_allocator_type> value;
		typedef typename value::json_object json_object;
		typedef typename value::json_array json_array;
		typedef typename value::json_number json_number;
		typedef typename value::json_string json_string;
		typedef typename value::json_true json_true;
		typedef typename value::json_false json_false;
		typedef typename value::json_null json_null;
	private:
		typedef std::basic_string<CharT, Traits, CharAlloc> string;
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
	private:
		json_string& document();
	private:
		json_string iDocumentText;
		string iErrorText;
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
}

#include "json.inl"

