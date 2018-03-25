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
			Whitespace
		};

		enum class state
		{
			Value,
			Name,
			EndName,
			Object,
			Array,
			String,
			NumberIntNeedDigit,
			NumberInt,
			NumberFracNeedDigit,
			NumberFrac,
			NumberExpSign,
			NumberExpIntNeedDigit,
			NumberExpInt,
			Escaping,
			EscapingUnicode,
			STATE_COUNT
		};
		constexpr std::size_t STATE_COUNT = static_cast<std::size_t>(state::STATE_COUNT);

		constexpr token TIN = token::Invalid;
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

		const std::array<std::array<token, 256>, STATE_COUNT> sTokenTables =
		{ 
			// state::Value 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TWH, TWH, TIN, TIN, TWH, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TWH, TIN, TQT, TIN, TCH, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TDI, TIN, TIN, // 0x2
				TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TOA, TIN, TIN, TIN, TCH, // 0x5
				TIN, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TOO, TIN, TIN, TIN, TIN, // 0x7
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
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TWH, TWH, TIN, TIN, TWH, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TWH, TIN, TIN, TIN, TCH, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x2
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCL, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TIN, TIN, TIN, TIN, TCH, // 0x5
				TIN, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TIN, TIN, TIN, TIN, TIN, // 0x7
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
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TWH, TWH, TIN, TIN, TWH, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TWH, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x2
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TCL, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x4
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x5
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x6
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x7
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x8
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x9
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xA
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xB
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xC
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xD
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xE
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xF
			}},
			// state::Object 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TWH, TWH, TIN, TIN, TWH, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TWH, TIN, TIN, TIN, TCH, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x2
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TIN, TIN, TIN, TIN, TCH, // 0x5
				TIN, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TIN, TIN, TCO, TIN, TIN, // 0x7
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x8
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x9
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xA
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xB
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xC
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xD
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xE
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xF
			}},
			// state::Array 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TWH, TWH, TIN, TIN, TWH, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TWH, TIN, TQT, TIN, TCH, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TCM, TDI, TDI, TIN, // 0x2
				TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TOA, TIN, TCA, TIN, TCH, // 0x5
				TIN, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TOO, TIN, TIN, TIN, TIN, // 0x7
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x8
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x9
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xA
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xB
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xC
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xD
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xE
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0xF
			}},
			// state::String 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TCH, TCH, TQT, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x2
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x3
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x4
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TES, TCH, TCH, TCH, // 0x5
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, // 0x6
				TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TCH, TIN, // 0x7
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
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x2
				TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TIN, TIN, TIN, TIN, TEX, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x4
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x5
				TIN, TIN, TIN, TIN, TIN, TEX, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x6
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x7
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x8
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x9
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xA
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xB
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xC
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xD
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xE
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xF
			}},
			// state::NumberInt
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TWH, TWH, TIN, TIN, TWH, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TDP, TIN, // 0x2
				TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TIN, TIN, TIN, TIN, TEX, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x4
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x5
				TIN, TIN, TIN, TIN, TIN, TEX, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x6
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x7
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x8
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x9
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xA
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xB
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xC
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xD
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xE
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xF
			}},
			// state::NumberFracNeedDigit
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x2
				TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x4
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x5
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x6
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x7
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x8
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x9
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xA
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xB
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xC
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xD
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xE
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xF
			}},
			// state::NumberFrac
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TWH, TWH, TIN, TIN, TWH, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x2
				TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TIN, TIN, TIN, TIN, TEX, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x4
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x5
				TIN, TIN, TIN, TIN, TIN, TEX, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x6
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x7
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x8
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x9
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xA
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xB
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xC
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xD
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xE
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xF
			}},
			// state::NumberExpSign
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TWH, TWH, TIN, TIN, TWH, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TPL, TIN, TMI, TIN, TIN, // 0x2
				TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x4
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x5
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x6
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x7
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x8
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x9
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xA
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xB
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xC
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xD
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xE
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xF
			}},
			// state::NumberExpIntNeedDigit
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x2
				TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x4
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x5
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x6
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x7
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x8
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x9
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xA
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xB
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xC
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xD
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xE
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xF
			}},
			// state::NumberExpInt
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TWH, TWH, TIN, TIN, TWH, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x2
				TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TDI, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x4
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x5
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x6
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x7
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x8
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x9
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xA
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xB
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xC
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xD
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xE
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xF
			}},
			// state::Escaping 
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TIN, TIN, TED, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TED, // 0x2
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x4
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TED, TIN, TIN, TIN, // 0x5
				TIN, TIN, TED, TIN, TIN, TIN, TED, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TED, TIN, // 0x6
				TIN, TIN, TED, TIN, TED, TED, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x7
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x8
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x9
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xA
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xB
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xC
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xD
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xE
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xF
			}},
			// state::EscapingUnicode
			std::array<token, 256>
			{{//0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TWH, TWH, TIN, TIN, TWH, TIN, TIN, // 0x0
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x1
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x2
				THD, THD, THD, THD, THD, THD, THD, THD, THD, THD, TIN, TIN, TIN, TIN, TIN, TIN, // 0x3
				TIN, THD, THD, THD, THD, THD, THD, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x4
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x5
				TIN, THD, THD, THD, THD, THD, THD, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x6
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x7
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x8
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0x9
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xA
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xB
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xC
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xD
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xE
				TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, TIN, // 0xF
			}}
		};
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

		if (aValidateUtf8 && !neolib::check_utf8(document().as_view()))
		{
			iErrorText = "invalid utf-8";
			return false;
		}

		// todo: parse

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
