// i_string.hpp - v1.0
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
#include <string_view>
#include <iostream>
#include "fwd_abstract.hpp"
#include "i_sequence_container.hpp"

namespace neolib
{
    class i_string : public i_sequence_container<char, i_random_access_const_iterator<char>, i_random_access_iterator<char>, false>
    {
        typedef i_string self_type;
        typedef i_sequence_container<char, i_random_access_const_iterator<char>, i_random_access_iterator<char> > base_type;
    public:
        typedef self_type abstract_type;
        typedef base_type::size_type size_type;
        typedef const char* const_fast_iterator;
        typedef char* fast_iterator;
    public:
        virtual i_string& operator=(const i_string& aOther) = 0;
    public:
        virtual const char* cdata() const = 0;
        virtual const char* data() const = 0;
        virtual char* data() = 0;
        virtual const char* c_str() const = 0;
        virtual const char& operator[](size_type aIndex) const = 0;
        virtual char& operator[](size_type aIndex) = 0;
        virtual void assign(const i_string& aOther) = 0;
        virtual void assign(const char* aSource, size_type aSourceLength) = 0;
        virtual void append(const i_string& aOther) = 0;
        virtual void append(const char* aSource, size_type aSourceLength) = 0;
    public:
        virtual void replace_all(const i_string& aSearch, const i_string& aReplace) = 0;
    public:
        operator std::string() const { return to_std_string(); }
        i_string& operator=(const std::string& aOther) { assign(aOther); return *this; }
        size_type length() const { return size(); }
        void assign(const std::string& aSource) { assign(aSource.c_str(), aSource.size()); }
        void append(const std::string& aSource) { append(aSource.c_str(), aSource.size()); }
        std::string to_std_string() const { return std::string(c_str(), size()); }
        std::string_view to_std_string_view() const { return std::string_view(c_str(), size()); }
    public:
        const_fast_iterator cfbegin() const { return cdata(); }
        const_fast_iterator cfend() const { return cdata() + size(); }
        const_fast_iterator fbegin() const { return data(); }
        const_fast_iterator fend() const { return data() + size(); }
        fast_iterator fbegin() { return data(); }
        fast_iterator fend() { return data() + size(); }
    };

    inline i_string& operator+=(i_string& lhs, const i_string& rhs)
    {
        lhs.append(rhs);
        return lhs;
    }

    inline bool operator==(const i_string& lhs, const i_string& rhs)
    {
        return lhs.size() == rhs.size() && std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
    }

    inline bool operator!=(const i_string& lhs, const i_string& rhs)
    {
        return lhs.size() != rhs.size() || std::strcmp(lhs.c_str(), rhs.c_str()) != 0;
    }

    inline bool operator<(const i_string& lhs, const i_string& rhs)
    {
        return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
    }

    inline std::ostream& operator<<(std::ostream& aStream, const i_string& aString)
    {
        aStream << aString.to_std_string();
        return aStream;
    }

    inline std::istream& operator>>(std::istream& aStream, i_string& aString)
    {
        std::string temp;
        aStream >> temp;
        aString.assign(temp.c_str(), temp.size());
        return aStream;
}
    }
