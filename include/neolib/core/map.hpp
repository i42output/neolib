// map.hpp
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
#include <map>
#include <neolib/core/pair.hpp>
#include <neolib/core/container_iterator.hpp>
#include <neolib/core/i_map.hpp>

namespace neolib
{
    template <typename Key, typename T, typename Pr = std::less<Key>, typename Alloc = std::allocator<std::pair<const Key, T>>>
    class map : public reference_counted<i_map<abstract_t<Key>, abstract_t<T>>>
    {
        typedef map<Key, T, Pr, Alloc> self_type;
        typedef reference_counted<i_map<abstract_t<Key>, abstract_t<T>>> base_type;
        // types
    public:
        typedef i_map<abstract_t<Key>, abstract_t<T>> abstract_type;
        typedef Key key_type;
        typedef T mapped_type;
        typedef pair<const key_type, mapped_type> value_type;
        typedef abstract_t<key_type> abstract_key_type;
        typedef abstract_t<mapped_type> abstract_mapped_type;
        typedef i_pair<const abstract_key_type, abstract_mapped_type> abstract_value_type;
        typedef Pr key_compare;
        typedef Alloc allocator_type;
        typedef std::map<key_type, value_type, key_compare, typename std::allocator_traits<allocator_type>::template rebind_alloc<std::pair<const key_type, value_type>>> std_type;
        typedef typename std_type::value_type container_value_type;
    public:
        using typename abstract_type::size_type;
        using typename abstract_type::const_iterator;
        using typename abstract_type::iterator;
        using typename abstract_type::generic_container_type;
    protected:
        typedef container::const_iterator<value_type, typename std_type::const_iterator> container_const_iterator;
        typedef container::iterator<value_type, typename std_type::iterator, typename std_type::const_iterator> container_iterator;
        typedef typename abstract_type::abstract_const_iterator abstract_const_iterator;
        typedef typename abstract_type::abstract_iterator abstract_iterator;
        // construction
    public:
        map() 
        {
        }
        map(const map& aOther) :
            iMap{ aOther.iMap }
        {
        }
        map(map&& aOther) :
            iMap{ std::move(aOther.iMap) }
        {
        }
        map(const std::initializer_list<value_type>& aIlist)
        {
            for (auto& value : aIlist)
                iMap.emplace(value.first(), value);
        }
        map(const generic_container_type& aOther)
        {
            assign(aOther);
        }
        template <typename InputIter>
        map(InputIter aFirst, InputIter aLast) :
            iMap(aFirst, aLast)
        {}
        // assignment
    public:
        map& operator=(const map& aOther)
        {
            iMap = aOther.iMap;
            return *this;
        }
        map& operator=(map&& aOther)
        {
            iMap = std::move(aOther.iMap);
            return *this;
        }
        // non-asbtract operations
    public:
        const std_type& as_std_map() const { return iMap; }
        std_type& as_std_map() { return iMap; }
        std_type to_std_map() const { return iMap; }
        // comparison
    public:
        constexpr bool operator==(const self_type& that) const noexcept
        {
            return as_std_map() == that.as_std_map();
        }
        constexpr std::partial_ordering operator<=>(const self_type& that) const noexcept
        {
            return as_std_map() <=> that.as_std_map();
        }
        // implementation
        // from i_container
    public:
        size_type size() const noexcept final { return iMap.size(); }
        size_type max_size() const noexcept final { return iMap.max_size(); }
        void clear() final { iMap.clear(); }
        void assign(const generic_container_type& aOther) final
        {
            if (&aOther == this) 
                return;
            clear();
            for (const_iterator i = aOther.begin(); i != aOther.end(); ++i)
                iMap.insert(container_value_type{ key_type{ i->first() }, value_type{ key_type{i->first()}, mapped_type{i->second()} } });
        }
        // from i_map
    public:
        using abstract_type::insert;
        using abstract_type::find;
        // from i_container
    private:
        abstract_const_iterator* do_begin() const final { return new container_const_iterator{ iMap.begin() }; }
        abstract_const_iterator* do_end() const final { return new container_const_iterator{ iMap.end() }; }
        abstract_iterator* do_begin() final { return new container_iterator{ iMap.begin() }; }
        abstract_iterator* do_end() final { return new container_iterator{ iMap.end() }; }
        abstract_iterator* do_erase(const abstract_const_iterator& aPosition) final { return new container_iterator{ iMap.erase(static_cast<const container_const_iterator&>(aPosition)) }; }
        abstract_iterator* do_erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) final { return new container_iterator{ iMap.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast)) }; }
        // from i_map
    public:
        abstract_mapped_type& operator[](const abstract_key_type& aKey) final
        { 
            auto existing = iMap.find(aKey);
            if (existing == iMap.end())
            {
                existing = iMap.insert(
                    typename std_type::value_type{
                        typename std_type::key_type{aKey},
                        typename std_type::mapped_type{
                            key_type{aKey},
                            mapped_type{}} }).first;
            }
            return existing->second.second(); 
        }
        const abstract_mapped_type& at(const abstract_key_type& aKey) const final
        {
            return iMap.at(aKey).second();
        }
        abstract_mapped_type& at(const abstract_key_type& aKey) final
        {
            return iMap.at(aKey).second();
        }
        // own
    public:
        template <typename Key2, typename... Args>
        value_type& emplace(Key2&& aKey, Args&&... aArgs)
        {
            auto result = iMap.emplace(std::forward<Key2>(aKey), 
                typename std_type::mapped_type{ aKey, mapped_type{ std::forward<Args>(aArgs)... } });
            return result.first->second;
        }
    private:
        abstract_iterator* do_insert(const abstract_key_type& aKey, const abstract_mapped_type& aMapped) final
        { 
            return new container_iterator{ iMap.insert(
                typename std_type::value_type{
                    typename std_type::key_type{aKey},
                    typename std_type::mapped_type{
                        key_type{aKey},
                        mapped_type{aMapped}} }).first };
        }
        abstract_const_iterator* do_find(const abstract_key_type& aKey) const final { return new container_const_iterator{ iMap.find(aKey) }; }
        abstract_iterator* do_find(const abstract_key_type& aKey) final { return new container_iterator{ iMap.find(aKey) }; }
        abstract_const_iterator* do_lower_bound(const abstract_key_type& aKey) const final { return new container_const_iterator{ iMap.lower_bound(aKey) }; }
        abstract_iterator* do_lower_bound(const abstract_key_type& aKey) final { return new container_iterator{ iMap.lower_bound(aKey) }; }
        abstract_const_iterator* do_upper_bound(const abstract_key_type& aKey) const final { return new container_const_iterator{ iMap.upper_bound(aKey) }; }
        abstract_iterator* do_upper_bound(const abstract_key_type& aKey) final { return new container_iterator{ iMap.upper_bound(aKey) }; }
    private:
        std_type iMap;
    };

    template <typename Key, typename T, typename Pr = std::less<Key>, typename Alloc = std::allocator<std::pair<const Key, T>>>
    class multimap : public reference_counted<i_multimap<abstract_t<Key>, abstract_t<T>>>
    {
        typedef multimap<Key, T, Pr, Alloc> self_type;
        typedef reference_counted<i_multimap<abstract_t<Key>, abstract_t<T>>> base_type;
        // types
    public:
        typedef i_multimap<abstract_t<Key>, abstract_t<T>> abstract_type;
        typedef Key key_type;
        typedef T mapped_type;
        typedef pair<const key_type, mapped_type> value_type;
        typedef abstract_t<key_type> abstract_key_type;
        typedef abstract_t<mapped_type> abstract_mapped_type;
        typedef i_pair<const abstract_key_type, abstract_mapped_type> abstract_value_type;
        typedef Pr key_compare;
        typedef Alloc allocator_type;
        typedef std::multimap<key_type, value_type, key_compare, typename std::allocator_traits<allocator_type>::template rebind_alloc<std::pair<const key_type, value_type>>> std_type;
        typedef typename std_type::value_type container_value_type;
    public:
        using typename abstract_type::size_type;
        using typename abstract_type::const_iterator;
        using typename abstract_type::iterator;
        using typename abstract_type::generic_container_type;
    protected:
        typedef container::const_iterator<value_type, typename std_type::const_iterator> container_const_iterator;
        typedef container::iterator<value_type, typename std_type::iterator, typename std_type::const_iterator> container_iterator;
        typedef typename abstract_type::abstract_const_iterator abstract_const_iterator;
        typedef typename abstract_type::abstract_iterator abstract_iterator;
        // construction
    public:
        multimap()
        {
        }
        multimap(const multimap& aOther) :
            iMap{ aOther.iMap }
        {
        }
        multimap(multimap&& aOther) :
            iMap{ std::move(aOther.iMap) }
        {
        }
        multimap(const std::initializer_list<value_type>& aIlist)
        {
            for (auto& value : aIlist)
                iMap.emplace(value.first(), value);
        }
        multimap(const generic_container_type& aOther)
        {
            assign(aOther);
        }
        template <typename InputIter>
        multimap(InputIter aFirst, InputIter aLast) :
            iMap(aFirst, aLast)
        {}
        // assignment
    public:
        multimap& operator=(const multimap& aOther)
        {
            iMap = aOther.iMap;
            return *this;
        }
        multimap& operator=(multimap&& aOther)
        {
            iMap = std::move(aOther.iMap);
            return *this;
        }
        // non-asbtract operations
    public:
        const std_type& as_std_multimap() const { return iMap; }
        std_type& as_std_multimap() { return iMap; }
        std_type to_std_multimap() const { return iMap; }
        // comparison
    public:
        constexpr bool operator==(const self_type& that) const noexcept
        {
            return as_std_multimap() == that.as_std_multimap();
        }
        constexpr std::partial_ordering operator<=>(const self_type& that) const noexcept
        {
            return as_std_multimap() <=> that.as_std_multimap();
        }
        // implementation
        // from i_container
    public:
        size_type size() const noexcept final { return iMap.size(); }
        size_type max_size() const noexcept final { return iMap.max_size(); }
        void clear() final { iMap.clear(); }
        void assign(const generic_container_type& aOther) final
        {
            if (&aOther == this)
                return;
            clear();
            for (const_iterator i = aOther.begin(); i != aOther.end(); ++i)
                iMap.insert(container_value_type{ key_type{ i->first() }, value_type{ key_type{i->first()}, mapped_type{i->second()} } });
        }
        // from i_multimap
    public:
        using abstract_type::insert;
        using abstract_type::find;
        // from i_container
    private:
        abstract_const_iterator* do_begin() const final { return new container_const_iterator{ iMap.begin() }; }
        abstract_const_iterator* do_end() const final { return new container_const_iterator{ iMap.end() }; }
        abstract_iterator* do_begin() final { return new container_iterator{ iMap.begin() }; }
        abstract_iterator* do_end() final { return new container_iterator{ iMap.end() }; }
        abstract_iterator* do_erase(const abstract_const_iterator& aPosition) final { return new container_iterator(iMap.erase(static_cast<const container_const_iterator&>(aPosition))); }
        abstract_iterator* do_erase(const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) final { return new container_iterator(iMap.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast))); }
        // own
    public:
        template <typename Key2, typename... Args>
        value_type& emplace(Key2&& aKey, Args&&... aArgs)
        {
            return iMap.emplace(std::forward<Key2>(aKey), typename std_type::mapped_type{ aKey, mapped_type{ std::forward<Args>(aArgs)... } })->second;
        }
        // from i_multimap
    private:
        abstract_iterator* do_insert(const abstract_key_type& aKey, const abstract_mapped_type& aMapped) final
        {
            return new container_iterator{ iMap.insert(
                typename std_type::value_type{
                    typename std_type::key_type{aKey},
                    typename std_type::mapped_type{
                        key_type{aKey},
                        mapped_type{aMapped}} }) };
        }
        abstract_const_iterator* do_find(const abstract_key_type& aKey) const final { return new container_const_iterator{ iMap.find(aKey) }; }
        abstract_iterator* do_find(const abstract_key_type& aKey) final { return new container_iterator{ iMap.find(aKey) }; }
        abstract_const_iterator* do_lower_bound(const abstract_key_type& aKey) const final { return new container_const_iterator{ iMap.lower_bound(aKey) }; }
        abstract_iterator* do_lower_bound(const abstract_key_type& aKey) final { return new container_iterator{ iMap.lower_bound(aKey) }; }
        abstract_const_iterator* do_upper_bound(const abstract_key_type& aKey) const final { return new container_const_iterator{ iMap.upper_bound(aKey) }; }
        abstract_iterator* do_upper_bound(const abstract_key_type& aKey) final { return new container_iterator{ iMap.upper_bound(aKey) }; }
    private:
        std_type iMap;
    };
}
