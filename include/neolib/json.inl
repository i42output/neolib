// json.inl
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

#include <neolib/neolib.hpp>
#include <unordered_map>
#include <fstream>
#include <neolib/string_utils.hpp>

namespace neolib
{
	namespace json_detail
	{
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
			Escaped,
			Plus,
			Minus,
			Digit,
			HexDigit,
			DecimalPoint,
			Exponent,
			Whitespace,
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
		constexpr token TED = token::Escaped;
		constexpr token TPL = token::Plus;
		constexpr token TMI = token::Minus;
		constexpr token TDI = token::Digit;
		constexpr token THD = token::HexDigit;
		constexpr token TDP = token::DecimalPoint;
		constexpr token TEX = token::Exponent;
		constexpr token TWH = token::Whitespace;

		enum class state
		{
			Error,
			Ignore,
			Element,
			Object,
			Array,
			Close,
			Value,
			Keyword,
			Name,
			EndName,
			String,
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
		constexpr state SEL = state::Element;
		constexpr state SOB = state::Object;
		constexpr state SAR = state::Array;
		constexpr state SCL = state::Close;
		constexpr state SVA = state::Value;
		constexpr state SKE = state::Keyword;
		constexpr state SNA = state::Name;
		constexpr state SEN = state::EndName;
		constexpr state SST = state::String;
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

		const std::array<std::array<state, TOKEN_COUNT>, STATE_COUNT> sStateTables = 
		{
			// state::Error
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::Ignore
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::Element
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::Object
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SOB, SCL, SXX, SXX, SXX, SXX, SNA, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SIG
			}},
			// state::Array
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SOB, SXX, SAR, SCL, SXX, SXX, SST, SKE, SXX, SXX, SXX, SN1, SN2, SXX, SXX, SXX, SIG
			}},
			// state::Close
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::Value
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SOB, SXX, SAR, SXX, SXX, SVA, SST, SKE, SXX, SXX, SXX, SN1, SN2, SXX, SXX, SXX, SIG
			}},
			// state::Keyword
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SVA, SXX, SXX, SKE, SXX, SXX, SXX, SXX, SKE, SXX, SXX, SXX, SEL
			}},
			// state::Name
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SEN, SNA, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::EndName
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SVA, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SIG
			}},
			// state::String
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SEL, SST, SES, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::NumberIntNeedDigit
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN2, SXX, SXX, SXX, SXX
			}},
			// state::NumberInt
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN2, SXX, SN3, SN5, SEL
			}},
			// state::NumberFracNeedDigit
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN4, SXX, SXX, SXX, SXX
			}},
			// state::NumberFrac
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN4, SXX, SXX, SN5, SEL
			}},
			// state::NumberExpSign
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN6, SN6, SXX, SXX, SXX, SXX, SXX
			}},
			// state::NumberExpIntNeedDigit
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN7, SXX, SXX, SXX, SXX
			}},
			// state::NumberExpInt
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SN7, SXX, SXX, SXX, SEL
			}},
			// state::Escaping
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SED, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::Escaped
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}},
			// state::EscapingUnicode
			std::array<state, TOKEN_COUNT>
			{{//TXX  TOO  TCO  TOA  TCA  TCL  TCM  TQT  TCH  TES  TED  TPL  TMI  TDI  THD  TDP  TEX  TWH
				SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX, SXX
			}}
		};

		const std::array<std::array<token, 256>, STATE_COUNT> sTokenTables =
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
			// state::Element
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
			// state::Object 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
				TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
				TWH, TXX, TQT, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
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
			// state::Keyword 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TWH, TWH, TXX, TXX, TWH, TXX, TXX, // 0x0
				TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x1
				TWH, TXX, TXX, TXX, TCH, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x2
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCL, TXX, TXX, TXX, TXX, TXX, // 0x3
				TXX, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TXX, TXX, TXX, TXX, TCH, // 0x5
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
				TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TDP, TXX, // 0x2
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
				TXX, TXX, TED, TXX, TED, TED, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, // 0x7
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
			auto token = sTokenTables[stateIndex][aToken];
			return sStateTables[stateIndex][static_cast<std::size_t>(token)];
		}
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline basic_json<Alloc, CharT, Traits, CharAlloc>::basic_json()
	{
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline basic_json<Alloc, CharT, Traits, CharAlloc>::basic_json(const std::string& aPath, bool aValidateUtf8)
	{
		if (!read(aPath, aValidateUtf8))
			throw json_error(error_text());
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	template <typename Elem, typename ElemTraits>
	inline basic_json<Alloc, CharT, Traits, CharAlloc>::basic_json(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf8)
	{
		if (!read(aInput, aValidateUtf8))
			throw json_error(error_text());
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline void basic_json<Alloc, CharT, Traits, CharAlloc>::clear()
	{
		document().clear();
	}
		
	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::read(const std::string& aPath, bool aValidateUtf8)
	{
		std::ifstream input{ aPath };
		if (!input)
		{
			iErrorText = "failed to open '" + aPath + "'";
			return false;
		}
		return read(input, aValidateUtf8);
	}
		
	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	template <typename Elem, typename ElemTraits>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::read(std::basic_istream<Elem, ElemTraits>& aInput, bool aValidateUtf8)
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

		if (aValidateUtf8 && !neolib::check_utf8(document().as_view()))
		{
			iErrorText = "invalid utf-8";
			return false;
		}

		const auto& view = document().as_view();
		auto nextCh = view.begin();
		auto endCh = view.end();
		std::size_t idx = 0;
		json_detail::state previousState = json_detail::state::Value;
		json_detail::state currentState = json_detail::state::Value;
		for (json_detail::state nextState = json_detail::next_state(previousState, *nextCh);
			nextCh != endCh; 
			nextState = json_detail::next_state(currentState, *(++nextCh)))
		{
			std::cout << *nextCh;
			if (nextState == json_detail::state::Ignore)
				continue;
			currentState = nextState;
			if (currentState != previousState)
				std::cout << "(" << (int)previousState << " >> " << (int)currentState << ")";
			previousState = currentState;
		}

		return true;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::write(const std::string& aPath)
	{
		std::ofstream output{ aPath, std::ofstream::out | std::ofstream::trunc };
		return write(output);
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	template <typename Elem, typename ElemTraits>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::write(std::basic_ostream<Elem, ElemTraits>& aOutput)
	{
		// todo: generate

		return true;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline typename const basic_json<Alloc, CharT, Traits, CharAlloc>::json_string& basic_json<Alloc, CharT, Traits, CharAlloc>::document() const
	{
		return iDocumentText;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline typename const basic_json<Alloc, CharT, Traits, CharAlloc>::string& basic_json<Alloc, CharT, Traits, CharAlloc>::error_text() const
	{
		return iErrorText;
	}

	template <typename Alloc, typename CharT, typename Traits, typename CharAlloc>
	inline bool basic_json<Alloc, CharT, Traits, CharAlloc>::has_root() const
	{
		return iRoot != boost::none;
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
	inline typename basic_json<Alloc, CharT, Traits, CharAlloc>::json_string& basic_json<Alloc, CharT, Traits, CharAlloc>::document()
	{
		return iDocumentText;
	}
}

