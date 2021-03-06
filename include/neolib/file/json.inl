// json.inl
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
#include <variant>
#include <unordered_map>
#include <fstream>
#include <iomanip>
#include <type_traits>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
#include <neolib/core/string_numeric.hpp>
#include <neolib/core/string_utf.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/core/type_traits.hpp>

namespace neolib
{
    namespace json_detail
    {
#ifdef DEBUG_JSON
        constexpr bool debug = true;
#else
        constexpr bool debug = false;
#endif
        enum class keyword
        {
            True,
            False,
            Null
        };

        enum class token : uint8_t
        {
            Invalid,
            OpenObject,
            CloseObject,
            OpenArray,
            CloseArray,
            Colon,
            Comma,
            Quote,
            Character,
            Escape,
            EscapingUnicode,
            Escaped,
            Plus,
            Minus,
            Digit,
            HexDigit,
            EscapedOrHexDigit,
            DecimalPoint,
            Exponent,
            Asterisk,
            ForwardSlash,
            Symbol,
            Space,
            Whitespace,
            EndOfInput,
            TOKEN_COUNT
        };
        constexpr std::size_t TOKEN_COUNT = static_cast<std::size_t>(token::TOKEN_COUNT);
        constexpr token TXXX = token::Invalid;
        constexpr token TOBJ = token::OpenObject;
        constexpr token TCLO = token::CloseObject;
        constexpr token TARR = token::OpenArray;
        constexpr token TCLA = token::CloseArray;
        constexpr token TCOL = token::Colon;
        constexpr token TCOM = token::Comma;
        constexpr token TQOT = token::Quote;
        constexpr token TCHA = token::Character;
        constexpr token TESC = token::Escape;
        constexpr token TESU = token::EscapingUnicode;
        constexpr token TECH = token::Escaped;
        constexpr token TPLU = token::Plus;
        constexpr token TMIN = token::Minus;
        constexpr token TDIG = token::Digit;
        constexpr token THEX = token::HexDigit;
        constexpr token TEHX = token::EscapedOrHexDigit;
        constexpr token TDEC = token::DecimalPoint;
        constexpr token TEXP = token::Exponent;
        constexpr token TAST = token::Asterisk;
        constexpr token TFWD = token::ForwardSlash;
        constexpr token TSYM = token::Symbol;
        constexpr token TSPA = token::Space;
        constexpr token TWSP = token::Whitespace;
        constexpr token TZZZ = token::EndOfInput;

        enum class state
        {
            Error,
            Ignore,
            EndOfParse,
            Element,
            Object,
            Array,
            Close,
            Value,
            NeedValueSeparator,
            NeedValue,
            NeedObjectValueSeparator,
            NeedObjectValue,
            Keyword,
            Name,
            EndName,
            String,
            StringEnd,
            NumberIntNeedDigit,
            NumberInt,
            NumberFracNeedDigit,
            NumberFrac,
            NumberExpSign,
            NumberExpIntNeedDigit,
            NumberExpInt,
            Escaping,
            Escaped,
            EscapingUnicode,
            CppStyleComment,
            CStyleComment,
            STATE_COUNT,
        };
        constexpr std::size_t STATE_COUNT = static_cast<std::size_t>(state::STATE_COUNT);
        constexpr state SXXX = state::Error;
        constexpr state SIGN = state::Ignore;
        constexpr state SZZZ = state::EndOfParse;
        constexpr state SELE = state::Element;
        constexpr state SOBJ = state::Object;
        constexpr state SARR = state::Array;
        constexpr state SCLO = state::Close;
        constexpr state SVAL = state::Value;
        constexpr state SNVS = state::NeedValueSeparator;
        constexpr state SNVA = state::NeedValue;
        constexpr state SOVS = state::NeedObjectValueSeparator;
        constexpr state SNOV = state::NeedObjectValue;
        constexpr state SKEY = state::Keyword;
        constexpr state SNAM = state::Name;
        constexpr state SENM = state::EndName;
        constexpr state SSTR = state::String;
        constexpr state SSEN = state::StringEnd;
        constexpr state SNU1 = state::NumberIntNeedDigit;
        constexpr state SNU2 = state::NumberInt;
        constexpr state SNU3 = state::NumberFracNeedDigit;
        constexpr state SNU4 = state::NumberFrac;
        constexpr state SNU5 = state::NumberExpSign;
        constexpr state SNU6 = state::NumberExpIntNeedDigit;
        constexpr state SNU7 = state::NumberExpInt;
        constexpr state SESC = state::Escaping;
        constexpr state SESD = state::Escaped;
        constexpr state SEUN = state::EscapingUnicode;
        constexpr state SCM1 = state::CppStyleComment;
        constexpr state SCM2 = state::CStyleComment;

        inline std::string to_string(state aState)
        {
            switch (aState)
            {
            case state::Error: 
                return std::string{ "Error" };
            case state::Ignore:
                return std::string{ "Ignore" };
            case state::EndOfParse:
                return std::string{ "EndOfParse" };
            case state::Element:
                return std::string{ "Element" };
            case state::Object:
                return std::string{ "Object" };
            case state::Array:
                return std::string{ "Array" };
            case state::Close:
                return std::string{ "Close" };
            case state::Value:
                return std::string{ "Value" };
            case state::NeedValueSeparator:
                return std::string{ "NeedValueSeparator" };
            case state::NeedValue:
                return std::string{ "NeedValue" };
            case state::NeedObjectValueSeparator:
                return std::string{ "NeedObjectValueSeparator" };
            case state::NeedObjectValue:
                return std::string{ "NeedObjectValue" };
            case state::Keyword:
                return std::string{ "Keyword" };
            case state::Name:
                return std::string{ "Name" };
            case state::EndName:
                return std::string{ "EndName" };
            case state::String:
                return std::string{ "String" };
            case state::StringEnd:
                return std::string{ "StringEnd" };
            case state::NumberIntNeedDigit:
                return std::string{ "NumberIntNeedDigit" };
            case state::NumberInt:
                return std::string{ "NumberInt" };
            case state::NumberFracNeedDigit:
                return std::string{ "NumberFracNeedDigit" };
            case state::NumberFrac:
                return std::string{ "NumberFrac" };
            case state::NumberExpSign:
                return std::string{ "NumberExpSign" };
            case state::NumberExpIntNeedDigit:
                return std::string{ "NumberExpIntNeedDigit" };
            case state::NumberExpInt:
                return std::string{ "NumberExpInt" };
            case state::Escaping:
                return std::string{ "Escaping" };
            case state::Escaped:
                return std::string{ "Escaped" };
            case state::EscapingUnicode:
                return std::string{ "EscapingUnicode" };
            case state::CppStyleComment:
            case state::CStyleComment:
                return std::string{ "Comment" };
            default:
                return std::string{ "??" };
            }
        }

        template <json_syntax Syntax>
        constexpr inline std::array<state, TOKEN_COUNT> object_state()
        {
            if constexpr (Syntax != json_syntax::Functional)
            {
                return
                    // state::Object
                    std::array<state, TOKEN_COUNT>
                    {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                        SXXX, SOBJ, SCLO, SXXX, SXXX, SXXX, SNOV, SNAM, SKEY, SXXX, SKEY, SKEY, SXXX, SXXX, SXXX, SKEY, SKEY, SXXX, SKEY, SXXX, SXXX, SXXX, SIGN, SIGN, SXXX
                    }};
            }
            else
            {
                return
                    // state::Object
                    std::array<state, TOKEN_COUNT>
                    {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                        SXXX, SOBJ, SCLO, SXXX, SXXX, SXXX, SNOV, SNAM, SKEY, SXXX, SKEY, SKEY, SXXX, SXXX, SXXX, SKEY, SKEY, SKEY, SKEY, SXXX, SXXX, SXXX, SIGN, SIGN, SXXX
                    }};
            }
        }

        template <json_syntax Syntax>
        constexpr inline std::array<state, TOKEN_COUNT> need_value_state()
        {
            if constexpr (Syntax != json_syntax::Functional)
            {
                return
                    // state::NeedValue
                    std::array<state, TOKEN_COUNT>
                    {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                        SXXX, SOBJ, SXXX, SARR, SXXX, SXXX, SXXX, SSTR, SKEY, SXXX, SKEY, SKEY, SXXX, SNU1, SNU2, SKEY, SKEY, SXXX, SKEY, SXXX, SXXX, SXXX, SIGN, SIGN, SXXX
                    }};
            }
            else
            {
                return
                    // state::NeedValue
                    std::array<state, TOKEN_COUNT>
                    {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                        SXXX, SOBJ, SXXX, SARR, SXXX, SXXX, SXXX, SSTR, SKEY, SXXX, SKEY, SKEY, SXXX, SNU1, SNU2, SKEY, SKEY, SKEY, SKEY, SXXX, SXXX, SXXX, SIGN, SIGN, SXXX
                    }};
            }
        }

        template <json_syntax Syntax>
        constexpr inline std::array<state, TOKEN_COUNT> need_object_value_state()
        {
            if constexpr (Syntax != json_syntax::Functional)
            {
                return
                    // state::NeedObjectValue
                    std::array<state, TOKEN_COUNT>
                    {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                        SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SNAM, SKEY, SXXX, SKEY, SKEY, SXXX, SXXX, SXXX, SKEY, SKEY, SXXX, SKEY, SXXX, SXXX, SXXX, SIGN, SIGN, SXXX
                    }};
            }
            else
            {
                return
                    // state::NeedObjectValue
                    std::array<state, TOKEN_COUNT>
                    {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                        SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SNAM, SKEY, SXXX, SKEY, SKEY, SXXX, SXXX, SXXX, SKEY, SKEY, SKEY, SKEY, SXXX, SXXX, SXXX, SIGN, SIGN, SXXX
                    }};
            }
        }

        template <json_syntax Syntax>
        constexpr inline std::array<state, TOKEN_COUNT> value_state()
        {
            if constexpr (Syntax == json_syntax::Standard || Syntax == json_syntax::StandardNoKeywords)
            {
                return
                    // state::Value
                    std::array<state, TOKEN_COUNT>
                    {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                        SXXX, SOBJ, SCLO, SARR, SCLO, SXXX, SXXX, SSTR, SKEY, SXXX, SKEY, SKEY, SXXX, SNU1, SNU2, SKEY, SKEY, SXXX, SKEY, SXXX, SXXX, SXXX, SIGN, SIGN, SZZZ
                    }};
            }
            else
            {
                return
                    // state::Value
                    std::array<state, TOKEN_COUNT>
                    {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                        SXXX, SOBJ, SCLO, SARR, SCLO, SXXX, SXXX, SSTR, SKEY, SXXX, SKEY, SKEY, SKEY, SNU1, SNU2, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SIGN, SIGN, SZZZ
                    }};
            }
        }

        template <json_syntax Syntax>
        constexpr inline std::array<state, TOKEN_COUNT> keyword_state()
        {
            if constexpr (Syntax == json_syntax::Standard || Syntax == json_syntax::StandardNoKeywords)
            {
                return
                    // state::Keyword
                    std::array<state, TOKEN_COUNT>
                    {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                        SXXX, SXXX, SCLO, SXXX, SCLO, SELE, SELE, SXXX, SKEY, SXXX, SKEY, SKEY, SXXX, SXXX, SKEY, SKEY, SKEY, SKEY, SKEY, SXXX, SXXX, SXXX, SELE, SELE, SXXX
                    }};
            }
            else if constexpr (Syntax == json_syntax::Relaxed)
            {
                return
                    // state::Keyword
                    std::array<state, TOKEN_COUNT>
                    {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                        SXXX, SXXX, SCLO, SXXX, SCLO, SELE, SELE, SXXX, SKEY, SXXX, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SELE, SELE, SXXX
                    }};
            }
            else
            {
                return
                    // state::Keyword
                    std::array<state, TOKEN_COUNT>
                    {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                        SXXX, SXXX, SCLO, SXXX, SCLO, SELE, SKEY, SXXX, SKEY, SXXX, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SKEY, SELE, SELE, SXXX
                    }};
            }
        }

        template <json_syntax Syntax>
        constexpr std::array<std::array<state, TOKEN_COUNT>, STATE_COUNT> sStateTables
        {
            // state::Error
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }},
            // state::Ignore
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }},
            // state::EndOfParse
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }},
            // state::Element
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SIGN, SIGN, SZZZ
            }},
            // state::Object
            object_state<Syntax>(),
            // state::Array
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SOBJ, SXXX, SARR, SCLO, SXXX, SXXX, SSTR, SKEY, SXXX, SKEY, SKEY, SXXX, SNU1, SNU2, SKEY, SKEY, SXXX, SKEY, SXXX, SXXX, SXXX, SIGN, SIGN, SXXX
            }},
            // state::Close
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }},
            // state::Value
            value_state<Syntax>(),
            // state::NeedValueSeparator
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SCLO, SXXX, SCLO, SXXX, SVAL, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SIGN, SIGN, SXXX
            }},
            // state::NeedValue
            need_value_state<Syntax>(),
            // state::NeedObjectValueSeparator
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SCLO, SXXX, SCLO, SXXX, SNOV, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SIGN, SIGN, SXXX
            }},
            // state::NeedObjectValue
            need_object_value_state<Syntax>(),
            // state::Keyword
            keyword_state<Syntax>(),
            // state::Name
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SNAM, SNAM, SNAM, SNAM, SNAM, SNAM, SENM, SNAM, SESC, SNAM, SNAM, SNAM, SNAM, SNAM, SNAM, SNAM, SNAM, SNAM, SNAM, SNAM, SNAM, SNAM, SXXX, SXXX
            }},
            // state::EndName
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SELE, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SIGN, SIGN, SXXX
            }},
            // state::String
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SSTR, SSTR, SSTR, SSTR, SSTR, SSTR, SSEN, SSTR, SESC, SSTR, SSTR, SSTR, SSTR, SSTR, SSTR, SSTR, SSTR, SSTR, SSTR, SSTR, SSTR, SSTR, SXXX, SXXX
            }},
            // state::StringEnd
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SCLO, SXXX, SCLO, SXXX, SELE, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SELE, SELE, SXXX
            }},
            // state::NumberIntNeedDigit
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SNU2, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }},
            // state::NumberInt
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SCLO, SXXX, SCLO, SXXX, SELE, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SNU2, SXXX, SXXX, SNU3, SNU5, SXXX, SXXX, SXXX, SELE, SELE, SXXX
            }},
            // state::NumberFracNeedDigit
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SNU4, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }},
            // state::NumberFrac
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SCLO, SXXX, SCLO, SXXX, SELE, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SNU4, SXXX, SXXX, SXXX, SNU5, SXXX, SXXX, SXXX, SELE, SELE, SXXX
            }},
            // state::NumberExpSign
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SNU6, SNU6, SNU7, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }},
            // state::NumberExpIntNeedDigit
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SNU7, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }},
            // state::NumberExpInt
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SCLO, SXXX, SCLO, SXXX, SELE, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SNU7, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SELE, SELE, SXXX
            }},
            // state::Escaping
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SESD, SXXX, SESD, SEUN, SESD, SXXX, SXXX, SXXX, SXXX, SESD, SXXX, SXXX, SXXX, SESD, SXXX, SXXX, SXXX, SXXX
            }},
            // state::Escaped
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }},
            // state::EscapingUnicode
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SESD, SESD, SESD, SXXX, SESD, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }},
            // state::CppStyleComment
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }},
            // state::CStyleComment
            std::array<state, TOKEN_COUNT>
            {{//TXXX  TOBJ  TCLO  TARR  TCLA  TCOL  TCOM  TQOT  TCHA  TESC  TESU  TECH  TPLU  TMIN  TDIG  THEX  TEHX  TDEC  TEXP  TAST  TFWD  TSYM  TSPA  TWSP  TZZZ
                SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX, SXXX
            }}
        };

        template <json_syntax Syntax>
        constexpr std::array<token, 256> sTokenTable
        { 
            {// 0x0   0x1   0x2   0x3   0x4   0x5   0x6   0x7   0x8   0x9   0xA   0xB   0xC   0xD   0xE   0xF
                TZZZ, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TWSP, TWSP, TXXX, TXXX, TWSP, TXXX, TXXX, // 0x0
                TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, // 0x1
                TSPA, TSYM, TQOT, TSYM, TCHA, TSYM, TSYM, TSYM, TSYM, TSYM, TAST, TPLU, TCOM, TMIN, TDEC, TFWD, // 0x2
                TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TCOL, TSYM, TSYM, TSYM, TSYM, TSYM, // 0x3
                TCHA, THEX, THEX, THEX, THEX, TEXP, THEX, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0x4
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TARR, TESC, TCLA, TSYM, TCHA, // 0x5
                TSYM, THEX, TEHX, THEX, THEX, TEXP, TEHX, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TECH, TCHA, // 0x6
                TCHA, TCHA, TECH, TCHA, TECH, TESU, TCHA, TCHA, TCHA, TCHA, TCHA, TOBJ, TSYM, TCLO, TSYM, TSYM, // 0x7
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0x8
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0x9
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xA
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xB
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xC
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xD
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xE
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xF
            },
        };

        template <>
        constexpr std::array<token, 256> sTokenTable<json_syntax::Relaxed>
        {
            {// 0x0   0x1   0x2   0x3   0x4   0x5   0x6   0x7   0x8   0x9   0xA   0xB   0xC   0xD   0xE   0xF
                TZZZ, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TWSP, TWSP, TXXX, TXXX, TWSP, TXXX, TXXX, // 0x0
                TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, // 0x1
                TSPA, TSYM, TQOT, TSYM, TCHA, TSYM, TSYM, TQOT, TSYM, TSYM, TAST, TPLU, TCOM, TMIN, TDEC, TFWD, // 0x2
                TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TCOL, TSYM, TSYM, TSYM, TSYM, TSYM, // 0x3
                TCHA, THEX, THEX, THEX, THEX, TEXP, THEX, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0x4
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TARR, TESC, TCLA, TSYM, TCHA, // 0x5
                TQOT, THEX, TEHX, THEX, THEX, TEXP, TEHX, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TECH, TCHA, // 0x6
                TCHA, TCHA, TECH, TCHA, TECH, TESU, TCHA, TCHA, TCHA, TCHA, TCHA, TOBJ, TSYM, TCLO, TSYM, TSYM, // 0x7
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0x8
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0x9
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xA
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xB
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xC
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xD
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xE
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xF
            },
        };

        template <>
        constexpr std::array<token, 256> sTokenTable<json_syntax::Functional>
        {
            {// 0x0   0x1   0x2   0x3   0x4   0x5   0x6   0x7   0x8   0x9   0xA   0xB   0xC   0xD   0xE   0xF
                TZZZ, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TWSP, TWSP, TXXX, TXXX, TWSP, TXXX, TXXX, // 0x0
                TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, TXXX, // 0x1
                TSPA, TSYM, TQOT, TSYM, TCHA, TSYM, TSYM, TQOT, TSYM, TSYM, TAST, TPLU, TSYM, TMIN, TDEC, TFWD, // 0x2
                TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TDIG, TCOL, TSYM, TSYM, TSYM, TSYM, TSYM, // 0x3
                TCHA, THEX, THEX, THEX, THEX, TEXP, THEX, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0x4
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TARR, TESC, TCLA, TSYM, TCHA, // 0x5
                TQOT, THEX, TEHX, THEX, THEX, TEXP, TEHX, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TECH, TCHA, // 0x6
                TCHA, TCHA, TECH, TCHA, TECH, TESU, TCHA, TCHA, TCHA, TCHA, TCHA, TOBJ, TSYM, TCLO, TSYM, TSYM, // 0x7
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0x8
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0x9
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xA
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xB
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xC
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xD
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xE
                TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, TCHA, // 0xF
            },
        };

        template <typename CharT>
        inline token to_token(const std::array<token, 256>& aTokenTable, CharT aCharacter)
        {
            auto const tableIndex = std::min<std::size_t>(static_cast<typename std::make_unsigned<CharT>::type>(aCharacter), 0xFFu);
            return aTokenTable[tableIndex];
        }

        template <json_syntax Syntax, typename CharT>
        inline state next_state(state aCurrentState, CharT aCharacter)
        {
            auto const token = to_token(sTokenTable<Syntax>, aCharacter);
            auto const stateIndex = static_cast<std::size_t>(aCurrentState);
            return sStateTables<Syntax>[stateIndex][static_cast<std::size_t>(token)];
        }

        template <json_syntax Syntax, typename CharT>
        inline state next_state(state aCurrentState, state aPreviousState, CharT aCurrentCharacter, CharT aNextCharacter)
        {
            if constexpr (Syntax == json_syntax::Standard || Syntax == json_syntax::StandardNoKeywords)
                return next_state<Syntax>(aCurrentState, aCurrentCharacter);
            else
            {
                auto const token = to_token(sTokenTable<Syntax>, aCurrentCharacter);
                switch (aCurrentState)
                {
                case state::CppStyleComment:
                    if (aCurrentCharacter != '\n' && token != token::EndOfInput)
                        return state::CppStyleComment;
                    return next_state<Syntax>(aPreviousState, aPreviousState, aCurrentCharacter, aNextCharacter);
                case state::CStyleComment:
                    if (token == json_detail::token::Asterisk)
                    {
                        auto const nextToken = to_token(sTokenTable<Syntax>, aNextCharacter);
                        if (nextToken == json_detail::token::ForwardSlash)
                            return state::Ignore;
                        else if (nextToken == json_detail::token::EndOfInput)
                            return state::Error;
                    }
                    else if (token == json_detail::token::EndOfInput)
                        return state::Error;
                    return state::CStyleComment;
                default:
                    if (token == json_detail::token::ForwardSlash)
                    {
                        auto const nextToken = to_token(sTokenTable<Syntax>, aNextCharacter);
                        if (nextToken == json_detail::token::ForwardSlash)
                            return state::CppStyleComment;
                        else if (nextToken == json_detail::token::Asterisk)
                            return state::CStyleComment;
                    }
                    break;
                }
                auto const stateIndex = static_cast<std::size_t>(aCurrentState);
                return sStateTables<Syntax>[stateIndex][static_cast<std::size_t>(token)];
            }
        }

        struct hash_first_character
        {
            template <typename String>
            std::size_t operator()(const String& aString) const noexcept
            {
                return std::hash<typename String::value_type>{}(aString[0]);
            }
        };
    }

    namespace json_detail
    {
        template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
        struct iterator_traits
        {
            typedef std::bidirectional_iterator_tag iterator_category;
            typedef neolib::basic_json_value<Syntax, Alloc, CharT, Traits, CharAlloc> value_type;
            typedef std::ptrdiff_t difference_type;
            typedef value_type* pointer;
            typedef value_type& reference;

            typedef neolib::basic_json<Syntax, Alloc, CharT, Traits, CharAlloc> document_type;
        };

        template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
        struct const_iterator_traits
        {
            typedef std::bidirectional_iterator_tag iterator_category;
            typedef neolib::basic_json_value<Syntax, Alloc, CharT, Traits, CharAlloc> value_type;
            typedef std::ptrdiff_t difference_type;
            typedef const value_type* pointer;
            typedef const value_type& reference;

            typedef neolib::basic_json<Syntax, Alloc, CharT, Traits, CharAlloc> document_type;
        };
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    template <typename IteratorTraits>
    class basic_json_value<Syntax, Alloc, CharT, Traits, CharAlloc>::iterator_base : public IteratorTraits
    {
    private:
        typedef IteratorTraits traits;
    protected:
        using typename traits::iterator_category;
        using typename traits::value_type;
        using typename traits::difference_type;
        using typename traits::pointer;
        using typename traits::reference;
    protected:
        typedef typename const_selector_from_pointer<const value_type*, value_type*, pointer>::type value_pointer;
        typedef typename const_selector_from_pointer<const value_type&, value_type&, pointer>::type value_reference;
    protected:
        iterator_base() : iValue{ nullptr }
        {
        }
        iterator_base(const iterator_base& aOther) : iValue{ aOther.iValue }
        {
        }
    protected:
        iterator_base(value_pointer aValue) : iValue{ aValue }
        {
        }
    protected:
        pointer operator->() const
        {
            return &*iValue;
        }
        reference operator*() const
        {
            return *iValue;
        }
    protected:
        void operator++()
        {
            iValue = value().next_sibling();
        }
        void operator--()
        {
            // todo
            iValue = nullptr;
        }
    protected:
        bool operator==(const iterator_base& aOther) const
        {
            return iValue == aOther.iValue;
        }
        bool operator!=(const iterator_base& aOther) const
        {
            return iValue != aOther.iValue;
        }
    protected:
        value_reference value() const
        {
            return *iValue;
        }
        bool has_parent() const
        {
            return iValue != nullptr && value().has_parent();
        }
        value_pointer parent() const
        {
            if (has_parent())
                return &value().parent();
            return nullptr;
        }
    private:
        value_pointer iValue;
    };

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    class basic_json_value<Syntax, Alloc, CharT, Traits, CharAlloc>::iterator : iterator_base<typename json_detail::iterator_traits<Syntax, Alloc, CharT, Traits, CharAlloc>>
    {
        typedef iterator_base<typename json_detail::iterator_traits<Syntax, Alloc, CharT, Traits, CharAlloc>> base_type;
        friend class basic_json_value<Syntax, Alloc, CharT, Traits, CharAlloc>;
    public:
        using typename base_type::iterator_category;
        using typename base_type::value_type;
        using typename base_type::difference_type;
        using typename base_type::pointer;
        using typename base_type::reference;
    public:
        using typename base_type::value_pointer;
    public:
        iterator()
        {
        }
        iterator(const iterator& aOther) : base_type{ aOther }
        {
        }
    private:
        iterator(value_pointer aValue) : base_type{ aValue }
        {
        }
    public:
        using base_type::operator*;
        using base_type::operator->;
    public:
        iterator & operator++()
        {
            base_type::operator++();
            return *this;
        }
        iterator operator++(int)
        {
            auto previous = *this;
            base_type::operator++();
            return previous;
        }
        iterator& operator--()
        {
            base_type::operator--();
            return *this;
        }
        iterator operator--(int)
        {
            auto previous = *this;
            base_type::operator--();
            return previous;
        }
    public:
        bool operator==(const iterator& aRhs) const
        {
            return base_type::operator==(aRhs);
        }
        bool operator!=(const iterator& aRhs) const
        {
            return base_type::operator!=(aRhs);
        }
    public:
        using base_type::value;
    };

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    class basic_json_value<Syntax, Alloc, CharT, Traits, CharAlloc>::const_iterator : iterator_base<typename json_detail::const_iterator_traits<Syntax, Alloc, CharT, Traits, CharAlloc>>
    {
        typedef iterator_base<typename json_detail::const_iterator_traits<Syntax, Alloc, CharT, Traits, CharAlloc>> base_type;
        friend class basic_json_value<Syntax, Alloc, CharT, Traits, CharAlloc>;
    public:
        using typename base_type::iterator_category;
        using typename base_type::value_type;
        using typename base_type::difference_type;
        using typename base_type::pointer;
        using typename base_type::reference;
    public:
        using typename base_type::value_pointer;
    public:
        const_iterator()
        {
        }
        const_iterator(const const_iterator& aOther) : base_type{ aOther }
        {
        }
        const_iterator(const iterator& aOther) : base_type{ aOther.iValue }
        {
        }
    private:
        const_iterator(value_pointer aValue) : base_type{ aValue }
        {
        }
    public:
        using base_type::operator*;
        using base_type::operator->;
    public:
        const_iterator & operator++()
        {
            base_type::operator++();
            return *this;
        }
        const_iterator operator++(int)
        {
            auto previous = *this;
            base_type::operator++();
            return previous;
        }
        const_iterator& operator--()
        {
            base_type::operator--();
            return *this;
        }
        const_iterator operator--(int)
        {
            auto previous = *this;
            base_type::operator--();
            return previous;
        }
    public:
        bool operator==(const const_iterator& aRhs) const
        {
            return base_type::operator==(aRhs);
        }
        bool operator!=(const const_iterator& aRhs) const
        {
            return base_type::operator!=(aRhs);
        }
    public:
        using base_type::value;
    };

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    template <typename IteratorTraits>
    class basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::iterator_base : public IteratorTraits
    {
    private:
        typedef IteratorTraits traits;
    protected:
        using typename traits::iterator_category;
        using typename traits::value_type;
        using typename traits::difference_type;
        using typename traits::pointer;
        using typename traits::reference;
    protected:
        typedef typename const_selector_from_pointer<const value_type*, value_type*, pointer>::type value_pointer;
        typedef typename const_selector_from_pointer<const value_type&, value_type&, pointer>::type value_reference;
    protected:
        iterator_base() : iValue{ nullptr }
        {
        }
        iterator_base(const iterator_base& aOther) : iValue{ aOther.iValue }
        {
        }
    protected:
        iterator_base(value_pointer aValue) : iValue{ aValue }
        {
        }
    protected:
        pointer operator->() const
        {
            return &*iValue;
        }
        reference operator*() const
        {
            return *iValue;
        }
    protected:
        void operator++()
        {
            if (value().has_children())
                iValue = value().first_child();
            else if (!value().is_last_sibling())
                iValue = value().next_sibling();
            else
                iValue = value().next_parent_sibling();
        }
        void operator--()
        {
            // todo
            iValue = nullptr;
        }
    protected:
        bool operator==(const iterator_base& aOther) const
        {
            return iValue == aOther.iValue;
        }
        bool operator!=(const iterator_base& aOther) const
        {
            return iValue != aOther.iValue;
        }
    protected:
        value_reference value() const
        {
            return *iValue;
        }
        bool has_parent() const
        {
            return iValue != nullptr && value().has_parent();
        }
        value_pointer parent() const
        {
            if (has_parent())
                return &value().parent();
            return nullptr;
        }
    private:
        value_pointer iValue;
    };

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    class basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::iterator : public iterator_base<typename json_detail::iterator_traits<Syntax, Alloc, CharT, Traits, CharAlloc>>
    {
        typedef iterator_base<typename json_detail::iterator_traits<Syntax, Alloc, CharT, Traits, CharAlloc>> base_type;
        friend class basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>;
    public:
        using typename base_type::iterator_category;
        using typename base_type::value_type;
        using typename base_type::difference_type;
        using typename base_type::pointer;
        using typename base_type::reference;
    public:
        using typename base_type::value_pointer;
    public:
        iterator()
        {
        }
        iterator(const iterator& aOther) : base_type{ aOther }
        {
        }
    private:
        iterator(value_pointer aValue) : base_type{ aValue }
        {
        }
    public:
        using base_type::operator*;
        using base_type::operator->;
    public:
        iterator& operator++()
        {
            base_type::operator++();
            return *this;
        }
        iterator operator++(int)
        {
            auto previous = *this;
            base_type::operator++();
            return previous;
        }
        iterator& operator--()
        {
            base_type::operator--();
            return *this;
        }
        iterator operator--(int)
        {
            auto previous = *this;
            base_type::operator--();
            return previous;
        }
    public:
        bool operator==(const iterator& aRhs) const
        {
            return base_type::operator==(aRhs);
        }
        bool operator!=(const iterator& aRhs) const
        {
            return base_type::operator!=(aRhs);
        }
    public:
        using base_type::value;
    };

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    class basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::const_iterator : public iterator_base<typename json_detail::const_iterator_traits<Syntax, Alloc, CharT, Traits, CharAlloc>>
    {
        typedef iterator_base<typename json_detail::const_iterator_traits<Syntax, Alloc, CharT, Traits, CharAlloc>> base_type;
        friend class basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>;
    public:
        using typename base_type::iterator_category;
        using typename base_type::value_type;
        using typename base_type::difference_type;
        using typename base_type::pointer;
        using typename base_type::reference;
    public:
        using typename base_type::value_pointer;
    public:
        const_iterator()
        {
        }
        const_iterator(const const_iterator& aOther) : base_type{ aOther }
        {
        }
        const_iterator(const iterator& aOther) : base_type{ aOther.iValue }
        {
        }
    private:
        const_iterator(value_pointer aValue) : base_type{ aValue }
        {
        }
    public:
        using base_type::operator*;
        using base_type::operator->;
    public:
        const_iterator& operator++()
        {
            base_type::operator++();
            return *this;
        }
        const_iterator operator++(int)
        {
            auto previous = *this;
            base_type::operator++();
            return previous;
        }
        const_iterator& operator--()
        {
            base_type::operator--();
            return *this;
        }
        const_iterator operator--(int)
        {
            auto previous = *this;
            base_type::operator--();
            return previous;
        }
    public:
        bool operator==(const const_iterator& aRhs) const
        {
            return base_type::operator==(aRhs);
        }
        bool operator!=(const const_iterator& aRhs) const
        {
            return base_type::operator!=(aRhs);
        }
    public:
        using base_type::value;
    };

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::basic_json() : iEncoding{ json_detail::default_encoding<CharT>::DEFAULT_ENCODING }, iCursor{}
    {
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::basic_json(const std::string& aPath, bool aValidateUtf) : iEncoding{ json_detail::default_encoding<CharT>::DEFAULT_ENCODING }, iCursor{}
    {
        if (!read(aPath, aValidateUtf))
            throw json_error(error_text());
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    template <typename Elem, typename ElemTraits>
    inline basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::basic_json(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf) : iEncoding{ json_detail::default_encoding<CharT>::DEFAULT_ENCODING }, iCursor{}
    {
        if (!read(aInput, aValidateUtf))
            throw json_error(error_text());
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline void basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::clear()
    {
        iCursor = decltype(iCursor){};
        document().clear();
        iUtf16HighSurrogate = std::nullopt;
    }

    template <json_syntax Syntax>
    inline constexpr const char* document_type()
    {
        if constexpr (Syntax == json_syntax::Standard || Syntax == json_syntax::StandardNoKeywords)
            return "JSON";
        else if constexpr (Syntax == json_syntax::Relaxed)
            return "RJSON";
        else if constexpr (Syntax == json_syntax::Functional)
            return "FJSON";
    }
        
    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline bool basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::read(const std::string& aPath, bool aValidateUtf)
    {
        std::ifstream input{ aPath, std::ios::binary };
        if (!input)
        {
            iErrorText = "failed to open " + std::string{ document_type<Syntax>() } + " file '" + aPath + "'";
            return false;
        }
        bool ok = do_read(input, aValidateUtf);
        if (ok)
            ok = do_parse();
        if (!ok)
            iErrorText = "failed to parse " + std::string{ document_type<Syntax>() } + " file '" + aPath + "', " + iErrorText;
        return ok;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    template <typename Elem, typename ElemTraits>
    inline bool basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::read(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf)
    {
        if (!aInput)
        {
            iErrorText = "failed to read " + std::string{ document_type<Syntax>() } + " text";
            return false;
        }
        bool ok = do_read(aInput, aValidateUtf);
        if (ok)
            ok = do_parse();
        if (!ok)
            iErrorText = "failed to parse " + std::string{ document_type<Syntax>() } + " text, " + iErrorText;
        return ok;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    template <typename Elem, typename ElemTraits>
    inline bool basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::do_read(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf)
    {
        clear();

        if (!aInput)
        {
            iErrorText = "input stream bad";
            return false;
        }

        typedef typename std::basic_istream<CharT, Traits>::pos_type pos_type;
        pos_type count = 0;
        aInput.seekg(0, std::ios::end);
        if (aInput)
        {
            count = aInput.tellg();
            if (count == static_cast<pos_type>(-1))
                count = 0;
            aInput.seekg(0, std::ios::beg);
        }
        else
            aInput.clear();

        if (count != typename std::basic_istream<CharT>::pos_type(0))
        {
            document().reserve(static_cast<typename json_string::size_type>(count) + 1);
            document().resize(static_cast<typename json_string::size_type>(count));
            aInput.read(&document()[0], count);
            document().resize(static_cast<typename json_string::size_type>(aInput.gcount()));
        }
        else
        {
            CharT buffer[1024];
            while (aInput.read(buffer, sizeof(buffer)))
                document().append(buffer, static_cast<typename json_string::size_type>(aInput.gcount()));
            if (aInput.eof())
                document().append(buffer, static_cast<typename json_string::size_type>(aInput.gcount()));
        }

        if (document().empty())
        {
            iErrorText = "empty document";
            return false;
        }

        if (json_detail::next_state<syntax>(json_detail::state::Value, document().back()) != json_detail::state::Ignore)
            document().push_back(character_type{ '\n' });
        document().push_back(character_type{ '\0' });

        if (aValidateUtf && !neolib::check_utf8(document().to_std_string_view()))
        {
            iErrorText = "invalid utf-8";
            return false;
        }

        return true;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline bool basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::do_parse()
    {
        static constexpr bool usePreviousState = (Syntax == json_syntax::Relaxed || Syntax == json_syntax::Functional || json_detail::debug);

        json_detail::state currentState = json_detail::state::Value;
        json_detail::state previousState = currentState;
        json_detail::state nextState;
        element currentElement = {};

        iCursor.line = 1;
        iCursor.column = 1;
            
        auto nextInputCh = &*document().begin();
        auto nextOutputCh = nextInputCh;

        auto increment_cursor = [&nextInputCh, this]()
        {
            if (*nextInputCh++ != '\n')
                ++iCursor.column;
            else
            {
                iCursor.column = 1;
                ++iCursor.line;
            }
        };

        // Main parse loop
        for (;;)
        {
            if constexpr (json_detail::debug)
            {
                if (*nextInputCh != '\n')
                    std::cout << *nextInputCh;
                else
                    std::cout << "\\n";
            }
            try
            {
                if constexpr (!usePreviousState)
                    nextState = json_detail::next_state<syntax>(currentState, *nextInputCh);
                else
                {
                    bool skippingComment = false;
                    auto tempState = currentState;
                    do {
                        nextState = json_detail::next_state<syntax>(tempState, previousState, *nextInputCh, nextInputCh != &iDocumentText.back() ? *(nextInputCh + 1) : '\0');
                        skippingComment = (nextState == json_detail::state::CppStyleComment || nextState == json_detail::state::CStyleComment);
                        if (skippingComment)
                        {
                            if (tempState != nextState && currentElement.start != nullptr && nextOutputCh == currentElement.start)
                                nextOutputCh = (currentState != json_detail::state::StringEnd ? nextInputCh : nextInputCh - 1);
                            tempState = nextState;
                            increment_cursor();
                        }
                        else if (tempState == json_detail::state::CStyleComment && nextState == json_detail::state::Ignore)
                            increment_cursor();
                    } while (skippingComment);
                }
                if constexpr (Syntax == json_syntax::Functional)
                {
                    if (nextState == json_detail::state::Error)
                    {
                        switch (currentState)
                        {
                        case json_detail::state::NumberInt:
                        case json_detail::state::NumberFrac:
                        case json_detail::state::NumberExpInt:
                            switch (to_token(json_detail::sTokenTable<json_syntax::Functional>, *nextInputCh))
                            {
                            case json_detail::token::Character:
                            case json_detail::token::HexDigit:
                            case json_detail::token::EscapedOrHexDigit:
                                currentState = json_detail::state::Keyword;
                                nextState = json_detail::state::Keyword;
                                currentElement.type = element::Keyword;
                                nextOutputCh = nextInputCh;
                                break;
                            default:
                                break;
                            }
                        default:
                            break;
                        }
                    }
                }
                switch (nextState)
                {
                case json_detail::state::Ignore:
                    increment_cursor();
                    continue;
                case json_detail::state::Error:
                    create_parse_error();
                    return false;
                case json_detail::state::EndOfParse:
                    if (nextInputCh != &iDocumentText.back())
                    {
                        create_parse_error();
                        return false;
                    }
                    return true;
                default:
                    if (currentState == nextState)
                    {
                        switch (currentState)
                        {
                        case json_detail::state::String:
                        case json_detail::state::Keyword:
                        case json_detail::state::Name:
                            if (currentElement.start != nextOutputCh)
                                *nextOutputCh++ = *nextInputCh;
                            // fall through
                        default:
                            increment_cursor();
                            continue;
                        case json_detail::state::Object:
                        case json_detail::state::Array:
                            break;
                        }
                    }
                }
                if constexpr (json_detail::debug)
                    std::cout << "(" << to_string(currentState) << " -> " << to_string(nextState) << ")";
                switch (nextState)
                {
                case json_detail::state::Close:
                case json_detail::state::Element:
                    switch (currentElement.type)
                    {
                    case element::Unknown:
                        break;
                    case element::String:
                        {
                            json_string newString{ currentElement.start, currentElement.start == nextOutputCh ? nextInputCh - 1 : nextOutputCh };
                            buy_value(currentElement, newString);
                        }
                        break;
                    case element::Name:
                        if (context() == json_type::Object && std::holds_alternative<std::monostate>(currentElement.name))
                        {
                            json_string newString{ currentElement.start, currentElement.start == nextOutputCh ? nextInputCh - 1 : nextOutputCh };
                            currentElement.name = newString;
                        }
                        break;
                    case element::Number:
                        {
                            json_string newNumber{ currentElement.start, currentElement.start == nextOutputCh ? nextInputCh : nextOutputCh };
                            if (currentState == json_detail::state::NumberInt)
                            {
                                std::visit([this, &currentElement](auto&& arg)
                                {
                                    buy_value(currentElement, std::forward<decltype(arg)>(arg));
                                }, string_to_number(newNumber.to_std_string_view()));
                            }
                            else
                                buy_value(currentElement, neolib::string_to_double(newNumber.to_std_string_view()));
                        }
                        break;
                    case element::Keyword:
                        {
                            static const std::unordered_map<std::string_view, json_detail::keyword, json_detail::hash_first_character> sJsonKeywords =
                            {
                                { "true", json_detail::keyword::True },
                                { "false", json_detail::keyword::False },
                                { "null",  json_detail::keyword::Null },
                            };
                            auto keywordText = json_string{ currentElement.start, currentElement.start == nextOutputCh ? nextInputCh : nextOutputCh };
                            auto keyword = sJsonKeywords.find(keywordText);
                            if (keyword != sJsonKeywords.end())
                            {
                                if (context() == json_type::Object && std::holds_alternative<std::monostate>(currentElement.name))
                                {
                                    create_parse_error("bad object field name");
                                    return false;
                                }
                                switch (keyword->second)
                                {
                                case json_detail::keyword::True:
                                    buy_value(currentElement, json_bool{ true });
                                    break;
                                case json_detail::keyword::False:
                                    buy_value(currentElement, json_bool{ false });
                                    break;
                                case json_detail::keyword::Null:
                                    buy_value(currentElement, json_null{});
                                    break;
                                }
                            }
                            else
                            {
                                if constexpr (syntax == json_syntax::StandardNoKeywords)
                                {
                                    create_parse_error("keywords unavailable");
                                    return false;
                                }
                                if (context() == json_type::Object && std::holds_alternative<std::monostate>(currentElement.name))
                                {
                                    currentElement.name = json_keyword{ keywordText };
                                    currentElement.type = element::Name;
                                }
                                else
                                    buy_value(currentElement, json_keyword{ keywordText });
                            }
                        }
                        break;
                    default:
                        break;
                    }
                    if (nextState == json_detail::state::Close)
                        iCompositeValueStack.pop_back();
                    switch (context())
                    {
                    case json_type::Object:
                        if constexpr (syntax == json_syntax::Standard)
                        {
                            if (std::holds_alternative<std::monostate>(currentElement.name))
                            {
                                if (nextState == json_detail::state::Close)
                                    nextState = json_detail::state::NeedObjectValueSeparator;
                                else if (*nextInputCh == ',')
                                    nextState = json_detail::state::NeedObjectValue;
                                else
                                    nextState = json_detail::state::NeedObjectValueSeparator;
                            }
                            else
                                nextState = json_detail::state::NeedValue;
                        }
                        else
                        {
                            if (std::holds_alternative<std::monostate>(currentElement.name))
                                nextState = json_detail::state::Object;
                            else
                                nextState = *nextInputCh != ':' ? json_detail::state::EndName : json_detail::state::NeedValue;
                        }
                        break;
                    case json_type::Array:
                        if constexpr (syntax == json_syntax::Standard)
                        {
                            if (*nextInputCh == ',')
                                nextState = json_detail::state::NeedValue;
                            else
                                nextState = json_detail::state::NeedValueSeparator;
                        }
                        else
                            nextState = json_detail::state::Value;
                        break;
                    default:
                        if (nextState == json_detail::state::Close)
                            nextState = json_detail::state::Value;
                        break;
                    }
                    currentElement.type = element::Unknown;
                    currentElement.start = nullptr;
                    break;
                case json_detail::state::String:
                    currentElement.type = element::String;
                    currentElement.start = (nextOutputCh = nextInputCh + 1);
                    break;
                case json_detail::state::Name:
                    currentElement.type = element::Name;
                    currentElement.start = (nextOutputCh = nextInputCh + 1);
                    break;
                case json_detail::state::EndName:
                    if (std::holds_alternative<std::monostate>(currentElement.name))
                    {
                        json_string newName
                        {
                            currentElement.start,
                            currentElement.start == nextOutputCh ? nextInputCh : nextOutputCh
                        };
                        currentElement.name = newName;
                    }
                    break;
                case json_detail::state::NumberIntNeedDigit:
                    currentElement.type = element::Number;
                    currentElement.start = nextInputCh;
                    break;
                case json_detail::state::NumberInt:
                    if (currentElement.type != element::Number)
                    {
                        currentElement.type = element::Number;
                        currentElement.start = nextInputCh;
                    }
                    break;
                case json_detail::state::Array:
                    {
                        json_value* newArray = buy_value(currentElement, json_array{});
                        iCompositeValueStack.push_back(newArray);
                        nextState = json_detail::state::Value;
                    }
                    break;
                case json_detail::state::Object:
                    {
                        json_value* newObject = buy_value(currentElement, json_object{});
                        iCompositeValueStack.push_back(newObject);
                    }
                    break;
                case json_detail::state::Keyword:
                    currentElement.type = element::Keyword;
                    currentElement.start = (nextOutputCh = nextInputCh);
                    break;
                case json_detail::state::StringEnd:
                    if constexpr (syntax == json_syntax::Relaxed)
                    {
                        // relaxed: support for three different quote characters
                        if (*nextInputCh != *(currentElement.start - 1))
                            nextState = json_detail::state::String;
                    }
                    break;
                case json_detail::state::Escaped:
                    if (nextOutputCh == currentElement.start)
                        nextOutputCh = (currentState != json_detail::state::EscapingUnicode ? nextInputCh - 1 : nextInputCh - 2);
                    if (currentState == json_detail::state::Escaping)
                    {
                        switch (*(nextInputCh))
                        {
                        case '\"':
                            (*nextOutputCh++) = '\"';
                            break;
                        case '\\':
                            (*nextOutputCh++) = '\\';
                            break;
                        case '/':
                            (*nextOutputCh++) = '/';
                            break;
                        case 'b':
                            (*nextOutputCh++) = '\b';
                            break;
                        case 'f':
                            (*nextOutputCh++) = '\f';
                            break;
                        case 'n':
                            (*nextOutputCh++) = '\n';
                            break;
                        case 'r':
                            (*nextOutputCh++) = '\r';
                            break;
                        case 't':
                            (*nextOutputCh++) = '\t';
                            break;
                        default:
                            break;
                        }
                        nextState = currentElement.type == element::String ? json_detail::state::String : json_detail::state::Name;
                    }
                    else if (currentState == json_detail::state::EscapingUnicode)
                    {
                        // todo throw an error if there are invalid surrogate pairs
                        if (currentElement.auxType != element::EscapedUnicode)
                        {
                            currentElement.auxType = element::EscapedUnicode;
                            currentElement.auxStart = nextInputCh;
                        }
                        if (nextInputCh + 1 - currentElement.auxStart == 4)
                        {
                            string_type s{ currentElement.auxStart, nextInputCh + 1 };
                            char16_t u16ch = static_cast<char16_t>(std::stoul(s, nullptr, 16));
                            if (utf16::is_high_surrogate(u16ch))
                            {
                                iUtf16HighSurrogate = u16ch;
                                currentElement.auxType = element::Unknown;
                                currentElement.type = element::String;
                                nextState = json_detail::state::String;
                                break;
                            }
                            else if (utf16::is_low_surrogate(u16ch) && iUtf16HighSurrogate != std::nullopt)
                            {
                                switch (encoding())
                                {
                                case json_encoding::Utf8:
                                    {
                                        char16_t surrogatePair[] = { *iUtf16HighSurrogate, u16ch };
                                        auto utf8 = utf16_to_utf8(std::u16string(&surrogatePair[0], 2));
                                        nextOutputCh = std::copy(utf8.begin(), utf8.end(), nextOutputCh);
                                    }
                                    break;
                                case json_encoding::Utf16LE:
                                case json_encoding::Utf16BE:
                                    (*nextOutputCh++) = static_cast<character_type>(*iUtf16HighSurrogate);
                                    (*nextOutputCh++) = static_cast<character_type>(u16ch);
                                    break;
                                case json_encoding::Utf32LE:
                                case json_encoding::Utf32BE:
                                    {
                                        char16_t surrogatePair[] = { *iUtf16HighSurrogate, u16ch };
                                        (*nextOutputCh++) =  static_cast<character_type>(utf8_to_utf32(utf16_to_utf8(std::u16string{ &surrogatePair[0], 2 }))[0]);
                                    }
                                    break;
                                default:
                                    break;
                                }
                                iUtf16HighSurrogate = std::nullopt;
                            }
                            else
                            {
                                switch (encoding())
                                {
                                case json_encoding::Utf8:
                                    {
                                        auto utf8 = utf16_to_utf8(std::u16string(1, u16ch));
                                        nextOutputCh = std::copy(utf8.begin(), utf8.end(), nextOutputCh);
                                    }
                                    break;
                                case json_encoding::Utf16LE:
                                case json_encoding::Utf16BE:
                                    *(nextOutputCh++) = static_cast<character_type>(u16ch);
                                    break;
                                case json_encoding::Utf32LE:
                                case json_encoding::Utf32BE:
                                    *(nextOutputCh++) = static_cast<character_type>(u16ch);
                                    break;
                                default:
                                    break;
                                }
                            }
                            currentElement.auxType = element::Unknown;
                            nextState = json_detail::state::String;
                        }
                        else
                        {
                            nextState = json_detail::state::EscapingUnicode;
                        }
                    }
                    break;
                default:
                    break;
                }
                if (currentState != nextState)
                {
                    if constexpr (json_detail::debug)
                        std::cout << "(" << to_string(nextState) << ")";
                    currentState = nextState;
                }
            }
            catch (std::exception& e)
            {
                create_parse_error(e.what());
                return false;
            }
            increment_cursor();
        }

        return true;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline bool basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::write(const std::string& aPath, const string_type& aIndent)
    {
        std::ofstream output{ aPath, std::ofstream::out | std::ofstream::trunc };
        return write(output, aIndent);
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    template <typename Elem, typename ElemTraits>
    inline bool basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::write(std::basic_ostream<Elem, ElemTraits>& aOutput, const string_type& aIndent)
    {
        static const string_type trueString = "true";
        static const string_type falseString = "false";
        static const string_type nullString = "null";
        int32_t level = 0;
        auto end = cend();
        auto indent = [&aOutput, &aIndent, &level]() 
        {
            for (int32_t l = 0; l < level; ++l)
                aOutput << aIndent;
        };
        for (auto i = cbegin(); i != end; ++i)
        {
            auto const& v = **i;
            indent();
            if (i.value().has_name())
            {
                if (!i.value().name_is_keyword())
                    aOutput << '\"' << i.value().name() << "\": ";
                else
                    aOutput << i.value().name() << ": ";
            }
            switch (i.value().type())
            {
            case json_type::Object:
                aOutput << '{';
                if (i.value().is_populated_composite())
                {
                    ++level;
                    aOutput << std::endl;
                }
                else
                    aOutput << '}';
                break;
            case json_type::Array:
                aOutput << '[';
                if (i.value().is_populated_composite())
                {
                    ++level;
                    aOutput << std::endl;
                }
                else
                    aOutput << ']';
                break;
            case json_type::Double:
                aOutput << std::get<json_double>(v);
                break;
            case json_type::Int64:
                aOutput << std::get<json_int64>(v);
                break;
            case json_type::Uint64:
                aOutput << std::get<json_uint64>(v);
                break;
            case json_type::Int:
                aOutput << std::get<json_int>(v);
                break;
            case json_type::Uint:
                aOutput << std::get<json_uint>(v);
                break;
            case json_type::String:
                aOutput << '\"';
                for (auto const& ch : std::get<json_string>(v))
                    switch (ch)
                    {
                    case '\"':
                        aOutput << "\\\"";
                        break;
                    case '\\':
                        aOutput << "\\\\";
                        break;
                    case '\b':
                        aOutput << "\\b";
                        break;
                    case '\f':
                        aOutput << "\\f";
                        break;
                    case '\n':
                        aOutput << "\\n";
                        break;
                    case '\r':
                        aOutput << "\\r";
                        break;
                    case '\t':
                        aOutput << "\\t";
                        break;
                    default:
                        if (static_cast<uint32_t>(ch) >= 32u)
                            aOutput << ch;
                        else
                            aOutput << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)ch;
                    }
                aOutput << '\"';
                break;
            case json_type::Bool:
                aOutput << (std::get<json_bool>(v) ? trueString : falseString);
                break;
            case json_type::Null:
                aOutput << nullString;
                break;
            case json_type::Keyword:
                aOutput << std::get<json_keyword>(v).text;
                break;
            default:
                break;
            }
            
            if (!i.value().is_composite() || i.value().is_empty_composite())
            {
                auto next = &i.value();
                bool needNewline = false;
                while (next->is_last_sibling() && next->has_parent())
                {
                    --level;
                    auto nextParent = &next->parent();
                    if (nextParent->type() == json_type::Array)
                    {
                        aOutput << std::endl;
                        indent();
                        aOutput << ']';
                        needNewline = true;
                    }
                    else if (nextParent->type() == json_type::Object)
                    {
                        aOutput << std::endl;
                        indent();
                        aOutput << '}';
                        needNewline = true;
                    }
                    if (!nextParent->is_last_sibling())
                    {
                        aOutput << ',';
                        needNewline = true;
                    }
                    next = nextParent;
                }
                if (needNewline && level > 0)
                    aOutput << std::endl;
            }
            if (!i.value().is_last_sibling() && (!i.value().is_composite() || i.value().is_empty_composite()))
                aOutput << ',' << std::endl;
        }
        return true;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline json_encoding basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::encoding() const
    {
        return iEncoding;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline const typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::json_string& basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::document() const
    {
        return iDocumentText;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline const typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::string_type& basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::error_text() const
    {
        return iErrorText;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline bool basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::has_root() const
    {
        return iRoot != std::nullopt;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline const typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::json_value& basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::croot() const
    {
        if (iRoot == std::nullopt)
            iRoot.emplace();
        return *iRoot;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline const typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::json_value& basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::root() const
    {
        return croot();
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::json_value& basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::root()
    {
        return const_cast<json_value&>(to_const(*this).root());
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline const typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::json_value& basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::at(const json_string& aPath) const
    {
        auto bitStart = aPath.begin();
        auto bitEnd = std::find(bitStart, aPath.end(), '.');
        json_value const* node = &root();
        if (bitEnd == bitStart)
            return *node;
        do
        {
            auto existing = std::find_if(node->begin(), node->end(), [&](auto&& child) { return child.name() == std::string_view{ &*bitStart, static_cast<std::string_view::size_type>(std::distance(bitStart, bitEnd)) };  });
            if (existing == node->end())
                throw json_path_not_found(aPath.to_std_string());
            node = &*existing;
            bitStart = (bitEnd != aPath.end() ? std::next(bitEnd) : bitEnd);
            bitEnd = std::find(bitStart, aPath.end(), '.');
        } while (bitStart != aPath.end());
        return *node;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::json_value& basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::at(const json_string& aPath)
    {
        return const_cast<json_value&>(to_const(*this).at(aPath));
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    template <typename Visitor>
    inline void basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::visit(Visitor&& aVisitor) const
    {
        if (has_root())
            root().visit(std::forward<Visitor>(aVisitor));
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    template <typename Visitor>
    inline void basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::visit(Visitor&& aVisitor)
    {
        if (has_root())
            root().visit(std::forward<Visitor>(aVisitor));
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::const_iterator basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::cbegin() const
    {
        return begin();
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::const_iterator basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::cend() const
    {
        return end();
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::const_iterator basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::begin() const
    {
        if (iRoot != std::nullopt)
            return const_iterator{ &*iRoot };
        return const_iterator{};
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::const_iterator basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::end() const
    {
        return const_iterator{};
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::iterator basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::begin()
    {
        if (iRoot != std::nullopt)
            return iterator{ &*iRoot };
        return iterator{};
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::iterator basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::end()
    {
        return iterator{};
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::string_type basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::to_error_text(const json_document_source_location& aDocumentSourceLocation, const string_type& aExtraInfo)
    {
        string_type text;
        if (!aExtraInfo.empty())
        {
            text += "(";
            text += aExtraInfo;
            text += ") ";
        }
        text += "line " + boost::lexical_cast<string_type>(aDocumentSourceLocation.line) + ", col " + boost::lexical_cast<string_type>(aDocumentSourceLocation.column);
        return text;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::string_type basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::to_error_text(const json_value& aNode, const string_type& aExtraInfo)
    {
        return to_error_text(aNode.document_source_location(), aExtraInfo);
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::string_type basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::to_error_text(const string_type& aExtraInfo) const
    {
        return to_error_text(iCursor, aExtraInfo);
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::json_string& basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::document()
    {
        return iDocumentText;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline json_type basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::context() const
    {
        if (!iCompositeValueStack.empty())
            return iCompositeValueStack.back()->type();
        return json_type::Unknown;
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    template <typename T>
    inline typename basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::json_value* basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::buy_value(element& aCurrentElement, T&& aValue)
    {
        switch (context())
        {
        case json_type::Array:
            {
                auto newObject = iCompositeValueStack.back()->buy_child(std::forward<T>(aValue));
                newObject->set_document_source_location(iCursor);
                if constexpr(std::is_same_v<typename std::remove_cv<typename std::remove_reference<T>::type>::type, json_array>)
                    newObject->template as<json_array>().set_contents(*newObject);
                else if constexpr(std::is_same_v<typename std::remove_cv<typename std::remove_reference<T>::type>::type, json_object>)
                    newObject->template as<json_object>().set_contents(*newObject);
                return newObject;
            }
        case json_type::Object:
            {
                auto newObject = iCompositeValueStack.back()->buy_child(std::forward<T>(aValue));
                newObject->set_document_source_location(iCursor);
                if constexpr(std::is_same_v<typename std::remove_cv<typename std::remove_reference<T>::type>::type, json_array>)
                    newObject->template as<json_array>().set_contents(*newObject);
                else if constexpr(std::is_same_v<typename std::remove_cv<typename std::remove_reference<T>::type>::type, json_object>)
                    newObject->template as<json_object>().set_contents(*newObject);
                if (std::holds_alternative<json_string>(aCurrentElement.name))
                    newObject->set_name(std::get<json_string>(aCurrentElement.name));
                else
                    newObject->set_name(std::get<json_keyword>(aCurrentElement.name));
                aCurrentElement.name = std::monostate{};
                return newObject;
            }
        default:
            root() = std::forward<T>(aValue);
            root().set_document_source_location(iCursor);
            return &root();
        }
    }

    template <json_syntax Syntax, typename Alloc, typename CharT, typename Traits, typename CharAlloc>
    inline void basic_json<Syntax, Alloc, CharT, Traits, CharAlloc>::create_parse_error(const string_type& aExtraInfo) const
    {
        iErrorText = to_error_text(aExtraInfo);
    }
}

