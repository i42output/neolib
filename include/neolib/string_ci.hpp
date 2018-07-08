// string_ci.hpp
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
#include <string>

namespace neolib 
{
	template <typename Traits>
	struct ci_char_traits : Traits
	{
	public:
		typedef typename Traits::char_type char_type;
		typedef typename Traits::int_type int_type;
	public:
		static int compare(const char_type* s1, const char_type* s2, std::size_t n)
		{
			for(;n-- > 0;++s1, ++s2)
			{
				if (eq(*s1, *s2))
					continue;
				else if (lt(*s1, *s2))
					return -1;
				else
					return 1;
			}
			return 0;
		}
		static const char_type* find(const char_type* str, std::size_t n, const char_type& c)
		{
			while(n > 0)
			{
				if (eq(*str, c))
					return str;
				++str;
				--n;	
			}
			return 0;
		}
		static bool eq(const char_type& c1, const char_type& c2)
		{
			return lower(c1) == lower(c2);
		}
		static bool lt(const char_type& c1, const char_type& c2)
		{
			return lower(c1) < lower(c2);
		}
		static int_type lower(char_type c)
		{
			return neolib::to_lower(c);
		}
	};

	typedef std::basic_string<char, ci_char_traits<std::char_traits<char> > > ci_string;
	typedef std::basic_string<char16_t, ci_char_traits<std::char_traits<char16_t> > > ci_u16string;

	inline ci_string make_ci_string(const std::string& s)
	{
		return ci_string(s.begin(), s.end());
	}
	inline ci_u16string make_ci_string(const std::u16string& s)
	{
		return ci_u16string(s.begin(), s.end());
	}

	inline std::string make_string(const ci_string& s)
	{
		return std::string(s.begin(), s.end());
	}
	inline std::u16string make_string(const ci_u16string & s)
	{
		return std::u16string(s.begin(), s.end());
	}

	inline bool operator==(const ci_string& s1, const std::string& s2)
	{
		return s1 == ci_string(s2.begin(), s2.end());
	}
	inline bool operator==(const std::string& s1, const ci_string& s2)
	{
		return ci_string(s1.begin(), s1.end()) == s2;
	}
	inline bool operator!=(const ci_string& s1, const std::string& s2)
	{
		return s1 != ci_string(s2.begin(), s2.end());
	}
	inline bool operator!=(const std::string& s1, const ci_string& s2)
	{
		return ci_string(s1.begin(), s1.end()) != s2;
	}
	inline bool operator<(const ci_string& s1, const std::string& s2)
	{
		return s1 < ci_string(s2.begin(), s2.end());
	}
	inline bool operator<(const std::string& s1, const ci_string& s2)
	{
		return ci_string(s1.begin(), s1.end()) < s2;
	}

	template <typename CharT, typename Traits, typename Alloc>	
	inline bool lexicographical_compare_ignoring_case(const std::basic_string<CharT, Traits, Alloc>& s1, 
		const std::basic_string<CharT, Traits, Alloc>& s2)
	{
		typedef std::basic_string<CharT, Traits, Alloc> string_type;
		typedef typename string_type::size_type size_type;
		size_type count = std::min(s1.size(), s2.size());
		size_type answer = ci_char_traits<std::char_traits<CharT> >::compare(s1.c_str(), s2.c_str(), count);
		return static_cast<int>(answer) < 0 || (answer == 0 && s1.size() < s2.size());
	}
}
