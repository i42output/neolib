// tld_packet.hpp
/*
 *  Copyright (c) 2007-present, Leigh Johnston.
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
#include <vector>

#include "binary_data_packet.hpp"

namespace neolib
{
	template <typename IdType, std::size_t MaxPacketSize = 1024, typename CharType = char, typename PacketTraits = DefaultPacketTraits>
	class basic_tld_packet : basic_binary_data_packet<CharType, PacketTraits>
	{
		// types
	public:
		typedef tld_packet<IdType, CharType, PacketTraits> our_type;
		typedef basic_binary_data_packet<CharType> base_type;
		typedef base_type::character_type character_type;
		typedef base_type::const_pointer const_pointer;
		typedef base_type::pointer pointer;
		typedef base_type::size_type size_type;
		typedef base_type::const_iterator const_iterator;
		typedef base_type::iterator iterator;
		typedef base_type::string_type string_type;
	private:
		typedef std::vector<CharType> contents_type;
		typedef typename contents_type::size_type contents_offset_type;
		// construction
		basic_tld_packet()
		{
			allocate(0);
		}
		// interface
	public:
		virtual const_pointer data() const
		{
			return &iContents[0];
		}
		virtual pointer data()
		{
			return &iContents[0];
		}
		virtual size_type length() const
		{
			return iContents.size();
		}
		virtual bool has_max_length() const
		{
			return MaxPacketSize != 0;
		}
		virtual size_type max_length() const
		{
			return MaxPacketSize;
		}
		virtual void clear()
		{
			iContents.clear();
			allocate(0);
			iWritePosition = iContents.size();
			iReadPosition = PacketTraits::HeaderSize;
		}
		virtual bool take_some(const_pointer& aFirst, const_pointer aLast)
		{
		}
		virtual clone_pointer clone() const
		{
			return clone_pointer(new our_type(*this));
		}
		virtual void copy_from(const basic_packet<CharType>& aSource)
		{
			iContents = aSource.iContents;
			allocate(0);
			iWritePosition = iContents.size();
			iReadPosition = PacketTraits::HeaderSize;
		}
		void encode(IdType Id)
		{
			contents_offset_type oldPosition = iWritePosition;
			iWritePosition = PacketTraits::IdOffset;
			encode(static_cast<uint32_t>(Id));
			iWritePosition = oldPosition;
		}
		IdType id() const
		{
			contents_offset_type oldPosition = iReadPosition;
			iReadPosition = PacketTraits::IdOffset;
			IdType result = static_cast<IdType>(decode<uint32_t>());
			iReadPosition = oldPosition;
			return result;
		}
		// implementation
	private:
		virtual void write(const void* aData, std::size_t aLength)
		{
		}
		virtual void read(void* aData, std::size_t aLength) const
		{
		}
		void allocate(std::size_t aLength)
		{
			iContents.resize(std::max(iContents.size() + aLength, PacketTraits::HeaderSize));
		}
		// attributes
	private:
		contents_type iContents;
		contents_offset_type iWritePosition;
		mutable contents_offset_type iReadPosition;
	};

	typedef basic_data_packet<char> data_packet;
}
