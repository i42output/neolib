// map.hpp - v1.0
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
#include <map>
#include "pair.hpp"
#include "container_iterator.hpp"
#include "i_map.hpp"

namespace neolib
{
    template <typename Key, typename T, typename ConcreteKey = Key, typename ConcreteType = T>
    class map : public reference_counted <i_map<Key, T>>
    {
        // types
    public:
        typedef Key abstract_key_type;
        typedef T abstract_mapped_type;
        typedef ConcreteKey concrete_key_type;
        typedef ConcreteType concrete_mapped_type;
        typedef i_pair<const abstract_key_type, abstract_mapped_type> abstract_value_type;
        typedef std::map<concrete_key_type, neolib::pair<const abstract_key_type, abstract_mapped_type, const concrete_key_type, concrete_mapped_type> > container_type;
    private:
        typedef i_map<Key, T> abstract_base;
    public:
        typedef typename abstract_base::size_type size_type;
        typedef typename abstract_base::const_iterator const_iterator;
        typedef typename abstract_base::iterator iterator;
        typedef typename abstract_base::generic_container_type generic_container_type;
    protected:
        typedef container::const_iterator<abstract_value_type, typename container_type::const_iterator> container_const_iterator;
        typedef container::iterator<abstract_value_type, typename container_type::iterator, typename container_type::const_iterator> container_iterator;
        typedef typename abstract_base::abstract_const_iterator abstract_const_iterator;
        typedef typename abstract_base::abstract_iterator abstract_iterator;
        // construction
    public:
        map() :
            iEndConstIterator(new container_const_iterator(iMap.cend())),
            iEndIterator(new container_iterator(iMap.end()))
        {}
        map(const generic_container_type& aOther) :
            iEndConstIterator(new container_const_iterator(iMap.cend())),
            iEndIterator(new container_iterator(iMap.end()))
        {
            assign(aOther);
        }
        template <typename InputIter>
        map(InputIter aFirst, InputIter aLast) :
            iMap(aFirst, aLast),
            iEndConstIterator(new container_const_iterator(iMap.cend())),
            iEndIterator(new container_iterator(iMap.end()))
        {}
        // implementation
        // from i_container
    public:
        size_type size() const override { return iMap.size(); }
        size_type max_size() const override { return iMap.max_size(); }
        void clear() override { iMap.clear(); }
        void assign(const generic_container_type& aOther) override
        {
            if (&aOther == this) 
                return;
            clear();
            for (const_iterator i = aOther.begin(); i != aOther.end(); ++i)
                iMap.insert(
                    typename container_type::value_type(
                        typename container_type::key_type(i->first()), 
                        typename container_type::mapped_type(
                            concrete_key_type(i->first()),
                            concrete_mapped_type(i->second()))));
        }
        // from i_container
    private:
        container_const_iterator* do_begin() const override { return new container_const_iterator(iMap.begin()); }
        container_const_iterator* do_end() const override { return static_cast<container_const_iterator*>(iEndConstIterator.wrapped_iterator()); }
        container_iterator* do_begin() override { return new container_iterator(iMap.begin()); }
        container_iterator* do_end() override { return static_cast<container_iterator*>(iEndIterator.wrapped_iterator()); }
        container_iterator* do_erase(const abstract_const_iterator& aPosition) override { return new container_iterator(iMap.erase(static_cast<const container_const_iterator&>(aPosition))); }
        container_iterator* do_erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) override { return new container_iterator(iMap.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); }
        // from i_map
    public:
        concrete_mapped_type& operator[](const abstract_key_type& aKey) override 
        { 
            auto existing = iMap.find(aKey);
            if (existing == iMap.end())
            {
                existing = iMap.insert(
                    typename container_type::value_type(
                        typename container_type::key_type(aKey),
                        typename container_type::mapped_type(
                            concrete_key_type(aKey),
                            concrete_mapped_type{}))).first;
            }
            return existing->second.second(); 
        }
    private:
        container_const_iterator* do_find(const abstract_key_type& aKey) const override { return new container_const_iterator(iMap.find(aKey)); }
        container_iterator* do_find(const abstract_key_type& aKey) override { return new container_iterator(iMap.find(aKey)); }
    private:
        container_type iMap;
        const_iterator iEndConstIterator;
        iterator iEndIterator;
    };
}
