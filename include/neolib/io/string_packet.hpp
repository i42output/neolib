// string_packet.hpp
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
#include <string>
#include <neolib/io/i_packet.hpp>

namespace neolib
{
    template <typename CharType>
    class basic_string_packet : public i_basic_packet<CharType>
    {
        typedef i_basic_packet<CharType> base_type;
        // constants
    public:
        static constexpr CharType CHAR_CR = static_cast<CharType>('\r');
        static constexpr CharType CHAR_LF = static_cast<CharType>('\n');
        // types
    public:
        typedef typename base_type::character_type character_type;
        typedef typename base_type::const_pointer const_pointer;
        typedef typename base_type::pointer pointer;
        typedef typename base_type::size_type size_type;
        typedef typename base_type::clone_pointer clone_pointer;
        typedef std::basic_string<CharType> contents_type;
        // construction
    public:
        basic_string_packet(const contents_type& aContents = contents_type()) : 
            iContents(aContents) 
        {
        }
        basic_string_packet(const character_type* aPointer, size_type aLength) : 
            iContents(aPointer, aLength) 
        {
        }
        basic_string_packet(const basic_string_packet& aOther) :
            iContents(aOther.iContents)
        {
        }
        basic_string_packet& operator=(const basic_string_packet& aOther)
        {
            if (this != &aOther)
                iContents = aOther.iContents;
            return *this;
        }
        // operations
    public:
        // from i_basic_packet
        virtual const_pointer data() const 
        { 
            if (base_type::empty())
                throw typename base_type::packet_empty();
            return &iContents[0]; 
        }
        virtual pointer data()
        { 
            if (base_type::empty())
                throw typename base_type::packet_empty();
            return &iContents[0]; 
        }
        virtual size_type length() const
        {
            return iContents.size();
        }
        virtual bool has_max_length() const
        {
            return false;
        }
        virtual size_type max_length() const
        {
            return iContents.max_size();
        }
        virtual void clear()
        {
            iContents.clear();
        }
        virtual bool take_some(const_pointer& aFirst, const_pointer aLast)
        {
            if (aFirst == aLast)
                return false;

            if (skip_delimiters(aFirst, aLast))
                return true;

            if (aFirst == aLast)
                return false;

            const_pointer const contentStart = aFirst;
            while (aFirst != aLast && !is_delimiter(*aFirst))
                ++aFirst;
            const_pointer const contentEnd = aFirst;

            if (contentStart != contentEnd)
            {
                if (has_max_length() && length() + (contentEnd - contentStart) > max_length())
                    throw typename base_type::packet_too_big();
                iContents.insert(iContents.end(), contentStart, contentEnd);
            }

            bool const terminated = skip_delimiters(aFirst, aLast);

            return terminated || contentEnd != aLast || (contentStart == contentEnd && !iContents.empty());
        }
        virtual clone_pointer clone() const
        {
            return clone_pointer(new basic_string_packet(*this));
        }
        virtual void copy_from(const i_basic_packet<CharType>& aSource)
        {
            iContents.clear();
            if (aSource.length() != 0)
                iContents.assign(aSource.data(), aSource.length());
        }
        // own
        const contents_type& contents() const
        {
            return iContents;
        }
        contents_type& contents()
        {
            return iContents;
        }
        // implementation
    private:
        virtual bool has_delimiters() const
        {
            return true;
        }
        virtual bool is_delimiter(character_type aCharacter) const 
        {
            return has_delimiters() && (aCharacter == CHAR_CR || aCharacter == CHAR_LF);
        }
        virtual bool is_terminating_delimiter(character_type aCharacter) const 
        {
            return has_delimiters() && aCharacter == CHAR_LF;
        }
    private:
        bool skip_delimiters(const_pointer& aFirst, const_pointer aLast) const
        {
            while (aFirst != aLast && is_delimiter(*aFirst))
            {
                bool terminated = is_terminating_delimiter(*aFirst);
                ++aFirst;
                if (terminated)
                    return true;
            }
            return false;
        }
        // attributes
    private:
        contents_type iContents;
    };

    typedef basic_string_packet<char> string_packet;
}
