// string_utils.h
/*
 *  Copyright (c) 2012 Leigh Johnston.
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
#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cwchar>
#include <cuchar>
#include <cctype>
#include <cwctype>
#include <cassert>
#include <boost/locale.hpp> 

namespace neolib 
{
	template <typename FwdIter1, typename FwdIter2, typename ResultContainer>
	inline FwdIter1 tokens(FwdIter1 aFirst, FwdIter1 aLast, FwdIter2 aDelimeterFirst, FwdIter2 aDelimiterLast, ResultContainer& aTokens, std::size_t aMaxTokens = 0, bool aSkipEmptyTokens = true, bool aDelimeterIsSubsequence = false)
	{
		if (aFirst == aLast)
			return aFirst;
		typedef typename ResultContainer::value_type value_type;
		if (aDelimeterFirst == aDelimiterLast)
		{
			aTokens.push_back(value_type(aFirst, aLast));
			return aLast;
		}
		FwdIter1 b = aFirst;
		FwdIter1 e = aDelimeterIsSubsequence ? std::search(b, aLast, aDelimeterFirst, aDelimiterLast) : std::find_first_of(b, aLast, aDelimeterFirst, aDelimiterLast);
		std::size_t tokens = 0;
		while(e != aLast && (aMaxTokens == 0 || tokens < aMaxTokens))
		{
			if (b == e && !aSkipEmptyTokens)
			{
				aTokens.push_back(value_type(b, b));
				++tokens;
			}
			else if (b != e)
			{
				aTokens.push_back(value_type(b, e));
				++tokens;
			}
			b = e;
			std::advance(b, aDelimeterIsSubsequence ? std::distance(aDelimeterFirst, aDelimiterLast) : 1);
			e = aDelimeterIsSubsequence ? std::search(b, aLast, aDelimeterFirst, aDelimiterLast) : std::find_first_of(b, aLast, aDelimeterFirst, aDelimiterLast);
		}
		if (b != e && (aMaxTokens == 0 || tokens < aMaxTokens))
		{
			aTokens.push_back(value_type(b, e));
			b = e;
		}
		return b;
	}
	
	template <typename CharT, typename Traits, typename Alloc, typename ResultContainer>
	inline void tokens(const std::basic_string<CharT, Traits, Alloc>& aLine, const std::basic_string<CharT, Traits, Alloc>& aDelimeter, ResultContainer& aTokens, std::size_t aMaxTokens = 0, bool aSkipEmptyTokens = true, bool aDelimeterIsSubsequence = false)
	{
		tokens(aLine.begin(), aLine.end(), aDelimeter.begin(), aDelimeter.end(), aTokens, aMaxTokens, aSkipEmptyTokens, aDelimeterIsSubsequence);
	}

	inline std::string to_string(const std::pair<std::string::const_iterator, std::string::const_iterator>& aIterPair)
	{
		return std::string(aIterPair.first, aIterPair.second);
	}

	template <typename CharT, typename Traits, typename Alloc>
	std::basic_string<CharT, Traits, Alloc> to_lower(const std::basic_string<CharT, Traits, Alloc>& aString)
	{
		static boost::locale::generator gen;
		static std::locale loc = gen("en_US.UTF-8");
		return boost::locale::to_lower(aString, loc);
	}

	template <typename CharT>
	CharT to_lower(CharT aCharacter)
	{
		return to_lower(std::basic_string<CharT>(1, aCharacter))[0];
	}

	template <typename CharT, typename Traits, typename Alloc>
	std::basic_string<CharT, Traits, Alloc> to_upper(const std::basic_string<CharT, Traits, Alloc>& aString)
	{
		static boost::locale::generator gen;
		static std::locale loc = gen("en_US.UTF-8");
		return boost::locale::to_upper(aString, loc);
	}

	template <typename CharT>
	CharT to_upper(CharT aCharacter)
	{
		return to_upper(std::basic_string<CharT>(1, aCharacter))[0];
	}

	struct string_span : std::pair<std::size_t, std::size_t>
	{
		typedef std::pair<std::size_t, std::size_t> span;
		typedef unsigned int type;
		string_span(const span& aSpan, type aType = 0) : span(aSpan), iType(aType) {}
		string_span(std::size_t aFirst, std::size_t aSecond, type aType = 0) : span(aFirst, aSecond), iType(aType) {}
		string_span& operator=(const span& aSpan) { span::operator=(aSpan); return *this; }
		type iType;
	};
	typedef std::vector<string_span> string_spans;

	template <typename CharT, typename Traits, typename Alloc>
	inline bool replace_string(std::basic_string<CharT, Traits, Alloc>& aString, const std::basic_string<CharT, Traits, Alloc>& aSearch, const std::basic_string<CharT, Traits, Alloc>& aReplace, string_spans* aSpans, const string_span::type* aNewSpanType)
	{
		if (aString.empty())
			return false;
		typedef std::basic_string<CharT, Traits, Alloc> string;
		typename string::size_type pos = 0;
		bool replaced = false;
		while ((pos = aString.find(aSearch, pos)) != string::npos)
		{
			aString.replace(pos, aSearch.size(), aReplace);
			if (aSpans != 0)
			{
				if (aNewSpanType && aSpans->empty())
					aSpans->push_back(string_span(pos, pos + aReplace.size(), *aNewSpanType));
				else 
				{
					for (string_spans::iterator i = aSpans->begin(); i != aSpans->end(); ++i)
					{
						if (i->first != i->second)
						{
							if (i->first >= pos)
							{
								if (aSearch.size() > aReplace.size())
									i->first -= aSearch.size() - aReplace.size();
								else
									i->first += aReplace.size() - aSearch.size();
							}
							if (i->second >= pos)
							{
								if (aSearch.size() > aReplace.size())
									i->second -= aSearch.size() - aReplace.size();
								else
									i->second += aReplace.size() - aSearch.size();
							}
						}
					}
				}
			}
			pos += aReplace.size();
			replaced = true;
		}
		return replaced;
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline bool replace_string(std::basic_string<CharT, Traits, Alloc>& aString, const std::basic_string<CharT, Traits, Alloc>& aSearch, const std::basic_string<CharT, Traits, Alloc>& aReplace)
	{
		return replace_string(aString, aSearch, aReplace, 0, static_cast<string_span::type*>(0));
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline bool replace_string(std::basic_string<CharT, Traits, Alloc>& aString, const std::basic_string<CharT, Traits, Alloc>& aSearch, const std::basic_string<CharT, Traits, Alloc>& aReplace, string_spans* aSpans)
	{
		return replace_string(aString, aSearch, aReplace, aSpans, static_cast<string_span::type*>(0));
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline bool replace_string(std::basic_string<CharT, Traits, Alloc>& aString, const std::basic_string<CharT, Traits, Alloc>& aSearch, const std::basic_string<CharT, Traits, Alloc>& aReplace, string_spans* aSpans, string_span::type aNewSpanType)
	{
		return replace_string(aString, aSearch, aReplace, aSpans, &aNewSpanType);
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline std::string& remove_leading(std::basic_string<CharT, Traits, Alloc>& aString, const std::basic_string<CharT, Traits, Alloc>& aLeading)
	{
		typename std::basic_string<CharT, Traits, Alloc>::size_type pos = aString.find_first_not_of(aLeading);
		if (pos != std::basic_string<CharT, Traits, Alloc>::npos)
			aString.erase(aString.begin(), aString.begin() + pos);
		else
			aString.clear();
		return aString;
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline std::string& remove_trailing(std::basic_string<CharT, Traits, Alloc>& aString, const std::basic_string<CharT, Traits, Alloc>& aTrailing)
	{
		typename std::basic_string<CharT, Traits, Alloc>::size_type pos = aString.find_last_not_of(aTrailing);
		if (pos != std::basic_string<CharT, Traits, Alloc>::npos)
			aString.erase(aString.begin() + pos + 1, aString.end());
		else
			aString.clear();
		return aString;
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline std::string& remove_leading_and_trailing(std::basic_string<CharT, Traits, Alloc>& aString, const std::basic_string<CharT, Traits, Alloc>& aLeadingTrailing)
	{
		remove_leading(aString, aLeadingTrailing);
		remove_trailing(aString, aLeadingTrailing);
		return aString;
	}

	template <typename CharT>
	inline bool contains_character(const CharT* aSequence, CharT aCharacter)
	{
		return strchr(aSequence, aCharacter) != NULL;
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline typename std::basic_string<CharT, Traits, Alloc>::size_type reverse_find_last_of(const std::basic_string<CharT, Traits, Alloc>& aString, const std::basic_string<CharT, Traits, Alloc>& aSequence, typename std::basic_string<CharT, Traits, Alloc>::size_type aPosition)
	{
		if (aString.empty())
			return std::basic_string<CharT, Traits, Alloc>::npos;
		typename std::basic_string<CharT, Traits, Alloc>::size_type last = std::basic_string<CharT, Traits, Alloc>::npos;
		for (;;)
		{
			if (contains_character(aSequence.c_str(), aString[aPosition]))
			{
				last = aPosition;
				if (aPosition == 0)
					break;
				--aPosition;
			}
			else
				break;
		}
		return last;
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline typename std::basic_string<CharT, Traits, Alloc>::size_type reverse_find_first_of(const std::basic_string<CharT, Traits, Alloc>& aString, const std::basic_string<CharT, Traits, Alloc>& aSequence, typename std::basic_string<CharT, Traits, Alloc>::size_type aPosition)
	{
		if (aString.empty())
			return std::basic_string<CharT, Traits, Alloc>::npos;
		if (aPosition == std::basic_string<CharT, Traits, Alloc>::npos)
			aPosition = aString.size() - 1;
		for (;;)
		{
			if (contains_character(aSequence.c_str(), aString[aPosition]))
				return aPosition;
			else if (aPosition == 0)
				return std::basic_string<CharT, Traits, Alloc>::npos;
			--aPosition;
		}
		return std::basic_string<CharT, Traits, Alloc>::npos;
	}

	template <typename CharT>
	inline long string_to_integer(const std::basic_string<CharT>& aString, int aBase = 10)
	{
		return strtol(aString.c_str(), 0, aBase);
	}

	#ifdef _MSC_VER

	template <typename CharT>
	inline long long string_to_integer_64(const std::basic_string<CharT>& aString, int aBase = 10)
	{
		return _strtoi64(aString.c_str(), 0, aBase);
	}

	#endif // _MSC_VER

	template <typename CharT>
	inline std::basic_string<CharT> integer_to_string(long aInteger, int aBase = 10, std::size_t aWidth = 0, CharT aFill = '0')
	{
		std::basic_stringstream<CharT> string;
		if (aBase == 16)
			string << std::hex << std::uppercase;
		if (aWidth != 0)
		{
			string.width(aWidth);
			string.fill(aFill);
		}
		string << aInteger;
		return string.str();
	}

	#ifdef _MSC_VER

	template <typename CharT>
	inline std::basic_string<CharT> integer_64_to_string(long long aInteger, int aBase = 10, std::size_t aWidth = 0, CharT aFill = '0')
	{
		std::basic_stringstream<CharT> string;
		switch(aBase)
		{
		case 8:
			string << std::oct;
			break;
		case 16:
			string << std::hex << std::uppercase;
			break;
		}
		if (aWidth != 0)
		{
			string.width(aWidth);
			string.fill(aFill);
		}
		string << aInteger;
		return string.str();
	}

	#endif // _MSC_VER

	template <typename CharT>
	inline unsigned long string_to_unsigned_integer(const std::basic_string<CharT>& aString, int aBase = 10)
	{
		return strtoul(aString.c_str(), 0, aBase);
	}

	#ifdef _MSC_VER

	template <typename CharT>
	inline unsigned long long string_to_unsigned_integer_64(const std::basic_string<CharT>& aString, int aBase = 10)
	{
		return _strtoui64(aString.c_str(), 0, aBase);
	}

	#endif // _MSC_VER

	template <typename CharT>
	inline std::basic_string<CharT> unsigned_integer_to_string(unsigned long aInteger, int aBase = 10, std::size_t aWidth = 0, CharT aFill = '0')
	{
		std::basic_stringstream<CharT> string;
		if (aBase == 16)
			string << std::hex << std::uppercase;
		if (aWidth != 0)
		{
			string.width(aWidth);
			string.fill(aFill);
		}
		string << aInteger;
		return string.str();
	}

	#ifdef _MSC_VER

	template <typename CharT>
	inline std::basic_string<CharT> unsigned_integer_64_to_string(unsigned long long aInteger, int aBase = 10, std::size_t aWidth = 0, CharT aFill = '0')
	{
		std::basic_stringstream<CharT> string;
		if (aBase == 16)
			string << std::hex << std::uppercase;
		if (aWidth != 0)
		{
			string.width(aWidth);
			string.fill(aFill);
		}
		string << aInteger;
		return string.str();
	}

	#endif // _MSC_VER

	template <typename CharT>
	inline double string_to_double(const std::basic_string<CharT>& aString)
	{
		return strtod(aString.c_str(), 0);
	}

	template <typename CharT>
	inline std::basic_string<CharT> double_to_string(double aDouble, std::size_t aPrecision = 0, bool aFixed = true, std::size_t aWidth = 0, CharT aFill = '0')
	{
		std::basic_stringstream<CharT> string;
		if (aPrecision != 0)
			string.precision(aPrecision);
		if (aFixed)
			string << std::fixed;
		else
			string << std::scientific;
		if (aWidth != 0)
		{
			string.width(aWidth);
			string.fill(aFill);
		}
		string << aDouble;
		return string.str();
	}

	inline std::string parse_escapes(const std::string& aString)
	{
		std::string ret = aString;
		std::string::size_type escapePos;
		while((escapePos = ret.find("\\r")) != std::string::npos)
			ret.replace(escapePos, 2, "\r");
		while((escapePos = ret.find("\\n")) != std::string::npos)
			ret.replace(escapePos, 2, "\n");
		while((escapePos = ret.find("\\t")) != std::string::npos)
			ret.replace(escapePos, 2, "\t");
		return ret;
	}

	inline std::string parse_url_escapes(const std::string& aString)
	{
		std::string ret = aString;
		for (std::string::size_type pos = 0; pos != ret.length(); ++pos)
			if (ret[pos] == '%' && pos + 2 < ret.length())
				ret.replace(pos, 3, 1, static_cast<char>(string_to_integer(ret.substr(pos + 1, 2), 16)));
		return ret;
	}

	namespace detail
	{
		template<typename CharT>
		inline CharT wildcard_match_any_string() { return '*'; }
		template<>
		inline char16_t wildcard_match_any_string<char16_t>() { return L'*'; }
		template<typename CharT>
		inline CharT wildcard_match_any_character() { return '?'; }
		template<>
		inline char16_t wildcard_match_any_character<char16_t>() { return L'?'; }

		template <typename Traits>
		struct wildcard_compare
		{
			typedef typename Traits::char_type char_type;
			bool operator()(char_type c1, char_type c2) const
			{
				if (c2 == wildcard_match_any_character<char_type>())
					return true;
				else
					return Traits::eq(c1, c2);
			}
		};
	}

	template <typename CharT, typename Traits, typename FwdIter>
	bool do_wildcard_match(FwdIter aTextBegin, FwdIter aTextEnd, FwdIter aPatternBegin, FwdIter aPatternEnd)
	{
		typedef std::pair<FwdIter, FwdIter> substring_t;
		typedef std::vector<substring_t> substrings_t;
		substrings_t substrings;
		CharT any_string = detail::wildcard_match_any_string<CharT>();
		neolib::tokens(aPatternBegin, aPatternEnd, &any_string, &any_string+1, substrings);

		FwdIter previousMatch = aTextBegin;
		for (typename substrings_t::const_iterator i = substrings.begin(); i != substrings.end();)
		{
			substring_t theSubstring = *i++;
			FwdIter nextMatch = std::search(previousMatch, aTextEnd, theSubstring.first, theSubstring.second, detail::wildcard_compare<Traits>());
			if (nextMatch == aTextEnd)
				return false;
			if (theSubstring.first == aPatternBegin && nextMatch != aTextBegin)
				return false;
			if (theSubstring.second == aPatternEnd && !std::equal(aTextEnd - (theSubstring.second - theSubstring.first), aTextEnd, theSubstring.first, detail::wildcard_compare<Traits>()))
				return false;
			if (theSubstring.second == aPatternEnd && aTextEnd - nextMatch != theSubstring.second - theSubstring.first)
				return false;
			previousMatch = nextMatch + (theSubstring.second - theSubstring.first);
		}
		return true;
	}

	template <typename CharT, typename FwdIter>
	bool wildcard_match(FwdIter aTextBegin, FwdIter aTextEnd, FwdIter aPatternBegin, FwdIter aPatternEnd)
	{
		return do_wildcard_match<CharT, std::char_traits<CharT>, FwdIter>(aTextBegin, aTextEnd, aPatternBegin, aPatternEnd);
	}

	template <typename CharT, typename Traits, typename Alloc>
	bool wildcard_match(const std::basic_string<CharT, Traits, Alloc>& aText, const std::basic_string<CharT, Traits, Alloc>& aPattern)
	{
		return do_wildcard_match<CharT, Traits, std::basic_string<CharT, Traits, Alloc>::const_iterator>(aText.begin(), aText.end(), aPattern.begin(), aPattern.end());
	}

	typedef char32_t unicode_char_t;
	const char INVALID_CHAR8 = '?';
	const unicode_char_t INVALID_CHAR32 = static_cast<unicode_char_t>(0xFFFD);

	namespace utf16
	{
		inline bool is_high_surrogate(unicode_char_t aCharacter)
		{
			return aCharacter >= 0xD800 && aCharacter <= 0xDBFF;
		}
		inline bool is_low_surrogate(unicode_char_t aCharacter)
		{
			return aCharacter >= 0xDC00 && aCharacter <= 0xDFFF;
		}
		inline bool is_surrogate_pair(unicode_char_t aHighValue, unicode_char_t aLowValue)
		{
			return is_high_surrogate(aHighValue) && is_low_surrogate(aLowValue);
		}
	}

	inline std::size_t append_utf8(std::string& aString, unicode_char_t aCharacter)
	{
		if (aCharacter <= 0x7F)
		{
			aString.append(1, static_cast<char>(aCharacter));
			return 1;
		}
		else if (aCharacter <= 0x7FF)
		{
			aString.append(1, static_cast<char>(((aCharacter >> 6) & 0x1F) | 0xC0));
			aString.append(1, static_cast<char>((aCharacter & 0x3F) | 0x80));
			return 2;
		}
		else if (aCharacter <= 0xFFFF)
		{
			aString.append(1, static_cast<char>(((aCharacter >> 12) & 0x0F) | 0xE0));
			aString.append(1, static_cast<char>(((aCharacter >> 6 ) & 0x3F) | 0x80));
			aString.append(1, static_cast<char>((aCharacter& 0x3F) | 0x80));
			return 3;
		}
		else if (aCharacter <= 0x10FFFF)
		{
			aString.append(1, static_cast<char>(((aCharacter >> 18) & 0x07) | 0xF0));
			aString.append(1, static_cast<char>(((aCharacter >> 12 ) & 0x3F) | 0x80));
			aString.append(1, static_cast<char>(((aCharacter >> 6 ) & 0x3F) | 0x80));
			aString.append(1, static_cast<char>((aCharacter& 0x3F) | 0x80));
			return 4;
		}
		else
		{
			aString.append(1, INVALID_CHAR8);
			return 1;
		}
	}

	typedef std::map<std::string::size_type, std::u16string::size_type> utf16_to_utf8_character_map;

	namespace detail
	{
		struct character_map_updater
		{
			struct short_narrow_string : std::logic_error {	short_narrow_string() : std::logic_error("neolib::detail::character_map_updater::short_narrow_string") {} };
			utf16_to_utf8_character_map& iCharMap;
			character_map_updater(utf16_to_utf8_character_map& aCharMap) : iCharMap(aCharMap) {}
			void operator()(std::u16string::size_type aFrom, bool aSurrogatePair, const std::string& aNarrowString, std::string::size_type aNumberAdded)
			{
				for (std::string::size_type i = 0; i < aNumberAdded; ++i)
					iCharMap[aNarrowString.size() - aNumberAdded + i] = aFrom;
				if (aSurrogatePair && aNarrowString.size())
				{
					if (aNarrowString.size() <= 1)
						throw short_narrow_string();
					iCharMap[aNarrowString.size() - 1] = aFrom + 1;
				}
			}
		};			
		struct no_character_map_updater
		{
			no_character_map_updater() {}
			void operator()(std::u16string::size_type, bool, const std::string&, std::string::size_type) {}
		};			
	}

	template <bool AllowUpper128, typename CharacterMapUpdater>
	inline std::string utf16_to_utf8(const std::u16string& aString, CharacterMapUpdater aCharacterMapUpdater)
	{
		bool previousWasUtf8Prefix = false;
		std::string narrowString;
		std::u16string::size_type from = 0;
		for (std::u16string::const_iterator i = aString.begin(); i != aString.end(); from = i - aString.begin())
		{
			bool sequenceCheck = previousWasUtf8Prefix;
			previousWasUtf8Prefix = false;
			unicode_char_t uch = *i++;
			bool surrogatePair = false;
			if (utf16::is_high_surrogate(uch) && i != aString.end() && utf16::is_surrogate_pair(uch, *i))
			{
				uch = ((uch & 0x3FF) << 10);
				uch = uch | (*i++ & 0x3FF);
				uch += 0x10000;
				surrogatePair = true;
			}
			else if (AllowUpper128)
			{
				int narrowChar = wctob(static_cast<wint_t>(uch)); 
				if (narrowChar != static_cast<int>(EOF) && narrowChar != static_cast<int>(WEOF) && static_cast<unsigned char>(narrowChar) > 0x7Fu)
				{
					unsigned char nch = static_cast<unsigned char>(narrowChar);
					if ((nch & 0xE0) == 0xC0 || (nch & 0xF0) == 0xE0 || (nch & 0xF8) == 0xF0)
					{
						previousWasUtf8Prefix = true;
					}
					else if (sequenceCheck && (nch & 0xC0) == 0x80)
					{
						int previousNarrowChar = static_cast<int>(narrowString[narrowString.size()-1]);
						narrowString.erase(narrowString.size() - 1);
						aCharacterMapUpdater(from, surrogatePair, narrowString, append_utf8(narrowString, static_cast<unicode_char_t>(btowc(previousNarrowChar))));
					}
					narrowString.append(1, static_cast<char>(narrowChar));
					aCharacterMapUpdater(from, surrogatePair, narrowString, 1);
					continue;
				}
			}
			aCharacterMapUpdater(from, surrogatePair, narrowString, append_utf8(narrowString, uch));
		}
		return narrowString;
	}

	template <bool AllowUpper128>
	inline std::string utf16_to_utf8(const std::u16string& aString)
	{
		return utf16_to_utf8<AllowUpper128>(aString, detail::no_character_map_updater());
	}

	template <bool AllowUpper128>
	inline std::string utf16_to_utf8(const std::u16string& aString, utf16_to_utf8_character_map& aCharMap)
	{
		return utf16_to_utf8<AllowUpper128>(aString, detail::character_map_updater(aCharMap));
	}

	inline std::string utf16_to_utf8(const std::u16string& aString)
	{
		return utf16_to_utf8<false>(aString);
	}

	inline std::string utf16_to_utf8(const std::u16string& aString, utf16_to_utf8_character_map& aCharMap)
	{
		return utf16_to_utf8<false>(aString, aCharMap);
	}

	namespace detail
	{
		template <typename FwdIter>
		inline unicode_char_t next_utf_bits(unicode_char_t aUnicodeChar, std::size_t aCount, FwdIter& aCurrent, FwdIter aEnd)
		{
			if (aUnicodeChar == 0 || (aUnicodeChar == 1 && aCount == 1)) // overlong sequences including encoding ASCII as 2-byte UTF-8
				return INVALID_CHAR32;
			unicode_char_t unicodeChar = aUnicodeChar;
			FwdIter start = aCurrent;
			for (std::size_t i = 0; i != aCount; ++i)
			{
				++aCurrent;
				if (aCurrent == aEnd)
				{
					aCurrent = start;
					return INVALID_CHAR32;
				}
				unsigned char nch = static_cast<unsigned char>(*aCurrent);
				if ((nch & 0xC0) == 0x80)
					unicodeChar = (unicodeChar << 6) | (static_cast<unicode_char_t>(nch & ~0xC0));
				else
				{
					aCurrent = start;
					return INVALID_CHAR32;
				}
			}
			return unicodeChar;
		}
		inline void default_utf16_conversion_callback(std::string::size_type /*aFrom*/, std::u16string::size_type /*aTo*/) {}
		inline void default_utf32_conversion_callback(std::string::size_type /*aFrom*/, std::u32string::size_type /*aTo*/) {}
	}

	template <typename Callback>
	inline std::u16string utf8_to_utf16(const std::string& aString, Callback aCallback, bool aCodePageFallback = false)
	{
		std::u16string utf16String;
		for (std::string::const_iterator i = aString.begin(); i != aString.end();)
		{
			aCallback(i - aString.begin(), utf16String.size());
			unsigned char nch = static_cast<unsigned char>(*i);
			unicode_char_t uch = 0;
			if ((nch & 0x80) == 0)
				uch = static_cast<unicode_char_t>(nch & 0x7F);
			else
			{
				std::string::const_iterator old = i;
				if ((nch & 0xE0) == 0xC0)
					uch = detail::next_utf_bits(static_cast<unicode_char_t>(nch & ~0xE0), 1, i, aString.end());
				else if ((nch & 0xF0) == 0xE0)
					uch = detail::next_utf_bits(static_cast<unicode_char_t>(nch & ~0xF0), 2, i, aString.end());
				else if ((nch & 0xF8) == 0xF0)
					uch = detail::next_utf_bits(static_cast<unicode_char_t>(nch & ~0xF8), 3, i, aString.end());
				else
					uch = INVALID_CHAR32;
				if (i == old && aCodePageFallback)
				{
					wchar_t wch;
					std::mbstate_t state = std::mbstate_t{};
					if (std::mbrtowc(&wch, reinterpret_cast<char*>(&nch), 1, &state) == 1)
						uch = static_cast<unicode_char_t>(wch);
					else
						uch = static_cast<unicode_char_t>(nch);
				}
			}
		
			if (uch <= 0xFFFF)
				utf16String.append(1, static_cast<char16_t>(uch));
			else
			{
				// UTF-16 bit...
				uch -= 0x10000;
				utf16String.append(1, static_cast<char16_t>(0xd800|(uch >> 10)));
				utf16String.append(1, static_cast<char16_t>(0xdc00|(uch & 0x3FF)));
			}

			if (i != aString.end())
				++i;
		}
		return utf16String;
	}

	inline std::u16string utf8_to_utf16(const std::string& aString, bool aCodePageFallback = false)
	{
		return utf8_to_utf16(aString, detail::default_utf16_conversion_callback, aCodePageFallback);
	}

	template <typename Callback>
	inline std::u32string utf8_to_utf32(std::string::const_iterator aBegin, std::string::const_iterator aEnd, Callback aCallback, bool aCodePageFallback = false)
	{
		std::u32string utf32String;
		for (std::string::const_iterator i = aBegin; i != aEnd;)
		{
			aCallback(i - aBegin, utf32String.size());

			unsigned char nch = static_cast<unsigned char>(*i);
			unicode_char_t uch = 0;
			if ((nch & 0x80) == 0)
				uch = static_cast<unicode_char_t>(nch & 0x7F);
			else
			{
				std::string::const_iterator old = i;
				if ((nch & 0xE0) == 0xC0)
					uch = detail::next_utf_bits(static_cast<unicode_char_t>(nch & ~0xE0), 1, i, aEnd);
				else if ((nch & 0xF0) == 0xE0)
					uch = detail::next_utf_bits(static_cast<unicode_char_t>(nch & ~0xF0), 2, i, aEnd);
				else if ((nch & 0xF8) == 0xF0)
					uch = detail::next_utf_bits(static_cast<unicode_char_t>(nch & ~0xF8), 3, i, aEnd);
				else
					uch = INVALID_CHAR32;
				if (i == old && aCodePageFallback)
				{
					char32_t ch32;
					std::mbstate_t state = std::mbstate_t{};
					if (std::mbrtoc32(&ch32, reinterpret_cast<char*>(&nch), 1, &state) == 1)
						uch = static_cast<unicode_char_t>(ch32);
					else
						uch = static_cast<unicode_char_t>(nch);
				}
			}

			utf32String.append(1, uch);

			if (i != aEnd)
				++i;
		}
		return utf32String;
	}

	template <typename Callback>
	inline std::u32string utf8_to_utf32(const std::string& aString, Callback aCallback, bool aCodePageFallback = false)
	{
		return utf8_to_utf32(aString.begin(), aStirng.end(), aCallBack, aCodePageFallback);
	}

	inline std::u32string utf8_to_utf32(std::string::const_iterator aBegin, std::string::const_iterator aEnd, bool aCodePageFallback = false)
	{
		return utf8_to_utf32(aBegin, aEnd, detail::default_utf32_conversion_callback, aCodePageFallback);
	}

	inline std::u32string utf8_to_utf32(const std::string& aString, bool aCodePageFallback = false)
	{
		return utf8_to_utf32(aString.begin(), aString.end(), aCodePageFallback);
	}

	inline std::string utf32_to_utf8(const std::u32string& aString)
	{
		std::string result;
		for (auto ch : aString)
			append_utf8(result, ch);
		return result;
	}

	inline bool is_utf8_trailing(char aCharacter)
	{
		return (aCharacter & 0xC0) == 0x80;
	}

	template <typename StringT>
	inline StringT utf16_to_any(const std::u16string& aString)
	{
		return utf16_to_utf8(aString);
	}

	template <>
	inline std::u16string utf16_to_any<std::u16string>(const std::u16string& aString)
	{
		return aString;
	}

	template <typename StringT>
	inline StringT utf8_to_any(const std::string& aString, bool aCodePageFallback = false)
	{
		return utf8_to_utf16(aString, aCodePageFallback);
	}

	template <>
	inline std::string utf8_to_any<std::string>(const std::string& aString, bool)
	{
		return aString;
	}

	inline std::u16string any_to_utf16(const std::string& aString, bool aCodePageFallback = false)
	{
		return utf8_to_utf16(aString, aCodePageFallback);
	}

	inline const std::string& any_to_utf8(const std::string& aString)
	{
		return aString;
	}

	inline std::string any_to_utf8(const std::u16string& aString)
	{
		return utf16_to_utf8(aString);
	}

	inline const std::u16string& any_to_utf16(const std::u16string& aString)
	{
		return aString;
	}

	template <typename StringT>
	class any_to_utf16_result
	{
	public:
		any_to_utf16_result(const typename StringT::value_type* aString, typename StringT::size_type aStringLength, bool aCodePageFallback = false) :
			iString(utf8_to_utf16(StringT(aString, aStringLength), aCodePageFallback))
		{
		}
	public:
		const char16_t* data() const
		{
			return iString.data();
		}
		std::u16string::size_type length() const
		{
			return iString.length();
		}
	private:
		const std::u16string iString;
	};

	template <>
	class any_to_utf16_result<std::u16string>
	{
	public:
		any_to_utf16_result(const char16_t* aString, std::u16string::size_type aStringLength) :
			iString(aString),
			iStringLength(aStringLength)
		{
		}
	public:
		const char16_t* data() const
		{
			return iString;
		}
		std::u16string::size_type length() const
		{
			return iStringLength;
		}
	private:
		const char16_t* iString;
		std::u16string::size_type iStringLength;
	};

	inline any_to_utf16_result<std::string> any_to_utf16(const std::string::value_type* aString, std::string::size_type aStringLength)
	{
		return any_to_utf16_result<std::string>(aString, aStringLength);
	}

	inline  any_to_utf16_result<std::u16string> any_to_utf16(const std::u16string::value_type* aString, std::u16string::size_type aStringLength)
	{
		return any_to_utf16_result<std::u16string>(aString, aStringLength);
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline std::string utf16_to_narrow(const std::basic_string<CharT, Traits, Alloc>& aWideString)
	{
		std::vector<char> narrowString;
		narrowString.resize(aWideString.size() + 1);
		wcstombs(&narrowString[0], aWideString.c_str(), aWideString.size() + 1);
		return std::string(&narrowString[0]);
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline std::u16string narrow_to_utf16(const std::basic_string<CharT, Traits, Alloc>& aNarrowString)
	{
		std::vector<char16_t> utf16String;
		utf16String.resize(aNarrowString.size() + 1);
		mbstowcs(&utf16String[0], aNarrowString.c_str(), aNarrowString.size() + 1);
		return std::u16string(&utf16String[0]);
	}

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
