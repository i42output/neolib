// ecs.hpp
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
#include <neolib/task/async_task.hpp>
#include <neolib/task/timer.hpp>
#include <neolib/app/object.hpp>
#include <neolib/ecs/i_ecs.hpp>
#include <neolib/ecs/component.hpp>
#include <neolib/ecs/system.hpp>

namespace neolib::ecs
{
    class ecs : public neolib::object<i_ecs>
    {
    public:
        define_declared_event(SystemsPaused, systems_paused)
        define_declared_event(SystemsResumed, systems_resumed)
        define_declared_event(EntityCreated, entity_created, entity_id)
        define_declared_event(EntityDestroyed, entity_destroyed, entity_id)
        define_declared_event(HandleUpdated, handle_updated, handle_id)
    private:
        typedef std::vector<handle_t> handles_t;
    public:
        ecs(ecs_flags aCreationFlags = ecs_flags::Default);
        ~ecs();
    public:
        neolib::recursive_spinlock& mutex() const override;
        neolib::thread_pool& thread_pool() const override;
    public:
        ecs_flags flags() const override;
        entity_id create_entity(const entity_archetype_id& aArchetypeId) override;
        void destroy_entity(entity_id aEntityId, bool aNotify = true) override;
    public:
        bool all_systems_paused() const override;
        void pause_all_systems() override;
        void resume_all_systems() override;
    public:
        const archetype_registry_t& archetypes() const override;
        archetype_registry_t& archetypes() override;
        const component_factories_t& component_factories() const override;
        component_factories_t& component_factories() override;
        const components_t& components() const override;
        components_t& components() override;
        const shared_component_factories_t& shared_component_factories() const override;
        shared_component_factories_t& shared_component_factories() override;
        const shared_components_t& shared_components() const override;
        shared_components_t& shared_components() override;
        const system_factories_t& system_factories() const override;
        system_factories_t& system_factories() override;
        const systems_t& systems() const override;
        systems_t& systems() override;
    public:
        const i_entity_archetype& archetype(entity_archetype_id aArchetypeId) const override;
        i_entity_archetype& archetype(entity_archetype_id aArchetypeId) override;
        bool component_instantiated(component_id aComponentId) const override;
        const i_component& component(component_id aComponentId) const override;
        i_component& component(component_id aComponentId) override;
        bool shared_component_instantiated(component_id aComponentId) const override;
        const i_shared_component& shared_component(component_id aComponentId) const override;
        i_shared_component& shared_component(component_id aComponentId) override;
        bool system_instantiated(system_id aSystemId) const override;
        const i_system& system(system_id aSystemId) const override;
        i_system& system(system_id aSystemId) override;
    public:
        entity_id next_entity_id() override;
        void free_entity_id(entity_id aId) override;
    public:
        bool archetype_registered(const i_entity_archetype& aArchetype) const override;
        void register_archetype(const i_entity_archetype& aArchetype) override;
        void register_archetype(std::shared_ptr<const i_entity_archetype> aArchetype) override;
        bool component_registered(component_id aComponentId) const override;
        void register_component(component_id aComponentId, component_factory aFactory) override;
        bool shared_component_registered(component_id aComponentId) const override;
        void register_shared_component(component_id aComponentId, shared_component_factory aFactory) override;
        bool system_registered(system_id aSystemId) const override;
        void register_system(system_id aSystemId, system_factory aFactory) override;
    public:
        handle_t to_handle(handle_id aId) const override;
        handle_id add_handle(const std::type_info& aTypeInfo, handle_t aHandle) override;
        handle_t update_handle(handle_id aId, const std::type_info& aTypeInfo, handle_t aHandle) override;
        handle_t release_handle(handle_id aId) override;
    private:
        handle_id next_handle_id();
        void free_handle_id(handle_id aId);
    public:
        using i_ecs::create_entity;
    public:
        using i_ecs::populate;
        using i_ecs::populate_shared;
        using i_ecs::component_instantiated;
        using i_ecs::component;
        using i_ecs::shared_component_instantiated;
        using i_ecs::shared_component;
        using i_ecs::system_instantiated;
        using i_ecs::system;
    public:
        using i_ecs::component_registered;
        using i_ecs::register_component;
        using i_ecs::shared_component_registered;
        using i_ecs::register_shared_component;
        using i_ecs::system_registered;
        using i_ecs::register_system;
    private:
        mutable neolib::recursive_spinlock iMutex;
        mutable std::optional<neolib::thread_pool> iThreadPool;
        ecs_flags iFlags;
        archetype_registry_t iArchetypeRegistry;
        component_factories_t iComponentFactories;
        mutable components_t iComponents;
        shared_component_factories_t iSharedComponentFactories;
        mutable shared_components_t iSharedComponents;
        system_factories_t iSystemFactories;
        mutable systems_t iSystems;
        entity_id iNextEntityId;
        std::vector<entity_id> iFreedEntityIds;
        handle_id iNextHandleId;
        std::vector<handle_id> iFreedHandleIds;
        handles_t iHandles;
        neolib::callback_timer iSystemTimer;
        std::atomic<bool> iSystemsPaused;
    };
}
