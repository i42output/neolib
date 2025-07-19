// leb128.hpp
/*
 *  Copyright (c) 2025 Leigh Johnston.
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
#include <cstdint>
#include <array>
#include <variant>
#include <stdexcept>

namespace neolib::leb128
{
    using leb128_1 = std::array<std::uint8_t, 1>;
    using leb128_2 = std::array<std::uint8_t, 2>;
    using leb128_3 = std::array<std::uint8_t, 3>;
    using leb128_4 = std::array<std::uint8_t, 4>;
    using leb128_5 = std::array<std::uint8_t, 5>;
    using leb128_6 = std::array<std::uint8_t, 6>;
    using leb128_7 = std::array<std::uint8_t, 7>;
    using leb128_8 = std::array<std::uint8_t, 8>;
    using leb128_9 = std::array<std::uint8_t, 9>;
    using leb128_10 = std::array<std::uint8_t, 10>;

    using leb128 = std::variant<
        leb128_1, leb128_2, leb128_3, leb128_4, leb128_5,
        leb128_6, leb128_7, leb128_8, leb128_9, leb128_10>;

    using uleb128 = leb128;
    using sleb128 = leb128;

    /// Encode any 64-bit unsigned integer as ULEB128.
    /// Returns the concrete array type whose size matches the encoded length.
    inline uleb128 ULEB128(std::uint64_t value)
    {
        std::uint8_t buf[10]{};     // 10 bytes is the max for 64 bits (10 × 7 = 70 payload bits).
        std::size_t  len = 0;

        // Emit little-endian, base-128, adding the continuation bit (0x80)
        // to every byte except the last.
        do {
            std::uint8_t byte = static_cast<std::uint8_t>(value & 0x7F);
            value >>= 7;
            if (value != 0) byte |= 0x80;   // set continuation
            buf[len++] = byte;
        } while (value != 0 && len < 10);

        // Runtime length ? concrete type
        switch (len) {
        case 1:  return leb128_1{ buf[0] };
        case 2:  return leb128_2{ buf[0], buf[1] };
        case 3:  return leb128_3{ buf[0], buf[1], buf[2] };
        case 4:  return leb128_4{ buf[0], buf[1], buf[2], buf[3] };
        case 5:  return leb128_5{ buf[0], buf[1], buf[2], buf[3], buf[4] };
        case 6:  return leb128_6{ buf[0], buf[1], buf[2], buf[3], buf[4], buf[5] };
        case 7:  return leb128_7{ buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6] };
        case 8:  return leb128_8{ buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] };
        case 9:  return leb128_9{ buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8] };
        case 10: return leb128_10{ buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9] };
        default:
            throw std::logic_error("ULEB128 encoding produced an invalid length");
        }
    }

    /// Encode any 64-bit *signed* integer as SLEB128.
    /// Returns the concrete array type whose size matches the encoded length.
    inline sleb128 SLEB128(std::int64_t value)
    {
        std::uint8_t buf[10]{};   // 10 bytes is still enough (10×7 = 70 payload bits)
        std::size_t  len = 0;

        bool more;
        do {
            std::uint8_t byte = static_cast<std::uint8_t>(value & 0x7F);
            value >>= 7;

            // Sign-extend after shift so that `value` is either all 0s or all 1s
            // if we’re done.
            const bool sign_bit = (byte & 0x40) != 0;
            more = !((value == 0 && !sign_bit) || (value == -1 && sign_bit));

            if (more) byte |= 0x80;          // set continuation bit
            buf[len++] = byte;
        } while (more && len < 10);

        // Runtime length ? concrete type
        switch (len) {
        case 1:  return leb128_1{ buf[0] };
        case 2:  return leb128_2{ buf[0], buf[1] };
        case 3:  return leb128_3{ buf[0], buf[1], buf[2] };
        case 4:  return leb128_4{ buf[0], buf[1], buf[2], buf[3] };
        case 5:  return leb128_5{ buf[0], buf[1], buf[2], buf[3], buf[4] };
        case 6:  return leb128_6{ buf[0], buf[1], buf[2], buf[3], buf[4], buf[5] };
        case 7:  return leb128_7{ buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6] };
        case 8:  return leb128_8{ buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] };
        case 9:  return leb128_9{ buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8] };
        case 10: return leb128_10{ buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9] };
        default:
            throw std::logic_error("SLEB128 encoding produced an invalid length");
        }
    }

    using oleb128 = std::variant<
        leb128_1, leb128_2, leb128_3, leb128_4, leb128_5>;

    /// Encode any 32-bit unsigned integer as LEB128 (for opcodes).
    /// Returns the concrete array type whose size matches the encoded length.
    inline oleb128 LEB128(std::uint32_t value)
    {
        std::uint8_t buf[5]{};     // 5 bytes is the max for 32 bits (5 × 7 = 35 payload bits).
        std::size_t  len = 0;

        // Emit little-endian, base-128, adding the continuation bit (0x80)
        // to every byte except the last.
        do {
            std::uint8_t byte = static_cast<std::uint8_t>(value & 0x7F);
            value >>= 7;
            if (value != 0) byte |= 0x80;   // set continuation
            buf[len++] = byte;
        } while (value != 0 && len < 5);

        // Runtime length ? concrete type
        switch (len) {
        case 1:  return leb128_1{ buf[0] };
        case 2:  return leb128_2{ buf[0], buf[1] };
        case 3:  return leb128_3{ buf[0], buf[1], buf[2] };
        case 4:  return leb128_4{ buf[0], buf[1], buf[2], buf[3] };
        case 5:  return leb128_5{ buf[0], buf[1], buf[2], buf[3], buf[4] };
        default:
            throw std::logic_error("LEB128 encoding produced an invalid length");
        }
    }
}
