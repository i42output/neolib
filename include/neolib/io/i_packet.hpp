// i_packet.hpp
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
#include <stdexcept>
#include <memory>

namespace neolib
{
    template <typename CharType>
    class i_basic_packet
    {
        // types
    public:
        using character_type = CharType;
        using const_pointer = const character_type*;
        using pointer = character_type*;
        using size_type = std::size_t;
        using const_iterator = const_pointer;
        using iterator = pointer;
        using clone_pointer = std::unique_ptr<i_basic_packet>;
        // exceptions
    public:
        struct packet_empty : std::logic_error { packet_empty() : std::logic_error("i_basic_packet::packet_empty") {} };
        struct packet_too_big : std::runtime_error { packet_too_big() : std::runtime_error("i_basic_packet::packet_too_big") {} };
        // construction
    public:
        virtual ~i_basic_packet() = default;
        // interface
    public:
        virtual const_pointer data() const = 0;
        virtual pointer data() = 0;
        virtual size_type length() const = 0;
        virtual bool has_max_length() const = 0;
        virtual size_type max_length() const = 0;
        bool empty() const { return length() == 0; }
        virtual void clear() = 0;
        const_iterator begin() const { return !empty() ? data() : 0; }
        const_iterator end() const { return !empty() ? data()  + length(): 0; }
        iterator begin() { return !empty() ? data() : 0; }
        iterator end() { return !empty() ? data()  + length(): 0; }
        virtual bool take_some(const_pointer& aFirst, const_pointer aLast) = 0;
        virtual clone_pointer clone() const = 0;
        virtual void copy_from(const i_basic_packet<CharType>& aSource) = 0;
    };

    using i_packet = i_basic_packet<char>;
}
