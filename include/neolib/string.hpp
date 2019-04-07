// string.hpp - v1.0
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

#include "neolib.hpp"
#include <string>
#include "reference_counted.hpp"
#include "i_string.hpp"
#include "container_iterator.hpp"
#include "quick_string.hpp"

namespace neolib
{
    class string : public reference_counted<i_string>
    {
        // types
    private:
        typedef reference_counted<i_string> base;
    public:
        typedef char value_type;
        typedef std::string container_type;
    protected:
        typedef container::random_access_const_iterator<char, std::string::const_iterator> container_const_iterator;
        typedef container::random_access_iterator<char, std::string::iterator, std::string::const_iterator> container_iterator;
        typedef base::abstract_const_iterator abstract_const_iterator;
        typedef base::abstract_iterator abstract_iterator;
        // construction
    public:
        string(const char* aString) : iString{ aString }, iEndConstIterator{}, iEndIterator{} {}
        string(const std::string& aString = std::string{}) : iString{ aString }, iEndConstIterator{}, iEndIterator{} {}
        string(const neolib::quick_string& aOther) : iString{ aOther }, iEndConstIterator{}, iEndIterator{} {}
        string(const string& aOther) : iString{ aOther.to_std_string() }, iEndConstIterator{}, iEndIterator{} {}
        string(const i_string& aOther) : iString{ aOther.to_std_string() }, iEndConstIterator{}, iEndIterator{} {}
        ~string() {}
        string& operator=(const string& aOther) { assign(aOther); return *this; }
        string& operator=(const i_string& aOther) override { assign(aOther); return *this; }
        // operations
    public:
        const container_type& container() const { return iString; }
        container_type& container() { return iString; }
        // implementation
    public:
        // from i_container
        size_type size() const override { return iString.size(); }
        size_type max_size() const override { return iString.max_size(); }
        void clear() override { reset_cache(); iString.clear(); }
        void assign(const i_container& aOther) override { if (&aOther == this) return; reset_cache(); iString.assign(aOther.begin(), aOther.end()); }
    private:
        // from i_container
        abstract_const_iterator* do_begin() const override { populate_cache(); return new container_const_iterator(iString.begin()); }
        abstract_const_iterator* do_end() const override { populate_cache(); return iEndConstIterator.wrapped_iterator(); }
        abstract_iterator* do_begin() override { populate_cache(); return new container_iterator(iString.begin()); }
        abstract_iterator* do_end() override { populate_cache(); return iEndIterator.wrapped_iterator(); }
        abstract_iterator* do_erase(const abstract_const_iterator& aPosition) override { reset_cache();  return new container_iterator(iString.erase(static_cast<const container_const_iterator&>(aPosition))); }
        abstract_iterator* do_erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) override { reset_cache(); return new container_iterator(iString.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); }
    public:
        // from i_sequence_container
        size_type capacity() const override { return iString.size(); }
        void reserve(size_type aCapacity) override { reset_cache(); iString.reserve(aCapacity); }
        void resize(size_type aSize, const value_type& aValue) override { reset_cache(); iString.resize(aSize, aValue); }
        void push_back(const value_type& aValue) override { reset_cache(); iString.push_back(aValue); }
        void pop_back() override { reset_cache(); iString.erase(iString.end() - 1); }
        const value_type& back() const override { return iString.back(); }
        value_type& back() override { return iString.back(); }
    private:
        // from i_sequence_container
        abstract_iterator* do_insert(const abstract_const_iterator& aPosition, const value_type& aValue) override { reset_cache(); return new container_iterator(iString.insert(static_cast<const container_const_iterator&>(aPosition), aValue)); }
    public:
        // from i_string
        const char* c_str() const override { return iString.c_str(); }
        const char& operator[](size_type aIndex) const override { return iString[aIndex]; }
        char& operator[](size_type aIndex) override { return iString[aIndex]; }
        void assign(const string& aOther) { reset_cache(); iString = aOther.to_std_string(); }
        void assign(const i_string& aOther) override { reset_cache(); iString = aOther.to_std_string(); }
        void assign(const char* aSource, size_type aSourceLength) override { reset_cache(); iString.assign(aSource, aSourceLength); }
    private:
        void reset_cache() { iEndConstIterator = const_iterator(); iEndIterator = iterator(); }
        void populate_cache() const
        {
            if (iEndConstIterator.wrapped_iterator() == nullptr)
                iEndConstIterator = const_iterator(new container_const_iterator(iString.end()));
        }
        void populate_cache()
        {
            if (iEndConstIterator.wrapped_iterator() == nullptr)
                iEndConstIterator = const_iterator(new container_const_iterator(iString.end()));
            if (iEndIterator.wrapped_iterator() == nullptr)
                iEndIterator = iterator(new container_iterator(iString.end()));
        }
        // attributes
    private:
        std::string iString;
        mutable const_iterator iEndConstIterator;
        mutable iterator iEndIterator;
    };

    inline string operator+(const string& lhs, const string& rhs)
    {
        return lhs.container() + rhs.container();
    }

    inline bool operator==(const string& lhs, const string& rhs)
    {
        return lhs.size() == rhs.size() && std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
    }

    inline bool operator==(const string& lhs, const i_string& rhs)
    {
        return lhs.size() == rhs.size() && std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
    }

    inline bool operator==(const i_string& lhs, const string& rhs)
    {
        return lhs.size() == rhs.size() && std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
    }

    inline bool operator!=(const string& lhs, const string& rhs)
    {
        return lhs.size() != rhs.size() || std::strcmp(lhs.c_str(), rhs.c_str()) != 0;
    }

    inline bool operator!=(const string& lhs, const i_string& rhs)
    {
        return lhs.size() != rhs.size() || std::strcmp(lhs.c_str(), rhs.c_str()) != 0;
    }

    inline bool operator!=(const i_string& lhs, const string& rhs)
    {
        return lhs.size() != rhs.size() || std::strcmp(lhs.c_str(), rhs.c_str()) != 0;
    }

    inline bool operator<(const string& lhs, const string& rhs)
    {
        return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
    }

    inline bool operator<(const string& lhs, const i_string& rhs)
    {
        return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
    }

    inline bool operator<(const i_string& lhs, const string& rhs)
    {
        return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
    }
}
