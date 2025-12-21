// i_ecs.hpp
/*
 *  Copyright (c) 2018-2025 Leigh Johnston.
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
#include <boost/unordered/unordered_flat_map.hpp>
#include <neolib/core/i_mutex.hpp>
#include <neolib/task/thread_pool.hpp>
#include <neolib/task/event.hpp>
#include <neolib/app/i_object.hpp>
#include <neolib/ecs/ecs_ids.hpp>
#include <neolib/ecs/i_entity_archetype.hpp>
#include <neolib/ecs/i_component.hpp>
#include <neolib/ecs/i_system.hpp>

namespace neolib::ecs
{
    using neolib::to_const;

    enum class ecs_flags : uint32_t
    {
        None                = 0x0000,
        PopulateEntityInfo  = 0x0001,
        Turbo               = 0x0002,
        CreatePaused        = 0x0004,
        NoThreads           = 0x0008,

        Default             = PopulateEntityInfo | Turbo
    };

    inline constexpr ecs_flags operator|(ecs_flags aLhs, ecs_flags aRhs)
    {
        return static_cast<ecs_flags>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr ecs_flags operator&(ecs_flags aLhs, ecs_flags aRhs)
    {
        return static_cast<ecs_flags>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline constexpr ecs_flags& operator|=(ecs_flags& aLhs, ecs_flags aRhs)
    {
        return aLhs = static_cast<ecs_flags>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr ecs_flags& operator&=(ecs_flags& aLhs, ecs_flags aRhs)
    {
        return aLhs = static_cast<ecs_flags>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    class i_ecs : public i_object
    {
    public:
        declare_event(systems_paused)
        declare_event(systems_resumed)
        declare_event(entity_created, entity_id)
        declare_event(entity_destroyed, entity_id)
        declare_event(handle_updated, handle_id)
    public:
        struct entity_archetype_not_found : std::logic_error { entity_archetype_not_found() : std::logic_error("i_ecs::entity_archetype_not_found") {} };
        struct component_not_found : std::logic_error { component_not_found() : std::logic_error("i_ecs::component_not_found") {} };
        struct system_not_found : std::logic_error { system_not_found() : std::logic_error("i_ecs::system_not_found") {} };
        struct uuid_exists : std::runtime_error { uuid_exists(const std::string& aContext) : std::runtime_error("i_ecs::uuid_exists: " + aContext) {} };
        struct entity_ids_exhausted : std::runtime_error { entity_ids_exhausted() : std::runtime_error("i_ecs::entity_ids_exhausted") {} };
        struct handle_ids_exhausted : std::runtime_error { handle_ids_exhausted() : std::runtime_error("i_ecs::handle_ids_exhausted") {} };
        struct invalid_handle_id : std::logic_error { invalid_handle_id() : std::logic_error("i_ecs::invalid_handle_id") {} };
    public:
        typedef std::function<std::unique_ptr<i_component>()> component_factory;
        typedef std::function<std::unique_ptr<i_shared_component>()> shared_component_factory;
        typedef std::function<std::unique_ptr<i_system>()> system_factory;
    protected:
        typedef boost::unordered_flat_map<entity_archetype_id, std::shared_ptr<const i_entity_archetype>, quick_uuid_hash> archetype_registry_t;
        typedef boost::unordered_flat_map<component_id, component_factory, quick_uuid_hash> component_factories_t;
        typedef boost::unordered_flat_map<component_id, std::unique_ptr<i_component>, quick_uuid_hash> components_t;
        typedef boost::unordered_flat_map<component_id, shared_component_factory, quick_uuid_hash> shared_component_factories_t;
        typedef boost::unordered_flat_map<component_id, std::unique_ptr<i_shared_component>, quick_uuid_hash> shared_components_t;
        typedef boost::unordered_flat_map<system_id, system_factory, quick_uuid_hash> system_factories_t;
        typedef boost::unordered_flat_map<system_id, std::unique_ptr<i_system>, quick_uuid_hash> systems_t;
    public:
        typedef void* handle_t;
    public:
        virtual neolib::i_lockable& mutex() const = 0;
        virtual neolib::i_lockable& entity_mutex() const = 0;
        virtual neolib::i_lockable& archetype_mutex() const = 0;
        virtual neolib::i_lockable& component_factory_mutex() const = 0;
        virtual neolib::i_lockable& component_mutex() const = 0;
        virtual neolib::i_lockable& shared_component_factory_mutex() const = 0;
        virtual neolib::i_lockable& shared_component_mutex() const = 0;
        virtual neolib::i_lockable& system_factory_mutex() const = 0;
        virtual neolib::i_lockable& system_mutex() const = 0;
        virtual neolib::thread_pool& thread_pool() const = 0; // todo: polymorphic threadpool
    public:
        virtual ecs_flags flags() const = 0;
        virtual entity_id create_entity(const entity_archetype_id& aArchetypeId) = 0;
        virtual void async_create_entity(const std::function<void()>& aCreator) = 0; // todo: polymorphic functor
        virtual void commit_async_entity_creation() = 0;
        virtual void destroy_entity(entity_id aEntityId, bool aNotify = true) = 0;
        virtual void async_destroy_entity(entity_id aEntityId, bool aNotify = true) = 0;
        virtual void commit_async_entity_destruction() = 0;
    public:
        virtual bool run_threaded(const system_id& aSystemId) const = 0;
        virtual bool all_systems_paused() const = 0;
        virtual void pause_all_systems() = 0;
        virtual void resume_all_systems() = 0;
    public:
        virtual const archetype_registry_t& archetypes() const = 0;
        virtual archetype_registry_t& archetypes() = 0;
        virtual const component_factories_t& component_factories() const = 0;
        virtual component_factories_t& component_factories() = 0;
        virtual const components_t& components() const = 0;
        virtual components_t& components() = 0;
        virtual const shared_component_factories_t& shared_component_factories() const = 0;
        virtual shared_component_factories_t& shared_component_factories() = 0;
        virtual const shared_components_t& shared_components() const = 0;
        virtual shared_components_t& shared_components() = 0;
        virtual const system_factories_t& system_factories() const = 0;
        virtual system_factories_t& system_factories() = 0;
        virtual const systems_t& systems() const = 0;
        virtual systems_t& systems() = 0;
    public:
        virtual const i_entity_archetype& archetype(entity_archetype_id aArchetypeId) const = 0;
        virtual i_entity_archetype& archetype(entity_archetype_id aArchetypeId) = 0;
        virtual bool component_instantiated(component_id aComponentId) const = 0;
        virtual const i_component& component(component_id aComponentId) const = 0;
        virtual i_component& component(component_id aComponentId) = 0;
        virtual bool shared_component_instantiated(component_id aComponentId) const = 0;
        virtual const i_shared_component& shared_component(component_id aComponentId) const = 0;
        virtual i_shared_component& shared_component(component_id aComponentId) = 0;
        virtual bool system_instantiated(system_id aSystemId) const = 0;
        virtual const i_system& system(system_id aSystemId) const = 0;
        virtual i_system& system(system_id aSystemId) = 0;
    public:
        virtual entity_id next_entity_id() = 0;
        virtual void free_entity_id(entity_id aId) = 0;
    public:
        virtual bool archetype_registered(const i_entity_archetype& aArchetype) const = 0;
        virtual void register_archetype(const i_entity_archetype& aArchetype) = 0;
        virtual void register_archetype(std::shared_ptr<const i_entity_archetype> aArchetype) = 0;
        virtual bool component_registered(component_id aComponentId) const = 0;
        virtual void register_component(component_id aComponentId, component_factory aFactory) = 0;
        virtual bool shared_component_registered(component_id aComponentId) const = 0;
        virtual void register_shared_component(component_id aComponentId, shared_component_factory aFactory) = 0;
        virtual bool system_registered(system_id aSystemId) const = 0;
        virtual void register_system(system_id aSystemId, system_factory aFactory) = 0;
    public:
        virtual handle_t to_handle(handle_id aId) const = 0;
        virtual handle_id add_handle(const std::type_info& aTypeInfo, handle_t aHandle) = 0;
        virtual handle_t update_handle(handle_id aId, const std::type_info& aTypeInfo, handle_t aHandle) = 0;
        virtual handle_t release_handle(handle_id aId) = 0;
        // helpers
    public:
        template <typename... ComponentData>
        entity_id create_entity(const entity_archetype_id& aArchetypeId, ComponentData&&... aComponentData);
        template <typename Archetype, typename... ComponentData>
        entity_id create_entity(const Archetype& aArchetype, ComponentData&&... aComponentData);
        template <typename... ComponentData>
        void async_create_entity(entity_archetype_id aArchetypeId, ComponentData... aComponentData);
        template <typename Archetype, typename... ComponentData>
        void async_create_entity(const Archetype& aArchetype, ComponentData... aComponentData);
    public:
        template <typename ComponentData, typename... ComponentDataRest>
        void populate(entity_id aEntity, ComponentData&& aComponentData, ComponentDataRest&&... aComponentDataRest)
        {
            populate(aEntity, std::forward<ComponentData>(aComponentData));
            populate(aEntity, std::forward<ComponentDataRest>(aComponentDataRest)...);
        }
        template <typename ComponentData>
        void populate(entity_id aEntity, ComponentData&& aComponentData)
        {
            component<ecs_data_type_t<ComponentData>>().populate(aEntity, std::forward<ComponentData>(aComponentData));
        }
        template <typename ComponentData, typename... ComponentDataRest>
        void populate_shared(const std::string& aName, ComponentData&& aComponentData, ComponentDataRest&&... aComponentDataRest)
        {
            populate_shared(aName, std::forward<ComponentData>(aComponentData));
            populate_shared(aName, std::forward<ComponentDataRest>(aComponentDataRest)...);
        }
        template <typename ComponentData>
        void populate_shared(const std::string& aName, ComponentData&& aComponentData)
        {
            shared_component<ecs_data_type_t<ComponentData>>().populate(aName, std::forward<ComponentData>(aComponentData));
        }
        template <typename ComponentData>
        bool component_instantiated() const
        {
            return component_instantiated(ecs_data_type_t<ComponentData>::meta::id());
        }
        template <typename ComponentData>
        const neolib::ecs::component<ComponentData>& component() const
        {
            return static_cast<const neolib::ecs::component<ecs_data_type_t<ComponentData>>&>(component(ecs_data_type_t<ComponentData>::meta::id()));
        }
        template <typename ComponentData>
        neolib::ecs::component<ComponentData>& component()
        {
            if (!component_registered<ecs_data_type_t<ComponentData>>())
                register_component<ecs_data_type_t<ComponentData>>();
            return const_cast<neolib::ecs::component<ecs_data_type_t<ComponentData>>&>(to_const(*this).component<ecs_data_type_t<ComponentData>>());
        }
        template <typename ComponentData>
        bool shared_component_instantiated() const
        {
            return shared_component_instantiated(ecs_data_type_t<ComponentData>::meta::id());
        }
        template <typename ComponentData>
        const neolib::ecs::shared_component<ComponentData>& shared_component() const
        {
            return static_cast<const neolib::ecs::shared_component<ecs_data_type_t<ComponentData>>&>(shared_component(ComponentData::meta::id()));
        }
        template <typename ComponentData>
        neolib::ecs::shared_component<ComponentData>& shared_component()
        {
            if (!shared_component_registered<ecs_data_type_t<ComponentData>>())
                register_shared_component<ecs_data_type_t<ComponentData>>();
            return const_cast<neolib::ecs::shared_component<ecs_data_type_t<ComponentData>>&>(to_const(*this).shared_component<ecs_data_type_t<ComponentData>>());
        }
        template <typename System>
        bool system_instantiated() const
        {
            return system_instantiated(ecs_data_type_t<System>::meta::id());
        }
        template <typename System>
        const ecs_data_type_t<System>& system() const
        {
            return static_cast<const ecs_data_type_t<System>&>(system(ecs_data_type_t<System>::meta::id()));
        }
        template <typename System>
        ecs_data_type_t<System>& system()
        {
            if (!system_registered<ecs_data_type_t<System>>())
                register_system<ecs_data_type_t<System>>();
            return const_cast<ecs_data_type_t<System>&>(to_const(*this).system<ecs_data_type_t<System>>());
        }
    public:
        template <typename ComponentData>
        bool component_registered() const
        {
            return component_registered(ecs_data_type_t<ComponentData>::meta::id());
        }
        template <typename ComponentData>
        void register_component()
        {
            register_component(ecs_data_type_t<ComponentData>::meta::id(), [&]() { return std::unique_ptr<i_component>{std::make_unique<neolib::ecs::component<ecs_data_type_t<ComponentData>>>(*this)}; });
        }
        template <typename ComponentData>
        bool shared_component_registered() const
        {
            return shared_component_registered(ecs_data_type_t<ComponentData>::meta::id());
        }
        template <typename ComponentData>
        void register_shared_component()
        {
            register_shared_component(ecs_data_type_t<ComponentData>::meta::id(), [&]() { return std::unique_ptr<i_shared_component>{std::make_unique<neolib::ecs::shared_component<ecs_data_type_t<ComponentData>>>(*this)}; });
        }
        template <typename System>
        bool system_registered() const
        {
            return system_registered(ecs_data_type_t<System>::meta::id());
        }
        template <typename System>
        void register_system()
        {
            register_system(
                ecs_data_type_t<System>::meta::id(),
                [&]() { return std::unique_ptr<i_system>{std::make_unique<ecs_data_type_t<System>>(*this)}; });
        }
    public:
        template <typename Handle>
        Handle to_handle(handle_id aId) const
        {
            return reinterpret_cast<Handle>(to_handle(aId));
        }
        template <typename Context, typename Handle>
        handle_id add_handle(Handle aHandle)
        {
            return add_handle(typeid(Context), reinterpret_cast<handle_t>(aHandle));
        }
        template <typename Context, typename Handle>
        Handle update_handle(handle_id aId, Handle aHandle)
        {
            if constexpr(std::is_pointer<Handle>::value)
                return reinterpret_cast<Handle>(update_handle(aId, typeid(Context), reinterpret_cast<handle_t>(aHandle)));
            else
                return static_cast<Handle>(reinterpret_cast<intptr_t>(update_handle(aId, typeid(Context), reinterpret_cast<handle_t>(aHandle))));
        }
        template <typename Handle>
        Handle release_handle(handle_id aId)
        {
            if constexpr(std::is_pointer<Handle>::value)
                return reinterpret_cast<Handle>(release_handle(aId));
            else
                return static_cast<Handle>(reinterpret_cast<intptr_t>(release_handle(aId)));
        }
    };

    template <typename Data>
    class shared_component_scoped_lock
    {
    public:
        shared_component_scoped_lock(const i_ecs& aEcs) :
            iLock{ aEcs.shared_component<Data>().mutex() }
        {
        }
        ~shared_component_scoped_lock()
        {
        }
    private:
        std::scoped_lock<neolib::i_lockable> iLock;
    };

    const struct dont_lock_t {} dont_lock;

    template <typename... Data>
    class scoped_component_lock
    {
    private:
        template <typename T, typename>
        static T fwd(T o) { return o; }
        class proxy_mutex_base : public i_lockable
        {
        public:
            struct not_linked : std::logic_error { not_linked() : std::logic_error{"neolib::neolib::ecs::scoped_component_lock::proxy_mutex::not_linked"} {} };
        public:
            proxy_mutex_base(i_lockable& aSubject) :
                iSubject{ &aSubject }
            {
            }
        public:
            void lock() noexcept final
            {
                if (linked())
                    subject().lock();
            }
            void unlock() noexcept final
            {
                if (linked())
                    subject().unlock();
            }
            bool try_lock() noexcept final
            {
                if (linked())
                    return subject().try_lock();
                else
                    return true;
            }
        public:
            bool operator<(proxy_mutex_base const& rhs) const
            {
                return std::less<i_lockable*>{}(iSubject, rhs.iSubject);
            }
        public:
            i_lockable& subject()
            {
                if (linked())
                    return *iSubject;
                throw not_linked();
            }
        public:
            bool linked() const
            {
                return iSubject != nullptr;
            }
            i_lockable& unlink()
            {
                if (linked())
                {
                    auto& link = *iSubject;
                    iSubject = nullptr;
                    return link;
                }
                throw not_linked();
            }
        private:
            i_lockable* iSubject;
        };

        template <typename Data2>
        class proxy_mutex : public proxy_mutex_base
        {
        public:
            proxy_mutex(const i_ecs& aEcs) :
                proxy_mutex_base{ aEcs.component<Data2>().mutex() }
            {
            }
            proxy_mutex(i_ecs& aEcs) :
                proxy_mutex_base{ aEcs.component<Data2>().mutex() }
            {
            }
        };

    public:
        scoped_component_lock(const i_ecs& aEcs) :
            iProxies{ fwd<const i_ecs&, Data>(aEcs)... }
        {
            create_lockable_array();
            lock();
        }
        scoped_component_lock(i_ecs& aEcs) :
            iProxies{ fwd<i_ecs&, Data>(aEcs)... }
        {
            create_lockable_array();
            lock();
        }
        scoped_component_lock(const i_ecs& aEcs, dont_lock_t) :
            iProxies{ fwd<const i_ecs&, Data>(aEcs)... }
        {
            create_lockable_array();
            iDontUnlock.emplace();
        }
        scoped_component_lock(i_ecs& aEcs, dont_lock_t) :
            iProxies{ fwd<i_ecs&, Data>(aEcs)... }
        {
            create_lockable_array();
            iDontUnlock.emplace();
        }
        ~scoped_component_lock()
        {
            if (!iDontUnlock)
                unlock();
        }

    public:
        void lock() noexcept
        {
            if constexpr (sizeof...(Data) >= 2)
                std::apply([](auto&... mx) {
                    std::lock(*mx...);
                        }, iLockables);
            else if constexpr (sizeof...(Data) == 1)
                iLockables[0]->lock();
        }
        void unlock() noexcept
        {
            for (std::size_t lockable = iLockables.size(); lockable-- > 0; )
                iLockables[lockable]->unlock();
        }
        bool try_lock() noexcept
        {
            for (std::size_t lockable = 0u; lockable < iLockables.size(); ++lockable)
                if (!iLockables[lockable]->try_lock())
                {
                    for (std::size_t lockable2 = lockable; lockable2-- > 0;)
                        iLockables[lockable2]->unlock();
                    return false;
                }
            return true;
        }

    public:
        template <typename Data2>
        i_lockable& mutex()
        {
            auto& m = std::get<index_of_v<Data2, Data...>>(iProxies).unlink();
            return m;
        }
        template <typename Data2>
        bool controlling() const
        {
            return std::get<index_of_v<Data2, Data...>>(iProxies).linked();
        }
        template <typename... Data2>
        void lock_if()
        {
            if constexpr (sizeof...(Data2) == 0)
                return;

            for (auto lockable : iLockables)
            {
                bool shouldLock =
                    ((
                        controlling<Data2>() &&
                        (&std::get<index_of_v<Data2, Data...>>(iProxies) == lockable)
                        ) || ...);

                if (shouldLock)
                    lockable->lock();
            }
        }
        template <typename... Data2>
        void unlock_if()
        {
            if constexpr (sizeof...(Data2) == 0)
                return;

            for (std::size_t i = iLockables.size(); i-- > 0; )
            {
                auto lockable = iLockables[i];

                bool shouldUnlock =
                    ((
                        controlling<Data2>() &&
                        (&std::get<index_of_v<Data2, Data...>>(iProxies) == lockable)
                        ) || ...);

                if (shouldUnlock)
                    lockable->unlock();
            }
        }

    private:
        void create_lockable_array()
        {
            std::size_t index = 0;
            std::apply(
                [&](auto&... proxies)
                    {
                        ((iLockables[index++] = &proxies), ...);
                    },
                iProxies
            );
            std::sort(iLockables.begin(), iLockables.end(), [](proxy_mutex_base* lhs, proxy_mutex_base* rhs) { return *lhs < *rhs; });
        }

    private:
        std::tuple<proxy_mutex<Data>...> iProxies;
        std::array<proxy_mutex_base*, sizeof...(Data)> iLockables;
        std::optional<dont_lock_t> iDontUnlock;
    };

    template <typename... UnlockData>
    class scoped_component_relock
    {
    public:
        template <typename... Data>
        scoped_component_relock(scoped_component_lock<Data...>& aLock, bool aUnlock = true) : 
            iRelock{ [&]() { aLock.template lock_if<UnlockData...>(); } }
        {
            if (aUnlock)
                aLock.template unlock_if<UnlockData...>();
        }
        ~scoped_component_relock()
        {
            iRelock();
        }
    private:
        std::function<void()> iRelock;
    };

    template <typename... ComponentData>
    inline entity_id i_ecs::create_entity(const entity_archetype_id& aArchetypeId, ComponentData&&... aComponentData)
    {
        scoped_component_lock<std::decay_t<ComponentData>...> lock{ *this };
        auto newEntity = create_entity(aArchetypeId);
        populate(newEntity, std::forward<ComponentData>(aComponentData)...);
        archetype(aArchetypeId).populate_default_components(*this, newEntity);
        return newEntity;
    }

    template <typename Archetype, typename... ComponentData>
    inline entity_id i_ecs::create_entity(const Archetype& aArchetype, ComponentData&&... aComponentData)
    {
        if (!archetype_registered(aArchetype))
            register_archetype(aArchetype);
        return create_entity(aArchetype.id(), std::forward<ComponentData>(aComponentData)...);
    }

    template <typename... ComponentData>
    inline void i_ecs::async_create_entity(entity_archetype_id aArchetypeId, ComponentData... aComponentData)
    {
        auto creator = [=, this]()
        {
            create_entity(aArchetypeId, aComponentData...);
        };
        async_create_entity(std::function<void()>{ creator });
    }

    template <typename Archetype, typename... ComponentData>    
    inline void i_ecs::async_create_entity(const Archetype& aArchetype, ComponentData... aComponentData)
    {
        auto creator = [=, this, &aArchetype]()
        {
            if (!archetype_registered(aArchetype))
                register_archetype(aArchetype);
            create_entity(aArchetype, aComponentData...);
        };
        async_create_entity(std::function<void()>{ creator });
    }
}
