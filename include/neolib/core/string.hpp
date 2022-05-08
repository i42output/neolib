// string.hpp
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
#include <boost/algorithm/string/replace.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/i_string.hpp>
#include <neolib/core/container_iterator.hpp>
#include <neolib/core/quick_string.hpp>

using namespace std::literals::string_literals;

namespace neolib
{
    class string : public reference_counted<i_string>
    {
        typedef reference_counted<i_string> base_type;
        // types
    public:
        typedef i_string abstract_type;
        typedef char value_type;
        typedef std::string std_type;
    protected:
        typedef container::random_access_const_iterator<char, std::string::const_iterator> container_const_iterator;
        typedef container::random_access_iterator<char, std::string::iterator, std::string::const_iterator> container_iterator;
        typedef typename container_const_iterator::abstract_const_iterator abstract_const_iterator;
        typedef typename container_iterator::abstract_iterator abstract_iterator;
        // construction
    public:
        string(size_type aCount, value_type aChar) : iString(aCount, aChar) {}
        string(const char* aString) : iString{ aString } {}
        string(const char* aString, std::size_t aLength) : iString{ aString, aLength } {}
        string(const std::string& aString = {}) : iString{ aString } {}
        string(const std::string_view& aStringView) : iString{ aStringView } {}
        string(const neolib::quick_string& aOther) : iString{ aOther } {}
        string(const string& aOther) : iString{ aOther.to_std_string() } {}
        string(string&& aOther) : iString{ std::move(aOther.to_std_string()) } {}
        string(const i_string& aOther) : iString{ aOther.to_std_string() } {}
        template <typename Iter>
        string(Iter aBegin, Iter aEnd) : iString{ aBegin, aEnd } {}
        ~string() {}
        string& operator=(const string& aOther) { assign(aOther); return *this; }
        string& operator=(string&& aOther) { assign(std::move(aOther)); return *this; }
        string& operator=(const i_string& aOther) override { assign(aOther); return *this; }
        // operations
    public:
        const std_type& to_std_string() const { return iString; }
        std_type& to_std_string() { return iString; }
        // implementation
        // from i_container
    public:
        size_type size() const override { return iString.size(); }
        size_type max_size() const override { return iString.max_size(); }
        void clear() override { iString.clear(); }
        void assign(const i_container& aOther) override { if (&aOther == this) return; iString.assign(aOther.begin(), aOther.end()); }
        // from i_container
    private:
        abstract_const_iterator* do_begin(void* memory) const override { return new (memory) container_const_iterator{ iString.begin() }; }
        abstract_const_iterator* do_end(void* memory) const override { return new (memory) container_const_iterator{ iString.end() }; }
        abstract_iterator* do_begin(void* memory) override { return new (memory) container_iterator{ iString.begin() }; }
        abstract_iterator* do_end(void* memory) override { return new (memory) container_iterator{ iString.end() }; }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aPosition) override { return new (memory) container_iterator{ iString.erase(static_cast<const container_const_iterator&>(aPosition)) }; }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) override { return new (memory) container_iterator{ iString.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast)) }; }
        // from i_sequence_container
    public:
        size_type capacity() const override { return iString.size(); }
        void reserve(size_type aCapacity) override { iString.reserve(aCapacity); }
        void resize(size_type aSize) override { iString.resize(aSize); }
        void resize(size_type aSize, const value_type& aValue) override { iString.resize(aSize, aValue); }
        void push_back(const value_type& aValue) override { iString.push_back(aValue); }
        void pop_back() override { iString.erase(iString.end() - 1); }
        const value_type& front() const override { return iString.front(); }
        value_type& front() override { return iString.front(); }
        const value_type& back() const override { return iString.back(); }
        value_type& back() override { return iString.back(); }
    private:
        abstract_iterator* do_insert(void* memory, const abstract_const_iterator& aPosition, const value_type& aValue) override { return new (memory) container_iterator{ iString.insert(static_cast<const container_const_iterator&>(aPosition), aValue) }; }
        // from i_random_access_container
    public:
        const value_type& at(size_type aIndex) const override { return iString.at(aIndex); }
        value_type& at(size_type aIndex) override { return iString.at(aIndex); }
        const value_type& operator[](size_type aIndex) const override { return iString[aIndex]; }
        value_type& operator[](size_type aIndex) override { return iString[aIndex]; }
    private:
        std::ptrdiff_t iterator_offset() const override { return sizeof(char); }
        // from i_string
    public:
        const char* cdata() const override { return iString.data(); }
        const char* data() const override { return iString.data(); }
        char* data() override { return iString.data(); }
        const char* c_str() const override { return iString.c_str(); }
        void assign(const string& aOther) { iString = aOther.to_std_string_view(); }
        void assign(const i_string& aOther) override { iString = aOther.to_std_string_view(); }
        void assign(const char* aSource, size_type aSourceLength) override { iString.assign(aSource, aSourceLength); }
        void append(const string& aOther) { iString.append(aOther.to_std_string_view()); }
        void append(const i_string& aOther) override { iString.append(aOther.to_std_string_view()); }
        void append(const char* aSource, size_type aSourceLength) override { iString.append(aSource, aSourceLength); }
    public:
        void replace_all(const i_string& aSearch, const i_string& aReplace) override { boost::replace_all(iString, aSearch.to_std_string_view(), aReplace.to_std_string_view()); }
    public:
        using i_string::assign;
        void assign(string&& aOther) { if (&aOther == this) return; iString.assign(std::move(aOther.to_std_string())); }
    public:
        std::strong_ordering operator<=>(const string& that) const
        {
            return iString <=> that.iString;
        }
        // attributes
    private:
        std::string iString;
    };

    inline string operator+(const string& lhs, const string& rhs)
    {
        return lhs.to_std_string() + rhs.to_std_string();
    }

    inline string& operator+=(string& lhs, const i_string& rhs)
    {
        lhs.to_std_string() += rhs.to_std_string_view();
        return lhs;
    }

    inline bool operator==(const string& lhs, const i_string& rhs)
    {
        return lhs.size() == rhs.size() && std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
    }

    inline bool operator==(const i_string& lhs, const string& rhs)
    {
        return lhs.size() == rhs.size() && std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
    }

    inline bool operator!=(const string& lhs, const i_string& rhs)
    {
        return !(lhs == rhs);
    }

    inline bool operator!=(const i_string& lhs, const string& rhs)
    {
        return !(lhs == rhs);
    }

    inline bool operator<(const string& lhs, const i_string& rhs)
    {
        return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
    }

    inline bool operator<(const i_string& lhs, const string& rhs)
    {
        return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
    }

    inline bool operator>(const string& lhs, const i_string& rhs)
    {
        return std::strcmp(lhs.c_str(), rhs.c_str()) > 0;
    }

    inline bool operator>(const i_string& lhs, const string& rhs)
    {
        return std::strcmp(lhs.c_str(), rhs.c_str()) > 0;
    }

    inline bool operator>=(const string& lhs, const i_string& rhs)
    {
        return !(lhs < rhs);
    }

    inline bool operator>=(const i_string& lhs, const string& rhs)
    {
        return !(lhs < rhs);
    }

    inline bool operator<=(const string& lhs, const i_string& rhs)
    {
        return !(lhs > rhs);
    }

    inline bool operator<=(const i_string& lhs, const string& rhs)
    {
        return !(lhs > rhs);
    }

    namespace string_literals
    {
        inline string operator"" _s(const char* str, size_t len)
        {
            return string{ str, len };
        }
    }
}
