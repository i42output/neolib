// i_container.hpp
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
#include <algorithm>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/i_iterator.hpp>

namespace neolib
{
    template <typename T, typename ConstIteratorType, typename IteratorType>
    class i_container : public i_reference_counted
    {
    protected:
        typedef i_container<T, ConstIteratorType, IteratorType> generic_container_type;
    public:
        typedef T value_type;
        typedef size_t size_type;
        typedef ConstIteratorType abstract_const_iterator;
        typedef IteratorType abstract_iterator;
    public:
        typedef typename abstract_const_iterator::iterator_wrapper const_iterator;
        typedef typename abstract_iterator::iterator_wrapper iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
    public:
        virtual size_type size() const noexcept = 0;
        virtual size_type max_size() const noexcept = 0;
        bool empty() const noexcept { return size() == 0; }
        const_iterator cbegin() const { return do_begin(); }
        const_iterator begin() const { return cbegin(); }
        iterator begin() { return do_begin(); }
        const_iterator cend() const { return do_end(); }
        const_iterator end() const { return cend(); }
        iterator end() { return do_end(); }
        const_reverse_iterator crbegin() const { return const_reverse_iterator{ end() }; }
        const_reverse_iterator rbegin() const { return crbegin(); }
        reverse_iterator rbegin() { return reverse_iterator{ end() }; }
        const_reverse_iterator crend() const { return const_reverse_iterator{ begin() }; }
        const_reverse_iterator rend() const { return crend(); }
        reverse_iterator rend() { return reverse_iterator{ begin() }; }
        iterator erase(const abstract_iterator& aPosition) { return do_erase(const_iterator(aPosition)); }
        iterator erase(const abstract_const_iterator& aPosition) { return do_erase(aPosition); }
        iterator erase(const abstract_iterator& aFirst, const abstract_iterator& aLast) { return do_erase(const_iterator(aFirst), const_iterator(aLast)); }
        iterator erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) { return do_erase(aFirst, aLast); }
        virtual void clear() = 0;
        virtual void assign(const i_container& aRhs) = 0;
    public:
        i_container& operator=(const i_container& aRhs)
        {
            assign(aRhs);
            return *this;
        }
    private:
        virtual abstract_const_iterator* do_begin() const = 0;
        virtual abstract_const_iterator* do_end() const = 0;
        virtual abstract_iterator* do_begin() = 0;
        virtual abstract_iterator* do_end() = 0;
        virtual abstract_iterator* do_erase(const abstract_const_iterator& aPosition) = 0;
        virtual abstract_iterator* do_erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) = 0;
    };

    template <typename T, typename ConstIteratorType, typename IteratorType>
    inline bool operator==(const i_container<T, ConstIteratorType, IteratorType>& lhs, const i_container<T, ConstIteratorType, IteratorType>& rhs)
    {
        return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    template <typename T, typename ConstIteratorType, typename IteratorType>
    inline std::partial_ordering operator<=>(const i_container<T, ConstIteratorType, IteratorType>& lhs, const i_container<T, ConstIteratorType, IteratorType>& rhs)
    {
        return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
}
