// string_utf.hpp
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
#include <neolib/core/string.hpp>

namespace neolib 
{
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
            struct short_narrow_string : std::logic_error {    short_narrow_string() : std::logic_error("neolib::detail::character_map_updater::short_narrow_string") {} };
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
                if (nch == 0xC0 || nch == 0xC1)
                {
                    aCurrent = start;
                    return INVALID_CHAR32;
                }
                if ((nch & 0xC0) == 0x80)
                    unicodeChar = (unicodeChar << 6) | (static_cast<unicode_char_t>(nch & ~0xC0));
                else
                {
                    aCurrent = start;
                    return INVALID_CHAR32;
                }
            }
            static const unicode_char_t sMaxCodePoint[] = { 0x7F, 0x7FF, 0xFFFF, 0x10FFFF };
            if (unicodeChar <= sMaxCodePoint[aCount - 1]) // overlong sequences
            {
                aCurrent = start;
                return INVALID_CHAR32;
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
                if (nch == 0xC0 || nch == 0xC1)
                    uch = INVALID_CHAR32;
                else if ((nch & 0xE0) == 0xC0)
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
        for (auto i = aBegin; i != aEnd; ++i)
        {
            aCallback(i - aBegin, utf32String.size());

            unsigned char nch = static_cast<unsigned char>(*i);
            unicode_char_t uch = 0;
            if ((nch & 0x80) == 0)
                uch = static_cast<unicode_char_t>(nch & 0x7F);
            else
            {
                auto old = i;
                if (nch == 0xC0 || nch == 0xC1)
                    uch = INVALID_CHAR32;
                else if ((nch & 0xE0) == 0xC0)
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
        }
        return utf32String;
    }

    template <typename Callback>
    inline std::u32string utf8_to_utf32(const std::string& aString, Callback aCallback, bool aCodePageFallback = false)
    {
        return utf8_to_utf32(aString.begin(), aString.end(), aCallback, aCodePageFallback);
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

    template <typename CharT, typename Traits>
    inline bool check_utf8(const std::basic_string_view<CharT, Traits>& aString)
    {
        auto end = aString.end();
        for (auto i = aString.begin(); i != end; ++i)
        {
            unsigned char nch = static_cast<unsigned char>(*i);
            unicode_char_t uch = 0;
            if ((nch & 0x80) == 0)
                uch = static_cast<unicode_char_t>(nch & 0x7F);
            else
            {
                auto old = i;
                if (nch == 0xC0 || nch == 0xC1)
                    uch = INVALID_CHAR32;
                else if ((nch & 0xE0) == 0xC0)
                    uch = detail::next_utf_bits(static_cast<unicode_char_t>(nch & ~0xE0), 1, i, end);
                else if ((nch & 0xF0) == 0xE0)
                    uch = detail::next_utf_bits(static_cast<unicode_char_t>(nch & ~0xF0), 2, i, end);
                else if ((nch & 0xF8) == 0xF0)
                    uch = detail::next_utf_bits(static_cast<unicode_char_t>(nch & ~0xF8), 3, i, end);
                else
                    uch = INVALID_CHAR32;
                if (i == old || uch == INVALID_CHAR32)
                    return false;
            }
        }
        return true;
    }

    template <typename CharT, typename Traits, typename Alloc>
    inline bool check_utf8(const std::basic_string<CharT, Traits, Alloc>& aString)
    {
        return check_utf8(std::basic_string_view<CharT, Traits>{ aString });
    }

    inline bool check_utf8(const i_string& aString)
    {
        return check_utf8(aString.to_std_string_view());
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
}
