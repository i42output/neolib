// component.hpp
/*
 *  Copyright (c) 2018, 2020 Leigh Johnston.
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
#include <atomic>
#include <vector>
#include <unordered_map>
#include <string>
#include <neolib/core/intrusive_sort.hpp>
#include <neolib/task/thread_pool.hpp>
#include <neolib/ecs/ecs_ids.hpp>
#include <neolib/ecs/i_ecs.hpp>

namespace neolib::ecs
{
    class i_ecs;

    template <typename Data>
    struct shared;

    template <typename T>
    inline bool batchable(const std::optional<T>& lhs, const std::optional<T>& rhs)
    {
        return !!lhs == !!rhs && (lhs == std::nullopt || batchable(*lhs, *rhs));
    }

    template <typename Data>
    inline bool batchable(const shared<Data>& lhs, const shared<Data>& rhs)
    {
        if (!!lhs.ptr != !!rhs.ptr)
            return false;
        if (lhs.ptr == nullptr)
            return true;
        return batchable(*lhs.ptr, *rhs.ptr);
    }

    namespace detail
    {
        template <typename Data>
        struct crack_component_data
        {
            typedef ecs_data_type_t<Data> data_type;
            typedef data_type value_type;
            typedef std::vector<value_type> container_type;
            static constexpr bool optional = false;
        };

        template <typename Data>
        struct crack_component_data<std::optional<Data>>
        {
            typedef ecs_data_type_t<Data> data_type;
            typedef std::optional<data_type> value_type;
            typedef std::vector<value_type> container_type;
            static constexpr bool optional = true;
        };

        template <typename Data>
        struct crack_component_data<shared<Data>>
        {
            typedef ecs_data_type_t<Data> data_type;
            typedef data_type mapped_type;
            typedef std::pair<const std::string, mapped_type> value_type;
            typedef std::unordered_map<std::string, mapped_type> container_type;
            static constexpr bool optional = false;
        };

        template <typename Data>
        struct crack_component_data<shared<std::optional<Data>>>
        {
            typedef ecs_data_type_t<Data> data_type;
            typedef std::optional<data_type> mapped_type;
            typedef std::pair<const std::string, mapped_type> value_type;
            typedef std::unordered_map<std::string, mapped_type> container_type;
            static constexpr bool optional = true;
        };
    }

    // Mutex tagged with component data type (visible in debugger) to help debugging multi-threaded issues
    template <typename Data>
    struct component_mutex : neolib::recursive_spinlock
    {
    };

    template <typename Data, typename Base>
    class component_base : public Base
    {
        typedef component_base<ecs_data_type_t<Data>, Base> self_type;
    public:
        struct entity_record_not_found : std::logic_error { entity_record_not_found() : std::logic_error("neolib::component::entity_record_not_found") {} };
        struct invalid_data : std::logic_error { invalid_data() : std::logic_error("neolib::component::invalid_data") {} };
    public:
        typedef typename detail::crack_component_data<Data>::data_type data_type;
        typedef typename data_type::meta data_meta_type;
        typedef typename detail::crack_component_data<Data>::value_type value_type;
        typedef typename detail::crack_component_data<Data>::container_type component_data_t;
    public:
        component_base(i_ecs& aEcs) : 
            iEcs{ aEcs }
        {
        }
        component_base(const self_type& aOther) :
            iEcs{ aOther.iEcs },
            iComponentData{ aOther.iComponentData }
        {
        }
    public:
        self_type& operator=(const self_type& aRhs)
        {
            iComponentData = aRhs.iComponentData;
            return *this;
        }
    public:
        i_ecs& ecs() const override
        {
            return iEcs;
        }
        const component_id& id() const override
        {
            return data_meta_type::id();
        }
    public:
        component_mutex<Data>& mutex() const override
        {
            return iMutex;
        }
    public:
        bool is_data_optional() const override
        {
            return detail::crack_component_data<Data>::optional;
        }
        const neolib::i_string& name() const override
        {
            return data_meta_type::name();
        }
        uint32_t field_count() const override
        {
            return data_meta_type::field_count();
        }
        component_data_field_type field_type(uint32_t aFieldIndex) const override
        {
            return data_meta_type::field_type(aFieldIndex);
        }
        neolib::uuid field_type_id(uint32_t aFieldIndex) const override
        {
            return data_meta_type::field_type_id(aFieldIndex);
        }
        const neolib::i_string& field_name(uint32_t aFieldIndex) const override
        {
            return data_meta_type::field_name(aFieldIndex);
        }
    public:
        const component_data_t& component_data() const
        {
            return iComponentData;
        }
        component_data_t& component_data()
        {
            return iComponentData;
        }
        const value_type& operator[](typename component_data_t::size_type aIndex) const
        {
            return *std::next(component_data().begin(), aIndex);
        }
        value_type& operator[](typename component_data_t::size_type aIndex)
        {
            return *std::next(component_data().begin(), aIndex);
        }
    private:
        mutable component_mutex<Data> iMutex;
        i_ecs& iEcs;
        component_data_t iComponentData;
    };

    template <typename Data>
    class component : public component_base<Data, i_component>
    {
        typedef component<Data> self_type;
        typedef component_base<Data, i_component> base_type;
    public:
        using typename base_type::entity_record_not_found;
        using typename base_type::invalid_data;
    public:
        typedef typename base_type::data_type data_type;
        typedef typename base_type::data_meta_type data_meta_type;
        typedef typename base_type::value_type value_type;
        typedef typename base_type::component_data_t component_data_t;
        typedef std::vector<entity_id> component_data_entities_t;
        typedef typename component_data_t::size_type reverse_index_t;
        typedef std::vector<reverse_index_t> reverse_indices_t;
    public:
        typedef std::unique_ptr<self_type> snapshot_ptr;
        class scoped_snapshot
        {
        public:
            scoped_snapshot(self_type& aOwner) :
                iOwner{ aOwner }
            {
                ++iOwner.iUsingSnapshot;
            }
            scoped_snapshot(const scoped_snapshot& aOther) :
                iOwner{ aOther.iOwner }
            {
                ++iOwner.iUsingSnapshot;
            }
            ~scoped_snapshot()
            {
                --iOwner.iUsingSnapshot;
            }
        public:
            self_type& data() const
            {
                return *iOwner.iSnapshot;
            }
        private:
            self_type& iOwner;
        };
    private:
        static constexpr reverse_index_t invalid = ~reverse_index_t{};
    public:
        component(i_ecs& aEcs) : 
            base_type{ aEcs },
            iHaveSnapshot{ false },
            iUsingSnapshot{ 0u }
        {
        }
        component(const self_type& aOther) :
            base_type{ aOther },
            iEntities{ aOther.iEntities },
            iReverseIndices{ aOther.iReverseIndices },
            iHaveSnapshot{ false },
            iUsingSnapshot{ 0u }
        {
        }
    public:
        self_type& operator=(const self_type& aRhs)
        {
            base_type::operator=(aRhs);
            iEntities = aRhs.iEntities;    
            iReverseIndices = aRhs.iReverseIndices;
            return *this;
        }
    public:
        using base_type::ecs;
        using base_type::id;
        using base_type::mutex;
    public:
        using base_type::is_data_optional;
        using base_type::name;
        using base_type::field_count;
        using base_type::field_type;
        using base_type::field_type_id;
        using base_type::field_name;
    public:
        using base_type::component_data;
        using base_type::operator[];
    public:
        entity_id entity(const value_type& aData) const
        {
            const value_type* lhs = &aData;
            const value_type* rhs = &base_type::component_data()[0];
            auto index = lhs - rhs;
            return entities()[index];
        }
        const component_data_entities_t& entities() const
        {
            return iEntities;
        }
        component_data_entities_t& entities()
        {
            return iEntities;
        }
        const reverse_indices_t& reverse_indices() const
        {
            return iReverseIndices;
        }
        reverse_indices_t& reverse_indices()
        {
            return iReverseIndices;
        }
        reverse_index_t reverse_index_no_lock(entity_id aEntity) const
        {
            if (reverse_indices().size() > aEntity)
                return reverse_indices()[aEntity];
            return invalid;
        }
        bool has_entity_record_no_lock(entity_id aEntity) const override
        {
            return reverse_index_no_lock(aEntity) != invalid;
        }
        const value_type& entity_record_no_lock(entity_id aEntity) const
        {
            auto reverseIndex = reverse_index_no_lock(aEntity);
            if (reverseIndex == invalid)
                throw entity_record_not_found();
            return base_type::component_data()[reverseIndex];
        }
        value_type& entity_record_no_lock(entity_id aEntity, bool aCreate = false)
        {
            if (aCreate && !has_entity_record_no_lock(aEntity))
                populate(aEntity, value_type{});
            return const_cast<value_type&>(to_const(*this).entity_record_no_lock(aEntity));
        }
        reverse_index_t reverse_index(entity_id aEntity) const
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            return reverse_index_no_lock(aEntity);
        }
        bool has_entity_record(entity_id aEntity) const override
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            return has_entity_record_no_lock(aEntity);
        }
        const value_type& entity_record(entity_id aEntity) const
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            return entity_record_no_lock(aEntity);
        }
        value_type& entity_record(entity_id aEntity, bool aCreate = false)
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            return entity_record_no_lock(aEntity, aCreate);
        }
        void destroy_entity_record(entity_id aEntity) override
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            auto reverseIndex = reverse_index(aEntity);
            if (reverseIndex == invalid)
                throw entity_record_not_found();
            if constexpr (data_meta_type::has_handles)
                data_meta_type::free_handles(base_type::component_data()[reverseIndex], ecs());
            std::swap(base_type::component_data()[reverseIndex], base_type::component_data().back());
            base_type::component_data().pop_back();
            auto tailEntity = entities().back();
            std::swap(entities()[reverseIndex], entities().back());
            entities().pop_back();
            reverse_indices()[tailEntity] = reverseIndex;
            reverse_indices()[aEntity] = invalid;
            if (have_snapshot())
            {
                auto ss = snapshot();
                if (ss.data().has_entity_record(aEntity))
                    ss.data().destroy_entity_record(aEntity);
            }
        }
        value_type& populate(entity_id aEntity, const value_type& aData)
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            return do_populate(aEntity, aData);
        }
        value_type& populate(entity_id aEntity, value_type&& aData)
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            return do_populate(aEntity, aData);
        }
        const void* populate(entity_id aEntity, const void* aComponentData, std::size_t aComponentDataSize) override
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            if ((aComponentData == nullptr && !is_data_optional()) || aComponentDataSize != sizeof(data_type))
                throw invalid_data();
            if (aComponentData != nullptr)
                return &do_populate(aEntity, *static_cast<const data_type*>(aComponentData));
            else
                return &do_populate(aEntity, value_type{}); // empty optional
        }
    public:
        bool have_snapshot() const
        {
            return iHaveSnapshot;
        }
        void take_snapshot()
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            if (!iUsingSnapshot)
            {
                if (iSnapshot == nullptr)
                    iSnapshot = snapshot_ptr{ new self_type{*this} };
                else
                    *iSnapshot = *this;
                iHaveSnapshot = true;
            }
        }
        scoped_snapshot snapshot()
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            return scoped_snapshot{ *this };
        }
        template <typename Compare>
        void sort(Compare aComparator)
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            neolib::intrusive_sort(base_type::component_data().begin(), base_type::component_data().end(),
                [this](auto lhs, auto rhs) 
                { 
                    std::swap(*lhs, *rhs);
                    auto lhsIndex = lhs - base_type::component_data().begin();
                    auto rhsIndex = rhs - base_type::component_data().begin();
                    auto& lhsEntity = entities()[lhsIndex];
                    auto& rhsEntity = entities()[rhsIndex];
                    std::swap(lhsEntity, rhsEntity);
                    if (lhsEntity != invalid)
                        reverse_indices()[lhsEntity] = lhsIndex;
                    if (rhsEntity != invalid)
                        reverse_indices()[rhsEntity] = rhsIndex;
                }, aComparator);
        }
    public:
        template <typename Callable>
        void apply(const Callable& aCallable)
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            for (auto& data : component_data())
                aCallable(*this, data);
        }
        template <typename Callable>
        void parallel_apply(const Callable& aCallable, std::size_t aMinimumParallelismCount = 0)
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            neolib::parallel_apply(ecs().thread_pool(), component_data(), [&](value_type& aData) { aCallable(*this, aData); }, aMinimumParallelismCount);
        }
    private:
        template <typename T>
        value_type& do_populate(entity_id aEntity, T&& aComponentData)
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            if (has_entity_record(aEntity))
                return do_update(aEntity, aComponentData);
            reverse_index_t reverseIndex = invalid;
            reverseIndex = base_type::component_data().size();
            base_type::component_data().push_back(std::forward<T>(aComponentData));
            try
            {
                entities().push_back(aEntity);
            }
            catch (...)
            {
                base_type::component_data().pop_back();
                throw;
            }
            try
            {
                if (reverse_indices().size() <= aEntity)
                    reverse_indices().resize(aEntity + 1, invalid);
                reverse_indices()[aEntity] = reverseIndex;
            }
            catch (...)
            {
                entities()[reverseIndex] = null_entity;
                throw;
            }
            return base_type::component_data()[reverseIndex];
        }
        template <typename T>
        value_type& do_update(entity_id aEntity, T&& aComponentData)
        {
            std::scoped_lock<component_mutex<Data>> lock{ mutex() };
            auto& record = entity_record(aEntity);
            record = aComponentData;
            return record;
        }
    private:
        component_data_entities_t iEntities;
        reverse_indices_t iReverseIndices;
        mutable std::atomic<bool> iHaveSnapshot;
        mutable std::atomic<uint32_t> iUsingSnapshot;
        mutable snapshot_ptr iSnapshot;
    };

    template <typename Data>
    struct shared
    {
        typedef typename detail::crack_component_data<shared<Data>>::mapped_type mapped_type;

        const mapped_type* ptr;

        shared() :
            ptr{ nullptr }
        {
        }
        shared(const mapped_type* aData) :
            ptr{ aData }
        {
        }
        shared(const mapped_type& aData) :
            ptr { &aData }
        {
        }
    };

    template <typename Data>
    class shared_component : public component_base<shared<ecs_data_type_t<Data>>, i_shared_component>
    {
        typedef shared_component<Data> self_type;
        typedef component_base<shared<ecs_data_type_t<Data>>, i_shared_component> base_type;
    public:
        using typename base_type::entity_record_not_found;
        using typename base_type::invalid_data;
    public:
        typedef typename base_type::data_type data_type;
        typedef typename base_type::data_meta_type data_meta_type;
        typedef typename base_type::value_type value_type;
        typedef typename base_type::component_data_t component_data_t;
        typedef typename component_data_t::mapped_type mapped_type;
    public:
        shared_component(i_ecs& aEcs) :
            base_type{ aEcs }
        {
        }
    public:
        using base_type::ecs;
        using base_type::id;
        using base_type::mutex;
    public:
        using base_type::is_data_optional;
        using base_type::name;
        using base_type::field_count;
        using base_type::field_type;
        using base_type::field_type_id;
        using base_type::field_name;
    public:
        using base_type::component_data;
    public:
        const mapped_type& operator[](typename component_data_t::size_type aIndex) const
        {
            return std::next(component_data().begin(), aIndex)->second;
        }
        mapped_type& operator[](typename component_data_t::size_type aIndex)
        {
            return std::next(component_data().begin(), aIndex)->second;
        }
        const mapped_type& operator[](const std::string& aName) const
        {
            return component_data()[aName];
        }
        mapped_type& operator[](const std::string& aName)
        {
            return component_data()[aName];
        }
    public:
        shared<mapped_type> populate(const std::string& aName, const mapped_type& aData)
        {
            base_type::component_data()[aName] = aData;
            auto& result = base_type::component_data()[aName];
            if constexpr (mapped_type::meta::has_updater)
                mapped_type::meta::update(result, ecs(), null_entity);
            return shared<mapped_type> { &result };
        }
        shared<mapped_type> populate(const std::string& aName, mapped_type&& aData)
        {
            base_type::component_data()[aName] = std::move(aData);
            auto& result = base_type::component_data()[aName];
            if constexpr (mapped_type::meta::has_updater)
                mapped_type::meta::update(result, ecs(), null_entity);
            return shared<mapped_type> { &result };
        }
        const void* populate(const std::string& aName, const void* aComponentData, std::size_t aComponentDataSize) override
        {
            if ((aComponentData == nullptr && !is_data_optional()) || aComponentDataSize != sizeof(mapped_type))
                throw invalid_data();
            if (aComponentData != nullptr)
                return populate(aName, *static_cast<const mapped_type*>(aComponentData)).ptr;
            else
                return populate(aName, mapped_type{}).ptr; // empty optional
        }
    };
}