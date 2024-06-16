// i_string_view.hpp
/*
 *  Copyright (c) 2024 Leigh Johnston.
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
#include <memory>
#include <string>
#include <string_view>
#include <iostream>
#include <boost/algorithm/string.hpp>

namespace neolib
{
    class i_string_view
    {
    public:
        using value_type = char;
        using pointer = value_type*;
        using const_pointer = value_type const*;
        using reference = value_type&;
        using const_reference = value_type const&;
        using const_iterator = char const*;
        using iterator = const_iterator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
    public:
        virtual ~i_string_view() = default;
    public:
        i_string_view& operator=(const i_string_view& aOther) noexcept
        {
            assign(aOther);
            return *this;

        }
        i_string_view& operator=(const std::string_view& aOther) noexcept
        {
            assign(aOther); 
            return *this;
        }
        i_string_view& operator=(const std::string& aOther) noexcept
        {
            assign(aOther); 
            return *this;
        }
    public:
        virtual bool empty() const noexcept = 0;
        virtual size_type size() const noexcept = 0;
        size_type length() const noexcept
        {
            return size();
        }
    public:
        const_reference operator[](size_type pos) const
        {
            return data()[pos];
        }
        const_reference at(size_type pos) const
        {
            if (pos < size())
                return operator[](pos);
            throw std::out_of_range("neolib::i_string_view");
        }
        const_reference front() const
        {
            return operator[](0);
        }
        const_reference back() const
        {
            return operator[](size() - 1);
        }
        virtual const_pointer data() const noexcept = 0;
    public:
        void assign(const_pointer aSource, size_type aSourceLength) noexcept
        {
            assign(aSource, std::next(aSource, aSourceLength));
        }
        void assign(i_string_view const& aSource) noexcept
        {
            assign(std::to_address(aSource.begin()), std::to_address(aSource.end()));
        }
        void assign(std::string_view const& aSource) noexcept
        {
            assign(std::to_address(aSource.begin()), std::to_address(aSource.end()));
        }
        void assign(std::string const& aSource) noexcept
        {
            assign(std::string_view{ aSource });
        }
        virtual void assign(const_pointer aFirst, const_pointer aLast) noexcept = 0;
    public:
        const_iterator cbegin() const noexcept
        {
            return data();
        }
        const_iterator begin() const noexcept
        {
            return data();
        }
        const_iterator cend() const noexcept
        {
            return std::next(data(), size());
        }
        const_iterator end() const noexcept
        {
            return std::next(data(), size());
        }
    public:
        std::string to_std_string() const 
        { 
            return std::string{ data(), size() }; 
        }
        std::string_view to_std_string_view() const noexcept
        { 
            return std::string_view{ data(), size() }; 
        }
    };

    inline bool operator==(const i_string_view& lhs, const i_string_view& rhs) noexcept
    {
        return lhs.to_std_string_view() == rhs.to_std_string_view();
    }

    inline std::strong_ordering operator<=>(const i_string_view& lhs, const i_string_view& rhs) noexcept
    {
        return lhs.to_std_string_view() <=> rhs.to_std_string_view();
    }

    inline bool operator==(const i_string_view& lhs, const std::string& rhs) noexcept
    {
        return lhs.to_std_string_view() == std::string_view{ rhs };
    }

    inline bool operator==(const std::string& lhs, const i_string_view& rhs) noexcept
    {
        return std::string_view{ lhs } == rhs.to_std_string_view();
    }

    struct ci_sv_equal_to
    {
        bool operator()(const i_string_view& lhs, const i_string_view& rhs) const
        {
            return boost::iequals(lhs.to_std_string_view(), rhs.to_std_string_view());
        }
    };

    struct ci_sv_less
    {
        bool operator()(const i_string_view& lhs, const i_string_view& rhs) const
        {
            return boost::ilexicographical_compare(lhs.to_std_string_view(), rhs.to_std_string_view());
        }
    };

    inline std::ostream& operator<<(std::ostream& aStream, const i_string_view& aString)
    {
        aStream << aString.to_std_string_view();
        return aStream;
    }

    inline std::istream& operator>>(std::istream& aStream, i_string_view& aString)
    {
        std::string temp;
        aStream >> temp;
        aString.assign(temp.data(), temp.size());
        return aStream;
    }
}
