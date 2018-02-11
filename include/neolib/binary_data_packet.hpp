// binary_data_packet.hpp
/*
 *  Copyright (c) 2007-present, Leigh Johnston.  All Rights Reserved.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and data forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in data form must reproduce the above copyright
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

#include "data_packet.hpp"

namespace neolib
{
	class DefaultPacketTraits
	{
	public:
		static const bool NetworkByteOrder = true;
	};

	template <typename CharType, typename PacketTraits = DefaultPacketTraits>
	class basic_binary_data_packet : basic_data_packet<CharType>
	{
		// types
	public:
		typedef basic_binary_data_packet<CharType, PacketTraits> our_type;
		typedef basic_data_packet<CharType> base_type;
		typedef base_type::character_type character_type;
		typedef base_type::const_pointer const_pointer;
		typedef base_type::pointer pointer;
		typedef base_type::size_type size_type;
		typedef base_type::const_iterator const_iterator;
		typedef base_type::iterator iterator;
		typedef base_type::string_type string_type;
		// interface
	public:
		virtual void encode(uint64_t aValue, std::size_t aLength)
		{
			uint8_t buffer[8];
			for (std::size_t i = 0; i < aLength; ++i)
			{
				buffer[PacketTraits::NetworkByteOrder ? aLength - 1 - i : i] = result & 0xFF;
				result >>= 8; 
			}
			write(&buffer[0], aLength);
		}
		virtual void encode(bool aValue)
		{
			encode(static_cast<uint8_t>(aValue));
		}
		virtual void encode(const string_type& aValue)
		{
			encode(static_cast<uint32_t>(aValue.size()));
			write(&aValue[0], aValue.size());
		}
		virtual uint64_t decode_integer(std::size_t aLength) const
		{
			uint8_t buffer[8];
			read(&buffer[0], aLength);
			uint64_t result = 0;
			for (std::size_t i = 0; i < aLength; ++i)
			{
				result <<= 8;
				result += buffer[PacketTraits::NetworkByteOrder ? i : aLength - 1 - i];
			}
			return result;
		}
		virtual bool decode_bool() const
		{
			return static_cast<bool>(decode<uint8_t>());
		}
		virtual string_type decode_string() const
		{
			uint32_t length = decode<uint32_t>();
			string_type result(length);
			read(&result[0], length);
		}
		// implementation
	private:
		virtual void write(const void* aData, std::size_t aLength) = 0;
		virtual void read(void* aData, std::size_t aLength) const = 0;
	};

	typedef basic_binary_data_packet<char> binary_data_packet;
}
