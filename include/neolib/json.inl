// json.inl
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

#include <neolib/neolib.hpp>
#include <variant>
#include <unordered_map>
#include <fstream>
#include <iomanip>
#include <type_traits>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
#include <neolib/string_numeric.hpp>
#include <neolib/string_utf.hpp>
#include <neolib/type_traits.hpp>

namespace neolib
{
	namespace json_detail
	{
		enum class keyword
		{
			True,
			False,
			Null
		};

		enum class token
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
			DecimalPoint,
			Exponent,
			Whitespace,
			EndOfInput,
			TOKEN_COUNT
		};
		constexpr std::size_t TOKEN_COUNT = static_cast<std::size_t>(token::TOKEN_COUNT);
		constexpr token TXX = token::Invalid;
		constexpr token TOO = token::OpenObject;
		constexpr token TCO = token::CloseObject;
		constexpr token TOA = token::OpenArray;
		constexpr token TCA = token::CloseArray;
		constexpr token TCL = token::Colon;
		constexpr token TCM = token::Comma;
		constexpr token TQT = token::Quote;
		constexpr token TCH = token::Character;
		constexpr token TES = token::Escape;
		constexpr token TEU = token::EscapingUnicode;
		constexpr token TED = token::Escaped;
		constexpr token TPL = token::Plus;
		constexpr token TMI = token::Minus;
		constexpr token TDI = token::Digit;
		constexpr token THD = token::HexDigit;
		constexpr token TDP = token::DecimalPoint;
		constexpr token TEX = token::Exponent;
		constexpr token TWH = token::Whitespace;
		constexpr token TZZ = token::EndOfInput;

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
			STATE_COUNT,
		};
		constexpr std::size_t STATE_COUNT = static_cast<std::size_t>(state::STATE_COUNT);
		constexpr state SXX = state::Error;
		constexpr state SIG = state::Ignore;
		constexpr state SZZ = state::EndOfParse;
		constexpr state SEL = state::Element;
		constexpr state SOB = state::Object;
		constexpr state SAR = state::Array;
		constexpr state SCL = state::Close;
		constexpr state SVA = state::Value;
		constexpr state SVS = state::NeedValueSeparator;
		constexpr state SNV = state::NeedValue;
		constexpr state SOS = state::NeedObjectValueSeparator;
		constexpr state SOV = state::NeedObjectValue;
		constexpr state SKE = state::Keyword;
		constexpr state SNA = state::Name;
		constexpr state SEN = state::EndName;
		constexpr state SST = state::String;
		constexpr state SSE = state::StringEnd;
		constexpr state SN1 = state::NumberIntNeedDigit;
		constexpr state SN2 = state::NumberInt;
		constexpr state SN3 = state::NumberFracNeedDigit;
		constexpr state SN4 = state::NumberFrac;
		constexpr state SN5 = state::NumberExpSign;
		constexpr state SN6 = state::NumberExpIntNeedDigit;
		constexpr state SN7 = state::NumberExpInt;
		constexpr state SES = state::Escaping;
		constexpr state SED = state::Escaped;
		constexpr state SEU = state::EscapingUnicode;

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
			default:
				return std::string{ "??" };
			}
		}

		constexpr std::array<std::array<state, TOKEN_COUNT>, STATE_COUNT> sStateTables =
		{
			// state::Error
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
			    SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::Ignore
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::EndOfParse
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
			    SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::Element
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SIG, SZZ
			}},
			// state::Object
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SOB, SCL, SXX, SXX, SXX, SOV, SNA, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SIG, SXX
			}},
			// state::Array
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SOB, SXX, SAR, SCL, SXX, SXX, SST, SKE, SXX, SXX, SXX, SXX, SN1, SN2, SXX, SXX, SXX, SIG, SXX
			}},
			// state::Close
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::Value
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SOB, SCL, SAR, SCL, SXX, SXX, SST, SKE, SXX, SXX, SXX, SXX, SN1, SN2, SXX, SXX, SXX, SIG, SZZ
			}},
			// state::NeedValueSeparator
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SCL, SXX, SCL, SXX, SVA, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SIG, SXX
			}},
			// state::NeedValue
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SOB, SXX, SAR, SXX, SXX, SXX, SST, SKE, SXX, SXX, SXX, SXX, SN1, SN2, SXX, SXX, SXX, SIG, SXX
			}},
			// state::NeedObjectValueSeparator
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SCL, SXX, SCL, SXX, SOV, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SIG, SXX
			}},
			// state::NeedObjectValue
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SNA, SKE, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SIG, SXX
			}},
			// state::Keyword
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SCL, SXX, SCL, SEL, SEL, SXX, SKE, SXX, SXX, SXX, SXX, SXX, SKE, SXX, SXX, SXX, SEL, SXX
			}},
			// state::Name
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SEN, SNA, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::EndName
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SEL, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SIG, SXX
			}},
			// state::String
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SSE, SST, SES, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::StringEnd
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SCL, SXX, SCL, SXX, SEL, SXX, SKE, SXX, SXX, SXX, SXX, SXX, SKE, SXX, SXX, SXX, SEL, SXX
			}},
			// state::NumberIntNeedDigit
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN2, SXX, SXX, SXX, SXX, SXX
			}},
			// state::NumberInt
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SCL, SXX, SCL, SXX, SEL, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN2, SXX, SN3, SN5, SEL, SXX
			}},
			// state::NumberFracNeedDigit
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN4, SXX, SXX, SXX, SXX, SXX
			}},
			// state::NumberFrac
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SCL, SXX, SCL, SXX, SEL, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN4, SXX, SXX, SN5, SEL, SXX
			}},
			// state::NumberExpSign
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN6, SN6, SN7, SXX, SXX, SXX, SXX, SXX
			}},
			// state::NumberExpIntNeedDigit
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN7, SXX, SXX, SXX, SXX, SXX
			}},
			// state::NumberExpInt
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SCL, SXX, SCL, SXX, SEL, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN7, SXX, SXX, SXX, SEL, SXX
			}},
			// state::Escaping
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SEU, SED, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::Escaped
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::EscapingUnicode
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TEU  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH  TZZ
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SED, SXX, SXX, SXX, SXX
			}}
		};

		constexpr std::array<std::array<token, 256>, STATE_COUNT> sTokenTables =
		{ 
			// state::Error 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::Ignore 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::EndOfParse
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::Element
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TZZ, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::Object 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TQT, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TOA, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TOO, TXX, TCO, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::Array 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TQT, TXX, TCH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCM, TMI, TXX, TXX, // 0x2
			    TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TOA, TXX, TCA, TXX, TCH, // 0x5
			    TXX, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TOO, TXX, TXX, TXX, TXX, // 0x7
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x8
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x9
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xA
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xB
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xC
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xD
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xE
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xF
			}},
			// state::Close 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::Value 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TZZ, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TQT, TXX, TCH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TDI, TXX, TXX, // 0x2
			    TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TOA, TXX, TCA, TXX, TCH, // 0x5
			    TXX, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TOO, TXX, TCO, TXX, TXX, // 0x7
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x8
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x9
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xA
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xB
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xC
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xD
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xE
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xF
			}},
			// state::NeedValueSeparator
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCM, TXX, TXX, TXX, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCA, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCO, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::NeedValue 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TQT, TXX, TCH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TDI, TXX, TXX, // 0x2
			    TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TOA, TXX, TXX, TXX, TCH, // 0x5
			    TXX, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TOO, TXX, TXX, TXX, TXX, // 0x7
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x8
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x9
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xA
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xB
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xC
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xD
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xE
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xF
			}},
			// state::NeedObjectValueSeparator
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCM, TXX, TXX, TXX, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCO, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::NeedObjectValue 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TQT, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::Keyword
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TXX, TXX, TCH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCM, TXX, TXX, TXX, // 0x2
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCL, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TXX, TXX, TCA, TXX, TCH, // 0x5
			    TXX, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x8
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x9
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xA
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xB
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xC
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xD
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xE
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xF
			}},
			// state::Name 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TCH, TCH, TQT, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x2
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x3
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TES, TCH, TCH, TCH, // 0x5
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TXX, // 0x7
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x8
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x9
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xA
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xB
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xC
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xD
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xE
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xF
			}},
			// state::EndName 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCL, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::String 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TCH, TCH, TQT, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x2
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x3
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TES, TCA, TCH, TCH, // 0x5
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TXX, // 0x7
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x8
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x9
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xA
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xB
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xC
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xD
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xE
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xF
			}},
			// state::StringEnd
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TXX, TXX, TCH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCM, TXX, TXX, TXX, // 0x2
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCL, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TXX, TXX, TCA, TXX, TCH, // 0x5
			    TXX, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TXX, TXX, TCO, TXX, TXX, // 0x7
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x8
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x9
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xA
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xB
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xC
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xD
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xE
			    TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xF
			}},
			// state::NumberIntNeedDigit
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TEX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TEX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::NumberInt
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCM, TXX, TDP, TXX, // 0x2
			    TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TEX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCA, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TEX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::NumberFracNeedDigit
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::NumberFrac
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCM, TXX, TXX, TXX, // 0x2
			    TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TEX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCA, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TEX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::NumberExpSign
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TPL, TXX, TMI, TXX, TXX, // 0x2
			    TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::NumberExpIntNeedDigit
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::NumberExpInt
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TWH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCM, TXX, TXX, TXX, // 0x2
			    TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TCA, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::Escaping 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TXX, TXX, TED, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TED, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TED, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TED, TXX, TXX, TXX, TED, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TED, TXX, // 0x6
			    TXX, TXX, TED, TXX, TED, TEU, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::Escaped
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}},
			// state::EscapingUnicode
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x0
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
			    THD, THD, THD, THD, THD, THD, THD, THD, THD, THD, TXX, TXX, TXX, TXX, TXX, TXX, // 0x3
			    TXX, THD, THD, THD, THD, THD, THD, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x4
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x5
			    TXX, THD, THD, THD, THD, THD, THD, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x6
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x8
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x9
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xA
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xB
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xC
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xD
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xE
			    TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0xF
			}}
		};

		inline state next_state(state aCurrentState, char aToken)
		{
			auto stateIndex = static_cast<std::size_t>(aCurrentState);
			auto token = sTokenTables[stateIndex][static_cast<std::size_t>(aToken)];
			return sStateTables[stateIndex][static_cast<std::size_t>(token)];
		}

		template <typename StringViewType>
		struct hash_first_character
		{
			std::size_t operator()(const StringViewType& aString) const noexcept
			{
				return std::hash<typename StringViewType::value_type>{}(aString[0]);
			}
		};
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	template <typename IteratorTraits>
	class basic_json<Alloc, CharT, Traits, CharAlloc>::iterator_base : public IteratorTraits
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
		typedef typename const_selector_from_pointer<const value*, value*, pointer>::type value_pointer;
		typedef typename const_selector_from_pointer<const value&, value&, pointer>::type value_reference;
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
			return &**iValue;
		}
		reference operator*() const
		{
			return **iValue;
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

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	class basic_json<Alloc, CharT, Traits, CharAlloc>::iterator : public iterator_base<std::iterator<std::bidirectional_iterator_tag, value_type, std::ptrdiff_t, pointer, reference>>
	{
		friend class basic_json<Alloc, CharT, Traits, CharAlloc>;
	private:
		typedef iterator_base<std::iterator<std::bidirectional_iterator_tag, value_type, std::ptrdiff_t, pointer, reference>> base_type;
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
		using base_type::operator==;
		using base_type::operator!=;
	public:
		using base_type::value;
	};

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	class basic_json<Alloc, CharT, Traits, CharAlloc>::const_iterator : public iterator_base<std::iterator<std::bidirectional_iterator_tag, value_type, std::ptrdiff_t, const_pointer, const_reference>>
	{
		friend class basic_json<Alloc, CharT, Traits, CharAlloc>;
	private:
		typedef iterator_base<std::iterator<std::bidirectional_iterator_tag, value_type, std::ptrdiff_t, const_pointer, const_reference>> base_type;
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
		using base_type::operator==;
		using base_type::operator!=;
	public:
		using base_type::value;
	};

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline basic_json<Alloc, CharT, Traits, CharAlloc>::basic_json() : iEncoding{ json_detail::default_encoding<CharT>::DEFAULT_ENCODING }
	{
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline basic_json<Alloc, CharT, Traits, CharAlloc>::basic_json(const std::string& aPath, bool aValidateUtf) : iEncoding{ json_detail::default_encoding<CharT>::DEFAULT_ENCODING }
	{
		if (!read(aPath, aValidateUtf))
			throw json_error(error_text());
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	template <typename Elem, typename ElemTraits>
	inline basic_json<Alloc, CharT, Traits, CharAlloc>::basic_json(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf) : iEncoding{ json_detail::default_encoding<CharT>::DEFAULT_ENCODING }
	{
		if (!read(aInput, aValidateUtf))
			throw json_error(error_text());
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline void basic_json<Alloc, CharT, Traits, CharAlloc>::clear()
	{
		document().clear();
		iUtf16HighSurrogate = std::nullopt;
	}
		
	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::read(const std::string& aPath, bool aValidateUtf)
	{
		std::ifstream input{ aPath, std::ios::binary };
		if (!input)
		{
			iErrorText = "failed to open JSON file '" + aPath + "'";
			return false;
		}
		bool ok = do_read(input, aValidateUtf);
		if (ok)
			ok = do_parse();
		if (!ok)
			iErrorText = "failed to parse JSON file '" + aPath + "', " + iErrorText;
		return ok;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	template <typename Elem, typename ElemTraits>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::read(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf)
	{
		if (!aInput)
		{
			iErrorText = "failed to read JSON text";
			return false;
		}
		bool ok = do_read(aInput, aValidateUtf);
		if (ok)
			ok = do_parse();
		if (!ok)
			iErrorText = "failed to parse JSON text, " + iErrorText;
		return ok;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	template <typename Elem, typename ElemTraits>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::do_read(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf)
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

		if (json_detail::next_state(json_detail::state::Value, document().back()) != json_detail::state::Ignore)
			document().push_back(character_type{ '\n' });
		document().push_back(character_type{ '\0' });

		if (aValidateUtf && !neolib::check_utf8(document().as_view()))
		{
			iErrorText = "invalid utf-8";
			return false;
		}

		return true;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::do_parse()
	{
		json_detail::state currentState = json_detail::state::Value;
		json_detail::state nextState;
		element currentElement = {};

		auto nextInputCh = &*document().begin();
		auto nextOutputCh = nextInputCh;

		// Main parse loop
		for(;;)
		{
#ifdef DEBUG_JSON
			if (*nextInputCh != '\n')
				std::cout << *nextInputCh;
			else
				std::cout << "\\n";
#endif
			try
			{
				nextState = json_detail::next_state(currentState, *nextInputCh);
				switch (nextState)
				{
				case json_detail::state::Ignore:
					break;
				case json_detail::state::Error:
					create_parse_error(nextInputCh);
					return false;
				case json_detail::state::EndOfParse:
					if (nextInputCh != &iDocumentText.back())
					{
						create_parse_error(nextInputCh);
						return false;
					}
					return true;
				default:
					{
						if (currentState == nextState && nextState != json_detail::state::Object && nextState != json_detail::state::Array)
						{
							switch (currentState)
							{
							case json_detail::state::String:
							case json_detail::state::Keyword:
							case json_detail::state::Name:
								if (currentElement.start != nextOutputCh)
									*nextOutputCh++ = *nextInputCh;
								break;
							}
							++nextInputCh;
							continue;
						}
#ifdef DEBUG_JSON
						bool changedState = false;
						std::cout << "(" << to_string(currentState) << " -> " << to_string(nextState) << ")";
#endif

						switch (nextState)
						{
						case json_detail::state::Close:
						case json_detail::state::Element:
							switch (currentElement.type)
							{
							case element::Unknown:
								break;
							case element::String:
							case element::Name:
								{
									json_string newString{ currentElement.start, currentElement.start == nextOutputCh ? nextInputCh - 1 : nextOutputCh };
									if (context() == json_type::Object && currentElement.name == std::nullopt)
										currentElement.name = newString;
									else
										buy_value(currentElement, newString);
								}
								break;
							case element::Number:
								{
									json_string newNumber{ currentElement.start, currentElement.start == nextOutputCh ? nextInputCh : nextOutputCh };
									if (currentState == json_detail::state::NumberInt)
									{
										std::visit([this, &currentElement](auto&& arg)
										{ 
											buy_value(currentElement, arg); 
										}, neolib::string_to_number(newNumber.as_view()));
									}
									else
										buy_value(currentElement, neolib::string_to_double(newNumber.as_view()));
								}
								break;
							case element::Keyword:
								{
									static const std::unordered_map<typename json_string::string_view_type, json_detail::keyword, json_detail::hash_first_character<typename json_string::string_view_type>> sJsonKeywords =
									{
										{ "true", json_detail::keyword::True },
										{ "false", json_detail::keyword::False },
										{ "null",  json_detail::keyword::Null },
									};
									auto keywordText = json_string{ currentElement.start, currentElement.start == nextOutputCh ? nextInputCh : nextOutputCh };
									auto keyword = sJsonKeywords.find(keywordText);
									if (keyword != sJsonKeywords.end())
									{
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
										buy_value(currentElement, json_keyword{ keywordText }); // todo: make custom keywords optional and raise parser error if not enabled
								}
								break;
							}
							if (nextState == json_detail::state::Close)
								iCompositeValueStack.pop_back();
							switch (context())
							{
							case json_type::Object:
								if (currentElement.type == element::Name)
									nextState = json_detail::state::NeedValue;
								else if (nextState == json_detail::state::Close)
									nextState = json_detail::state::NeedObjectValueSeparator;
								else if (*nextInputCh == ',')
									nextState = json_detail::state::NeedObjectValue;
								else
									nextState = json_detail::state::NeedObjectValueSeparator;
#ifdef DEBUG_JSON
								changedState = true;
#endif
								break;
							case json_type::Array:
								if (*nextInputCh == ',')
									nextState = json_detail::state::NeedValue;
								else
									nextState = json_detail::state::NeedValueSeparator;
#ifdef DEBUG_JSON
								changedState = true;
#endif
								break;
							default:
								if (nextState == json_detail::state::Close)
								{
									nextState = json_detail::state::Value;
#ifdef DEBUG_JSON
									changedState = true;
#endif
								}
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
						case json_detail::state::NumberInt:
							currentElement.type = element::Number;
							currentElement.start = nextInputCh;
							break;
						case json_detail::state::Array:
							{
								value* newArray = buy_value(currentElement, json_array{});
								iCompositeValueStack.push_back(newArray);
								nextState = json_detail::state::Value;
#ifdef DEBUG_JSON
								changedState = true;
#endif
							}
						break;
							case json_detail::state::Object:
							{
								value* newObject = buy_value(currentElement, json_object{});
								iCompositeValueStack.push_back(newObject);
#ifdef DEBUG_JSON
								changedState = true;
#endif
							}
						break;
						case json_detail::state::Keyword:
							currentElement.type = element::Keyword;
							currentElement.start = (nextOutputCh = nextInputCh);
							break;
						case json_detail::state::Escaped:
							{
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
									}
									nextState = json_detail::state::String;
#ifdef DEBUG_JSON
									changedState = true;
#endif
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
													(*nextOutputCh++) = static_cast<character_type>(utf8_to_utf32(utf16_to_utf8(std::u16string{ &surrogatePair[0], 2 }))[0]);
												}
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
											}
										}
										currentElement.auxType = element::Unknown;
										nextState = json_detail::state::String;
									}
									else
									{
										nextState = json_detail::state::EscapingUnicode;
									}
#ifdef DEBUG_JSON
									changedState = true;
#endif
								}
							}
							break;
						}
#ifdef DEBUG_JSON
						if (changedState)
							std::cout << "(" << to_string(nextState) << ")";
#endif
						currentState = nextState;
					}
				}
			}
			catch (std::exception& e)
			{
				create_parse_error(nextInputCh, e.what());
				return false;
			}
			++nextInputCh;
		}

		return true;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::write(const std::string& aPath, const string_type& aIndent)
	{
		std::ofstream output{ aPath, std::ofstream::out | std::ofstream::trunc };
		return write(output, aIndent);
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	template <typename Elem, typename ElemTraits>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::write(std::basic_ostream<Elem, ElemTraits>& aOutput, const string_type& aIndent)
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
			indent();
			if (i.value().has_name())
				aOutput << '\"' << i.value().name() << "\": ";
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
				aOutput << static_variant_cast<json_double>(*i);
				break;
			case json_type::Int64:
				aOutput << static_variant_cast<json_int64>(*i);
				break;
			case json_type::Uint64:
				aOutput << static_variant_cast<json_uint64>(*i);
				break;
			case json_type::Int:
				aOutput << static_variant_cast<json_int>(*i);
				break;
			case json_type::Uint:
				aOutput << static_variant_cast<json_uint>(*i);
				break;
			case json_type::String:
				aOutput << '\"';
				for (auto const& ch : static_variant_cast<const json_string&>(*i))
					switch (ch)
					{
					case '\"':
						aOutput << "\\\"";
						break;
					case '\\':
						aOutput << "\\\\";
						break;
					case '/':
						aOutput << "\\/";
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
				aOutput << (static_variant_cast<json_bool>(*i) ? trueString : falseString);
				break;
			case json_type::Null:
				aOutput << nullString;
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

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline json_encoding basic_json<Alloc, CharT, Traits, CharAlloc>::encoding() const
	{
		return iEncoding;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline typename const basic_json<Alloc, CharT, Traits, CharAlloc>::json_string& basic_json<Alloc, CharT, Traits, CharAlloc>::document() const
	{
		return iDocumentText;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline typename const basic_json<Alloc, CharT, Traits, CharAlloc>::string_type& basic_json<Alloc, CharT, Traits, CharAlloc>::error_text() const
	{
		return iErrorText;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::has_root() const
	{
		return iRoot != std::nullopt;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline const typename basic_json<Alloc, CharT, Traits, CharAlloc>::value& basic_json<Alloc, CharT, Traits, CharAlloc>::root() const
	{
		if (has_root())
			return *iRoot;
		throw no_root();
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline typename basic_json<Alloc, CharT, Traits, CharAlloc>::value& basic_json<Alloc, CharT, Traits, CharAlloc>::root()
	{
		return const_cast<value&>(const_cast<const self_type*>(this)->root());
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline void basic_json<Alloc, CharT, Traits, CharAlloc>::accept(i_visitor& aVisitor)
	{
		if (has_root())
			root().accept(aVisitor);
		else
			throw no_root();
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	typename basic_json<Alloc, CharT, Traits, CharAlloc>::const_iterator basic_json<Alloc, CharT, Traits, CharAlloc>::cbegin() const
	{
		return begin();
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	typename basic_json<Alloc, CharT, Traits, CharAlloc>::const_iterator basic_json<Alloc, CharT, Traits, CharAlloc>::cend() const
	{
		return end();
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	typename basic_json<Alloc, CharT, Traits, CharAlloc>::const_iterator basic_json<Alloc, CharT, Traits, CharAlloc>::begin() const
	{
		if (iRoot != std::nullopt)
			return const_iterator{ &*iRoot };
		return const_iterator{};
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	typename basic_json<Alloc, CharT, Traits, CharAlloc>::const_iterator basic_json<Alloc, CharT, Traits, CharAlloc>::end() const
	{
		return const_iterator{};
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	typename basic_json<Alloc, CharT, Traits, CharAlloc>::iterator basic_json<Alloc, CharT, Traits, CharAlloc>::begin()
	{
		if (iRoot != std::nullopt)
			return iterator{ &*iRoot };
		return iterator{};
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	typename basic_json<Alloc, CharT, Traits, CharAlloc>::iterator basic_json<Alloc, CharT, Traits, CharAlloc>::end()
	{
		return iterator{};
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline typename basic_json<Alloc, CharT, Traits, CharAlloc>::json_string& basic_json<Alloc, CharT, Traits, CharAlloc>::document()
	{
		return iDocumentText;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline json_type basic_json<Alloc, CharT, Traits, CharAlloc>::context() const
	{
		if (!iCompositeValueStack.empty())
			return iCompositeValueStack.back()->type();
		return json_type::Unknown;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	template <typename T>
	inline typename basic_json<Alloc, CharT, Traits, CharAlloc>::value* basic_json<Alloc, CharT, Traits, CharAlloc>::buy_value(element& aCurrentElement, T&& aValue)
	{
		switch (context())
		{
		case json_type::Array:
			return iCompositeValueStack.back()->buy_child(std::forward<T>(aValue));
		case json_type::Object:
			{
				auto newObject = iCompositeValueStack.back()->buy_child(std::forward<T>(aValue));
				newObject->set_name(*aCurrentElement.name);
				aCurrentElement.name = std::nullopt;
				return newObject;
			}
		default:
			iRoot = value{};
			*iRoot = std::forward<T>(aValue);
			return &*iRoot;
		}
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline void basic_json<Alloc, CharT, Traits, CharAlloc>::create_parse_error(const character_type* aDocumentPos, const string_type& aExtraInfo)
	{
		uint32_t line = 1;
		uint32_t col = 1;
		for (auto pos = &*document().as_view().begin(); pos != aDocumentPos; ++pos)
		{
			if (*pos == '\n')
			{
				++line;
				col = 1;
			}
			else
				++col;
		}
		iErrorText.clear();
		if (!aExtraInfo.empty())
		{
			iErrorText += "(";
			iErrorText += aExtraInfo;
			iErrorText += ") ";
		}
		iErrorText += "line " + boost::lexical_cast<std::string>(line) + ", col " + boost::lexical_cast<std::string>(col);
	}
}

