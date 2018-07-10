// variant.hpp - v1.0
/*
 *  Copyright (c) 2007 Leigh Johnston.
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

#include <type_traits>
#include <variant>

namespace neolib
{
	typedef std::monostate none_t;
	const none_t none;

	template <typename... Types>
	using variant = std::variant<none_t, Types...>;

	template <typename... Types>
	inline bool operator==(const variant<Types...>& aVariant, none_t)
	{
		return std::holds_alternative<none_t>(aVariant);
	}

	template <typename... Types>
	inline bool operator==(none_t, const variant<Types...>& aVariant)
	{
		return std::holds_alternative<none_t>(aVariant);
	}

	template <typename... Types>
	inline bool operator!=(const variant<Types...>& aVariant, none_t)
	{
		return !std::holds_alternative<none_t>(aVariant);
	}

	template <typename... Types>
	inline bool operator!=(none_t, const variant<Types...>& aVariant)
	{
		return !std::holds_alternative<none_t>(aVariant);
	}

	// Deprecated, use std::get.
	template <typename T, typename Variant>
	inline T static_variant_cast(const Variant& aVariant)
	{
		typedef T result_type;
		typedef typename std::remove_cv<typename std::remove_reference<result_type>::type>::type alternative_type;
		auto& result = std::get<alternative_type>(aVariant);
		return static_cast<result_type>(result);
	}

	// Deprecated, use std::get.
	template <typename T, typename Variant>
	inline T static_variant_cast(Variant& aVariant)
	{ 
		typedef T result_type;
		typedef typename std::remove_cv<typename std::remove_reference<result_type>::type>::type alternative_type;
		auto& result = std::get<alternative_type>(aVariant);
		return static_cast<result_type>(result);
	}
}

// Deprecated, use std::get.
using neolib::static_variant_cast;