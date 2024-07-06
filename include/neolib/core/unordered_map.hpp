// unordered_map.hpp
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
#include <unordered_map>
#include <boost/unordered/unordered_flat_map.hpp>
#include <neolib/core/pair.hpp>
#include <neolib/core/container_iterator.hpp>
#include <neolib/core/i_unordered_map.hpp>

namespace neolib
{
    template <typename Key, typename T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, typename Alloc = std::allocator<std::pair<const Key, T>>>
    class unordered_map : public reference_counted<i_unordered_map<abstract_t<Key>, abstract_t<T>>>
    {
        using base_type = reference_counted<i_unordered_map<abstract_t<Key>, abstract_t<T>>>;
        // types
    public:
        using abstract_type = i_unordered_map<abstract_t<Key>, abstract_t<T>>;
        using key_type = Key;
        using mapped_type = T;
        using value_type = pair<const key_type, mapped_type>;
        using abstract_key_type = abstract_t<key_type>;
        using abstract_mapped_type = abstract_t<mapped_type>;
        using abstract_value_type = i_pair<const abstract_key_type, abstract_mapped_type>;
        using key_hash = Hash;
        using key_equal = KeyEqual;
        using allocator_type = Alloc;
        using std_type = std::unordered_map<key_type, value_type, key_hash, key_equal, typename std::allocator_traits<allocator_type>::template rebind_alloc<std::pair<const key_type, value_type>>>;
        using container_value_type = typename std_type::value_type;
    public:
        using typename abstract_type::size_type;
        using typename abstract_type::const_iterator;
        using typename abstract_type::iterator;
        using typename abstract_type::generic_container_type;
    protected:
        using container_const_iterator = container::const_iterator<value_type, typename std_type::const_iterator>;
        using container_iterator = container::iterator<value_type, typename std_type::iterator, typename std_type::const_iterator>;
        using abstract_const_iterator = typename abstract_type::abstract_const_iterator;
        using abstract_iterator = typename abstract_type::abstract_iterator;
        // construction
    public:
        unordered_map() 
        {
        }
        unordered_map(const unordered_map& aOther) :
            iMap{ aOther.iMap }
        {
        }
        unordered_map(unordered_map&& aOther) :
            iMap{ std::move(aOther.iMap) }
        {
        }
        unordered_map(const std::initializer_list<value_type>& aIlist)
        {
            for (auto& value : aIlist)
                iMap.emplace(value.first(), value);
        }
        unordered_map(const generic_container_type& aOther)
        {
            assign(aOther);
        }
        template <typename InputIter>
        unordered_map(InputIter aFirst, InputIter aLast) :
            iMap(aFirst, aLast)
        {}
        // assignment
    public:
        unordered_map& operator=(const unordered_map& aOther)
        {
            iMap = aOther.iMap;
            return *this;
        }
        unordered_map& operator=(unordered_map&& aOther)
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
        constexpr bool operator==(const unordered_map& that) const noexcept
        {
            return as_std_map() == that.as_std_map();
        }
        constexpr bool operator!=(const unordered_map& that) const noexcept
        {
            return as_std_map() != that.as_std_map();
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
        abstract_const_iterator* do_begin(void* memory) const final { return new (memory) container_const_iterator{ iMap.begin() }; }
        abstract_const_iterator* do_end(void* memory) const final { return new (memory) container_const_iterator{ iMap.end() }; }
        abstract_iterator* do_begin(void* memory) final { return new (memory) container_iterator{ iMap.begin() }; }
        abstract_iterator* do_end(void* memory) final { return new (memory) container_iterator{ iMap.end() }; }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aPosition) final { return new (memory) container_iterator{ iMap.erase(static_cast<const container_const_iterator&>(aPosition)) }; }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) final { return new (memory) container_iterator{ iMap.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast)) }; }
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
        abstract_iterator* do_insert(void* memory, const abstract_key_type& aKey, const abstract_mapped_type& aMapped) final
        { 
            return new (memory) container_iterator{ iMap.insert(
                typename std_type::value_type{
                    typename std_type::key_type{aKey},
                    typename std_type::mapped_type{
                        key_type{aKey},
                        mapped_type{aMapped}} }).first };
        }
        abstract_const_iterator* do_find(void* memory, const abstract_key_type& aKey) const final { return new (memory) container_const_iterator{ iMap.find(aKey) }; }
        abstract_iterator* do_find(void* memory, const abstract_key_type& aKey) final { return new (memory) container_iterator{ iMap.find(aKey) }; }
    private:
        std_type iMap;
    };

    template <typename Key, typename T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, typename Alloc = std::allocator<std::pair<const Key, T>>>
    class unordered_flat_map : public reference_counted<i_unordered_map<abstract_t<Key>, abstract_t<T>>>
    {
        using base_type = reference_counted<i_unordered_map<abstract_t<Key>, abstract_t<T>>>;
        // types
    public:
        using abstract_type = i_unordered_map<abstract_t<Key>, abstract_t<T>>;
        using key_type = Key;
        using mapped_type = T;
        using value_type = pair<const key_type, mapped_type>;
        using abstract_key_type = abstract_t<key_type>;
        using abstract_mapped_type = abstract_t<mapped_type>;
        using abstract_value_type = i_pair<const abstract_key_type, abstract_mapped_type>;
        using key_hash = Hash;
        using key_equal = KeyEqual;
        using allocator_type = Alloc;
        using std_type = boost::unordered_flat_map<key_type, value_type, key_hash, key_equal, typename std::allocator_traits<allocator_type>::template rebind_alloc<std::pair<const key_type, value_type>>>;
        using container_value_type = typename std_type::value_type;
    public:
        using typename abstract_type::size_type;
        using typename abstract_type::const_iterator;
        using typename abstract_type::iterator;
        using typename abstract_type::generic_container_type;
    protected:
        using container_const_iterator = container::const_iterator<value_type, typename std_type::const_iterator>;
        using container_iterator = container::iterator<value_type, typename std_type::iterator, typename std_type::const_iterator>;
        using abstract_const_iterator = typename abstract_type::abstract_const_iterator;
        using abstract_iterator = typename abstract_type::abstract_iterator;
        // construction
    public:
        unordered_flat_map()
        {
        }
        unordered_flat_map(const unordered_flat_map& aOther) :
            iMap{ aOther.iMap }
        {
        }
        unordered_flat_map(unordered_flat_map&& aOther) :
            iMap{ std::move(aOther.iMap) }
        {
        }
        unordered_flat_map(const std::initializer_list<value_type>& aIlist)
        {
            for (auto& value : aIlist)
                iMap.emplace(value.first(), value);
        }
        unordered_flat_map(const generic_container_type& aOther)
        {
            assign(aOther);
        }
        template <typename InputIter>
        unordered_flat_map(InputIter aFirst, InputIter aLast) :
            iMap(aFirst, aLast)
        {}
        // assignment
    public:
        unordered_flat_map& operator=(const unordered_flat_map& aOther)
        {
            iMap = aOther.iMap;
            return *this;
        }
        unordered_flat_map& operator=(unordered_flat_map&& aOther)
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
        constexpr bool operator==(const unordered_flat_map& that) const noexcept
        {
            return as_std_map() == that.as_std_map();
        }
        constexpr bool operator!=(const unordered_flat_map& that) const noexcept
        {
            return as_std_map() != that.as_std_map();
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
        abstract_const_iterator* do_begin(void* memory) const final { return new (memory) container_const_iterator{ iMap.begin() }; }
        abstract_const_iterator* do_end(void* memory) const final { return new (memory) container_const_iterator{ iMap.end() }; }
        abstract_iterator* do_begin(void* memory) final { return new (memory) container_iterator{ iMap.begin() }; }
        abstract_iterator* do_end(void* memory) final { return new (memory) container_iterator{ iMap.end() }; }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aPosition) final { return new (memory) container_iterator{ iMap.erase(static_cast<const container_const_iterator&>(aPosition)) }; }
        abstract_iterator* do_erase(void* memory, const abstract_const_iterator& aFirst, const abstract_const_iterator& aLast) final { return new (memory) container_iterator{ iMap.erase(static_cast<const container_const_iterator&>(aFirst), static_cast<const container_const_iterator&>(aLast)) }; }
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
        abstract_iterator* do_insert(void* memory, const abstract_key_type& aKey, const abstract_mapped_type& aMapped) final
        {
            return new (memory) container_iterator{ iMap.insert(
                typename std_type::value_type{
                    typename std_type::key_type{aKey},
                    typename std_type::mapped_type{
                        key_type{aKey},
                        mapped_type{aMapped}} }).first };
        }
        abstract_const_iterator* do_find(void* memory, const abstract_key_type& aKey) const final { return new (memory) container_const_iterator{ iMap.find(aKey) }; }
        abstract_iterator* do_find(void* memory, const abstract_key_type& aKey) final { return new (memory) container_iterator{ iMap.find(aKey) }; }
    private:
        std_type iMap;
    };
}
