// data_packet.hpp
/*
 *  Copyright (c) 2012 Leigh Johnston.
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
#include <string>

#include "i_packet.hpp"

namespace neolib
{
	namespace detail
	{
		template <typename PacketType, typename T> struct decoder;
		template <typename PacketType> struct decoder<PacketType, uint8_t> { uint8_t operator()(const PacketType& p) const { return static_cast<uint8_t>(p.decode_integer(sizeof(uint8_t))); } };
		template <typename PacketType> struct decoder<PacketType, uint16_t> { uint16_t operator()(const PacketType& p) const { return static_cast<uint16_t>(p.decode_integer(sizeof(uint16_t))); } };
		template <typename PacketType> struct decoder<PacketType, uint32_t> { uint32_t operator()(const PacketType& p) const { return static_cast<uint32_t>(p.decode_integer(sizeof(uint32_t))); } };
		template <typename PacketType> struct decoder<PacketType, uint64_t> { uint64_t operator()(const PacketType& p) const { return static_cast<uint64_t>(p.decode_integer(sizeof(uint64_t))); } };
		template <typename PacketType> struct decoder<PacketType, int8_t> { uint8_t operator()(const PacketType& p) const { return static_cast<int8_t>(p.decode_integer(sizeof(int8_t))); } };
		template <typename PacketType> struct decoder<PacketType, int16_t> { uint8_t operator()(const PacketType& p) const { return static_cast<int16_t>(p.decode_integer(sizeof(int16_t))); } };
		template <typename PacketType> struct decoder<PacketType, int32_t> { int32_t operator()(const PacketType& p) const { return static_cast<int32_t>(p.decode_integer(sizeof(int32_t))); } };
		template <typename PacketType> struct decoder<PacketType, int64_t> { int64_t operator()(const PacketType& p) const { return static_cast<int64_t>(p.decode_integer(sizeof(int64_t))); } };
		template <typename PacketType> struct decoder<PacketType, bool> { bool operator()(const PacketType& p) const { return p.decode_bool(); } };
		template <typename PacketType> struct decoder<PacketType, typename PacketType::string_type> { typename PacketType::string_type operator()(const PacketType& p) const { return p.decode_string(); } };
	}

	template <typename CharType>
	class basic_data_packet : i_basic_packet<CharType>
	{
		// types
	public:
		typedef basic_data_packet<CharType> our_type;
		typedef i_basic_packet<CharType> base_type;
		typedef base_type::character_type character_type;
		typedef base_type::const_pointer const_pointer;
		typedef base_type::pointer pointer;
		typedef base_type::size_type size_type;
		typedef base_type::const_iterator const_iterator;
		typedef base_type::iterator iterator;
		typedef std::basic_string<CharType> string_type;
		// interface
	public:
		void encode(uint8_t aValue)
		{
			encode(static_cast<uint64_t>(aValue), sizeof(uint8_t));
		}
		void encode(uint16_t aValue)
		{
			encode(static_cast<uint64_t>(aValue), sizeof(uint16_t))
		}
		void encode(uint32_t aValue)
		{
			encode(static_cast<uint64_t>(aValue), sizeof(uint32_t))
		}
		void encode(uint64_t aValue)
		{
			encode(static_cast<uint64_t>(aValue), sizeof(uint64_t))
		}
		void encode(int8_t aValue)
		{
			encode(static_cast<uint64_t>(aValue), sizeof(int8_t));
		}
		void encode(int16_t aValue)
		{
			encode(static_cast<uint64_t>(aValue), sizeof(int16_t))
		}
		void encode(int32_t aValue)
		{
			encode(static_cast<uint64_t>(aValue), sizeof(int32_t))
		}
		void encode(int64_t aValue)
		{
			encode(static_cast<uint64_t>(aValue), sizeof(int64_t))
		}
		void encode(int64_t aValue, std::size_t aLength)
		{
			encode(static_cast<uint64_t>(aValue), aLength);
		}
		virtual void encode(uint64_t aValue, std::size_t aLength) = 0;
		virtual void encode(bool aValue) = 0;
		virtual void encode(const string_type& aValue) = 0;
		template <typename T>
		T decode() const
		{
			return detail::decoder<our_type, T>()(*this);
		}
		virtual uint64_t decode_integer(std::size_t aLength) const = 0;
		virtual bool decode_bool() const = 0;
		virtual string_type decode_string() const = 0;
	};

	typedef basic_data_packet<char> data_packet;
}
