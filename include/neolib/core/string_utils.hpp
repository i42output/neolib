// string_utils.hpp
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

#include <neolib/neolib.hpp>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <variant>
#include <locale>
#include <format>
#include <neolib/core/string.hpp>
#include <neolib/core/string_numeric.hpp>
#include <neolib/core/string_utf.hpp>

namespace neolib 
{
    struct NEOLIB_EXPORT comma_as_whitespace : std::ctype<char>
    {
        static const mask* make_table()
        {
            static std::vector<mask> v(classic_table(), classic_table() + table_size);
            v[','] |= space;
            return &v[0];
        }
        comma_as_whitespace(std::size_t refs = 0) : ctype{ make_table(), false, refs } {}
    };

    struct NEOLIB_EXPORT comma_and_brackets_as_whitespace : std::ctype<char>
    {
        static const mask* make_table()
        {
            static std::vector<mask> v(classic_table(), classic_table() + table_size);
            v[','] |= space;
            v['['] |= space;
            v[']'] |= space;
            v['('] |= space;
            v[')'] |= space;
            v['{'] |= space;
            v['}'] |= space;
            return &v[0];
        }
        comma_and_brackets_as_whitespace(std::size_t refs = 0) : ctype{ make_table(), false, refs } {}
    };

    struct NEOLIB_EXPORT comma_only_whitespace : std::ctype<char>
    {
        static const mask* make_table()
        {
            static std::vector<mask> v(classic_table(), classic_table() + table_size);
            v[','] |= space;
            v[' '] = alpha;
            return &v[0];
        }
        comma_only_whitespace(std::size_t refs = 0) : ctype{ make_table(), false, refs } {}
    };

    template <typename T>
    inline std::string to_std_string(T const& aValue)
    {
        std::ostringstream oss;
        oss << aValue;
        return oss.str();
    }

    template <typename T>
    inline T from_std_string(std::string const& aValueAsString)
    {
        T result;
        std::istringstream iss{ aValueAsString };
        iss >> result;
        return result;

    }

    template <typename T>
    inline string to_string(T const& aValue)
    {
        return to_std_string(aValue);
    }

    template <typename T>
    inline T from_string(i_string const& aValueAsString)
    {
        return from_std_string<T>(aValueAsString.to_std_string());
    }

    template <typename FwdIter1, typename FwdIter2, typename ResultContainer>
    inline FwdIter1 tokens(FwdIter1 aFirst, FwdIter1 aLast, FwdIter2 aDelimeterFirst, FwdIter2 aDelimiterLast, ResultContainer& aTokens, std::size_t aMaxTokens = 0, bool aSkipEmptyTokens = true, bool aDelimeterIsSubsequence = false)
    {
        if (aFirst == aLast)
            return aFirst;
        typedef typename ResultContainer::value_type value_type;
        if (aDelimeterFirst == aDelimiterLast)
        {
            aTokens.push_back(value_type{ aFirst, aLast });
            return aLast;
        }
        FwdIter1 b = aFirst;
        FwdIter1 e = aDelimeterIsSubsequence ? std::search(b, aLast, aDelimeterFirst, aDelimiterLast) : std::find_first_of(b, aLast, aDelimeterFirst, aDelimiterLast);
        std::size_t tokens = 0;
        std::optional<FwdIter1> last;
        while(e != aLast && (aMaxTokens == 0 || tokens < aMaxTokens))
        {
            if (b == e && !aSkipEmptyTokens)
            {
                aTokens.push_back(value_type{ b, b });
                last = b;
                ++tokens;
            }
            else if (b != e)
            {
                aTokens.push_back(value_type{ b, e });
                last = e;
                ++tokens;
            }
            b = e;
            std::advance(b, aDelimeterIsSubsequence ? std::distance(aDelimeterFirst, aDelimiterLast) : 1);
            e = aDelimeterIsSubsequence ? std::search(b, aLast, aDelimeterFirst, aDelimiterLast) : std::find_first_of(b, aLast, aDelimeterFirst, aDelimiterLast);
        }
        if (b != e && (aMaxTokens == 0 || tokens < aMaxTokens))
        {
            aTokens.push_back(value_type{ b, e });
            b = e;
        }
        else if (b == e && last && last.value() != e && !aSkipEmptyTokens)
            aTokens.push_back(value_type{ e, e });
        return b;
    }
    
    template <typename FwdIter, typename CharT, typename Traits, typename Alloc, typename ResultContainer>
    inline void tokens(FwdIter aFirst, FwdIter aLast, const std::basic_string<CharT, Traits, Alloc>& aDelimeter, ResultContainer& aTokens, std::size_t aMaxTokens = 0, bool aSkipEmptyTokens = true, bool aDelimeterIsSubsequence = false)
    {
        tokens(aFirst, aLast, aDelimeter.begin(), aDelimeter.end(), aTokens, aMaxTokens, aSkipEmptyTokens, aDelimeterIsSubsequence);
    }

    template <typename CharT, typename Traits, typename Alloc, typename ResultContainer>
    inline void tokens(const std::basic_string<CharT, Traits, Alloc>& aLine, const std::basic_string<CharT, Traits, Alloc>& aDelimeter, ResultContainer& aTokens, std::size_t aMaxTokens = 0, bool aSkipEmptyTokens = true, bool aDelimeterIsSubsequence = false)
    {
        tokens(aLine.begin(), aLine.end(), aDelimeter.begin(), aDelimeter.end(), aTokens, aMaxTokens, aSkipEmptyTokens, aDelimeterIsSubsequence);
    }

    template <typename FwdIter, typename CharT, typename Traits, typename Alloc>
    inline std::vector<std::basic_string<CharT, Traits, Alloc>> tokens(FwdIter aFirst, FwdIter aLast, const std::basic_string<CharT, Traits, Alloc>& aDelimeter, std::size_t aMaxTokens = 0, bool aSkipEmptyTokens = true, bool aDelimeterIsSubsequence = false)
    {
        std::vector<std::basic_string<CharT, Traits, Alloc>> results;
        tokens(aFirst, aLast, aDelimeter.begin(), aDelimeter.end(), results, aMaxTokens, aSkipEmptyTokens, aDelimeterIsSubsequence);
        return results;
    }

    template <typename CharT, typename Traits, typename Alloc>
    inline std::vector<std::basic_string<CharT, Traits, Alloc>> tokens(const std::basic_string<CharT, Traits, Alloc>& aLine, const std::basic_string<CharT, Traits, Alloc>& aDelimeter, std::size_t aMaxTokens = 0, bool aSkipEmptyTokens = true, bool aDelimeterIsSubsequence = false)
    {
        std::vector<std::basic_string<CharT, Traits, Alloc>> results;
        tokens(aLine.begin(), aLine.end(), aDelimeter.begin(), aDelimeter.end(), results, aMaxTokens, aSkipEmptyTokens, aDelimeterIsSubsequence);
        return results;
    }

    inline std::string to_string(const std::pair<char const*, char const*>& aIterPair)
    {
        return std::string(aIterPair.first, aIterPair.second);
    }

    template <typename CharT, typename Traits, typename Alloc>
    inline std::basic_string<CharT, Traits, Alloc> to_lower(const std::basic_string<CharT, Traits, Alloc>& aString)
    {
        static boost::locale::generator gen;
        static std::locale loc = gen("en_US.UTF-8");
        return boost::locale::to_lower(aString, loc);
    }

    template <typename CharT>
    inline CharT to_lower(CharT aCharacter)
    {
        return to_lower(std::basic_string<CharT>(1, aCharacter))[0];
    }

    template <typename CharT, typename Traits, typename Alloc>
    inline std::basic_string<CharT, Traits, Alloc> to_upper(const std::basic_string<CharT, Traits, Alloc>& aString)
    {
        static boost::locale::generator gen;
        static std::locale loc = gen("en_US.UTF-8");
        return boost::locale::to_upper(aString, loc);
    }

    template <typename CharT>
    inline CharT to_upper(CharT aCharacter)
    {
        return to_upper(std::basic_string<CharT>(1, aCharacter))[0];
    }

    struct NEOLIB_EXPORT string_span : std::pair<std::size_t, std::size_t>
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
                ret.replace(pos, 3, 1, static_cast<char>(string_to_int32(ret.substr(pos + 1, 2), 16)));
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
    inline bool do_wildcard_match(FwdIter aTextBegin, FwdIter aTextEnd, FwdIter aPatternBegin, FwdIter aPatternEnd)
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
    inline bool wildcard_match(FwdIter aTextBegin, FwdIter aTextEnd, FwdIter aPatternBegin, FwdIter aPatternEnd)
    {
        return do_wildcard_match<CharT, std::char_traits<CharT>, FwdIter>(aTextBegin, aTextEnd, aPatternBegin, aPatternEnd);
    }

    template <typename CharT, typename Traits, typename Alloc>
    inline bool wildcard_match(const std::basic_string<CharT, Traits, Alloc>& aText, const std::basic_string<CharT, Traits, Alloc>& aPattern)
    {
        return do_wildcard_match<CharT, Traits, std::basic_string<CharT, Traits, Alloc>::const_iterator>(aText.begin(), aText.end(), aPattern.begin(), aPattern.end());
    }

    class string_search_fsa
    {
    private:
        typedef std::function<void(char const*, char const*)> action_t;
        struct state
        {
            std::map<char, state> match;
            std::list<action_t> actions;
        };
        typedef std::tuple<const action_t*, char const*, char const*> result_t;
        typedef std::set<result_t> results_t;
    public:
        string_search_fsa();
    public:
        void add_pattern(std::string const& aPattern, action_t aAction);
        void search(std::string const& aText, bool aRemoveSubmatches = true) const;
    private:
        void search(state const& aState, char const* aStart, char const* aNext, char const* aEnd, bool aSearchingWildcard, results_t& aResults) const;
        void rebuild();
    private:
        std::map<std::string, action_t> iPatterns;
        state iRoot;
    };

    struct format_result
    {
        struct formatted_arg
        {
            std::size_t arg;
            std::ptrdiff_t begin;
            std::ptrdiff_t end;

            constexpr std::strong_ordering operator<=>(formatted_arg const&) const noexcept = default;
        };
        typedef std::vector<formatted_arg> arg_list;

        std::string text;
        arg_list args;

        arg_list::const_iterator find_arg(std::size_t aArgIndex) const
        {
            return std::find_if(args.begin(), args.end(), [&](auto const& a) { return a.arg == aArgIndex; });
        }
        bool has_arg(std::size_t aArgIndex) const
        {
            return find_arg(aArgIndex) != args.end();
        }
        std::string_view arg_span(std::size_t aArgIndex) const
        {
            auto existing = find_arg(aArgIndex);
            if (existing != args.end())
                return { std::next(text.begin(), existing->begin), std::next(text.begin(), existing->end) };
            throw std::format_error("neolib::format_result");
        }
    };

    template <typename... Args>
    inline format_result format(std::string_view const& aFormat, Args&&... aArgs)
    {
        format_result result;
        auto next = aFormat.begin();
        while (next != aFormat.end())
        {
            auto nextArg = std::find(next, aFormat.end(), '{');
            if (nextArg == aFormat.end())
            {
                result.text.append(next, nextArg);
                next = nextArg;
            }
            else if (std::next(nextArg) == aFormat.end())
                throw std::format_error("neolib::format");
            else if (*std::next(nextArg) == '{')
            {
                result.text.append(next, std::next(nextArg, 2));
                next = std::next(nextArg, 2);
            }
            else
            {
                result.text.append(next, nextArg);
                auto nextArgEnd = std::find(nextArg, aFormat.end(), '}');
                if (nextArgEnd == aFormat.end())
                    throw std::format_error("neolib::format");
                else
                {
                    // todo: add support for omission of arg-id in format string
                    ++nextArgEnd;
                    next = nextArgEnd;
                    auto argIdTerminators = { ':', '}' };
                    auto argIdEnd = std::find_first_of(nextArg, nextArgEnd, argIdTerminators.begin(), argIdTerminators.end());
                    std::optional<std::size_t> argId;
                    if (argIdEnd - nextArg >= 2)
                        argId = static_cast<std::size_t>(std::stoi(std::string{ std::next(nextArg), argIdEnd }));
                    if (argId)
                        result.args.emplace_back(argId.value(), result.text.size());
                    result.text.append(std::vformat(std::string_view{ nextArg, nextArgEnd }, std::make_format_args(std::forward<Args>(aArgs)...)));
                    if (argId)
                        result.args.back().end = result.text.size();
                }
            }
        }
        return result;
    }
}
