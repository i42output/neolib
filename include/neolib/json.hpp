// json.hpp
/*
 *  NoFussJSON v1.0
 *
 *  Copyright (c) 2018-present, Leigh Johnston.  All Rights Reserved.
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

	template <typename Alloc = std::allocator<char>>
	class basic_json_value
	{
	public:
		typedef Alloc allocator_type;
		typedef basic_json_value<allocator_type> self_type;
	private:
		typedef typename allocator_type::template rebind<self_type>::other value_allocator;
	public:
		typedef variant<uint64_t, int64_t, double> json_number;
		typedef std::string json_string;
		typedef std::true_type json_true;
		typedef std::false_type json_false;
		typedef std::nullptr_t json_null;
		typedef std::map<json_string, self_type, value_allocator> json_object;
		typedef std::list<self_type, value_allocator> json_array;
	private:
		typedef variant<json_object, json_array, json_number, json_string, json_true, json_false, json_null> value_type;
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
		json_type type() const
		{
			return static_cast<json_type>(iValue.which() - 1);
		}
	private:
		value_type iValue;
	};

	template <typename CharT, typename Alloc = std::allocator<CharT>>
	class basic_json
	{
	public:
		typedef Alloc allocator_type;
		typedef basic_json_value<allocator_type> value;
	public:
		basic_json();
		basic_json(const std::string& aPath);
	public:
		template <typename Elem, typename Traits>
		bool read(std::basic_istream<Elem, Traits>& aInput);
		template <typename Elem, typename Traits>
		bool write(std::basic_ostream<Elem, Traits>& aOutput);
	};

	typedef basic_json<char> json;
	typedef json::value json_value;
	typedef json_value::json_object json_object;
	typedef json_value::json_array json_array;
	typedef json_value::json_number json_number;
	typedef json_value::json_string json_string;
	typedef json_value::json_true json_true;
	typedef json_value::json_false json_false;
	typedef json_value::json_null json_null;
}

