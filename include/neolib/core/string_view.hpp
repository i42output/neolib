// string_view.hpp
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

#include <neolib/core/string.hpp>
#include <neolib/core/i_string_view.hpp>

namespace neolib
{
    class string_view : public i_string_view
    {
    public:
        using size_type = std::size_t;
        using const_iterator = char const*;
        using iterator = const_iterator;
    public:
        string_view() :
            iFirst{ nullptr },
            iLast{ nullptr }
        {
        }
        string_view(char const* aOther, size_type aCount) :
            iFirst{ aOther },
            iLast{ std::next(aOther, aCount) }
        {
        }
        string_view(char const* aOther) :
            iFirst{ aOther },
            iLast{ std::next(aOther, std::strlen(aOther)) }
        {
        }
        string_view(string_view const& aOther) :
            iFirst{ aOther.iFirst },
            iLast{ aOther.iLast }
        {
        }
        string_view(i_string_view const& aOther) :
            iFirst{ std::to_address(aOther.begin()) },
            iLast{ std::to_address(aOther.end()) }
        {
        }
        string_view(std::string_view const& aOther) :
            iFirst{ std::to_address(aOther.begin()) },
            iLast{ std::to_address(aOther.end()) }
        {
        }
        string_view(i_string const& aOther) :
            iFirst{ std::to_address(aOther.begin()) },
            iLast{ std::to_address(aOther.end()) }
        {
        }
        string_view(std::string const& aOther) :
            iFirst{ std::to_address(aOther.begin()) },
            iLast{ std::to_address(aOther.end()) }
        {
        }
    public:
        i_string_view& operator=(const i_string_view& aOther) noexcept final
        {
            iFirst = std::to_address(aOther.begin());
            iLast = std::to_address(aOther.end());
            return *this;
        }
    public:
        bool empty() const noexcept
        {
            return iFirst == iLast;
        }
        virtual char const* data() const noexcept
        {
            return iFirst;
        }
        virtual size_type size() const noexcept
        {
            return std::distance(iFirst, iLast);
        }
    public:
        void assign(char const* aFirst, char const* aLast) noexcept
        {
            iFirst = aFirst;
            iLast = aLast;
        }
    private:
        char const* iFirst;
        char const* iLast;
    };
}
