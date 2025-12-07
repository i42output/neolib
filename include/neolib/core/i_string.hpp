// i_string.hpp
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
#include <boost/algorithm/string.hpp>
#include <neolib/core/fwd_abstract.hpp>
#include <neolib/core/i_contiguous_random_access_container.hpp>
#include <neolib/core/i_string_view.hpp>

namespace neolib
{
    class i_string : public i_contiguous_random_access_container<char>
    {
        typedef i_contiguous_random_access_container<char> base_type;
    public:
        typedef i_string abstract_type;
    public:
        using typename base_type::value_type;
        using typename base_type::size_type;
    public:
        static constexpr auto npos{ static_cast<size_type>(-1) };
    public:
        virtual i_string& operator=(const i_string& aOther) = 0;
        virtual i_string& operator=(const i_string_view& aOther) = 0;
    public:
        virtual const value_type* c_str() const noexcept = 0;
        virtual void assign(const i_string& aOther) = 0;
        virtual void assign(const i_string_view& aOther) = 0;
        virtual void assign(const value_type* aSource, size_type aSourceLength) = 0;
        virtual void append(const i_string& aOther) = 0;
        virtual void append(const i_string_view& aOther) = 0;
        virtual void append(const value_type* aSource, size_type aSourceLength) = 0;
    public:
        virtual void replace_all(const i_string& aSearch, const i_string& aReplace) = 0;
    public:
        explicit operator std::string() const { return to_std_string(); }
        operator std::string_view() const { return to_std_string_view(); }
        i_string& operator=(const char* aOther) { assign(aOther); return *this; }
        i_string& operator=(const std::string& aOther) { assign(aOther); return *this; }
        i_string& operator=(const std::string_view& aOther) { assign(aOther); return *this; }
        size_type length() const { return size(); }
        void assign(const char* aSource) { assign(std::string_view{ aSource }); }
        void assign(const std::string& aSource) { assign(aSource.c_str( ), aSource.size()); }
        void assign(const std::string_view& aSource) { assign(aSource.data(), aSource.size()); }
        void append(const char* aSource) { append(std::string_view{ aSource }); }
        void append(const std::string& aSource) { append(aSource.c_str(), aSource.size()); }
        void append(const std::string_view& aSource) { append(aSource.data(), aSource.size()); }
        std::string to_std_string() const { return std::string{ c_str(), size() }; }
        std::string_view to_std_string_view() const noexcept { return std::string_view{ c_str(), size() }; }
    };

    inline bool operator==(const i_string& lhs, const i_string& rhs) noexcept
    {
        return lhs.to_std_string_view() == rhs.to_std_string_view();
    }

    inline std::strong_ordering operator<=>(const i_string& lhs, const i_string& rhs) noexcept
    {
        return lhs.to_std_string_view() <=> rhs.to_std_string_view();
    }

    inline i_string& operator+=(i_string& lhs, const i_string& rhs)
    {
        lhs.append(rhs);
        return lhs;
    }

    struct ci_equal_to
    {
        bool operator()(const i_string& lhs, const i_string& rhs) const
        {
            return boost::iequals(lhs.to_std_string_view(), rhs.to_std_string_view());
        }
    };

    struct ci_less
    {
        bool operator()(const i_string& lhs, const i_string& rhs) const
        {
            return boost::ilexicographical_compare(lhs.to_std_string_view(), rhs.to_std_string_view());
        }
    };

    inline std::ostream& operator<<(std::ostream& aStream, const i_string& aString)
    {
        aStream << aString.to_std_string_view();
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

namespace std
{
    template <> struct hash<neolib::i_string>
    {
        typedef neolib::i_string argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& aString) const
        {
            return hash<std::string>()(aString.to_std_string());
        }
    };
}
