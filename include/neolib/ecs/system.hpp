// system.hpp
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
#include <neolib/core/set.hpp>
#include <neolib/core/allocator.hpp>
#include <neolib/task/thread.hpp>
#include <neolib/app/i_power.hpp>
#include <neolib/ecs/i_system.hpp>
#include <neolib/ecs/entity_info.hpp>

namespace neolib::ecs
{
    class i_ecs;

    template <typename... ComponentData>
    class system : public i_system
    {
    private:
        typedef neolib::set<component_id, std::less<component_id>, neolib::fast_pool_allocator<component_id>> component_list;
    public:
        system(i_ecs& aEcs) :
            iEcs{ aEcs }, iComponents{ ComponentData::meta::id()... }, iPaused{ 0u }
        {
            (ecs().template component<ComponentData>(), ...);
        }
        system(const system& aOther) :
            iEcs{ aOther.iEcs }, iComponents{ aOther.iComponents }, iPaused{ 0u }
        {
            (ecs().template component<ComponentData>(), ...);
        }
        system(system&& aOther) :
            iEcs{ aOther.iEcs }, iComponents{ std::move(aOther.iComponents) }, iPaused{ 0u }
        {
            (ecs().template component<ComponentData>(), ...);
        }
        template <typename ComponentIdIter>
        system(i_ecs& aEcs, ComponentIdIter aFirstComponent, ComponentIdIter aLastComponent) :
            iEcs{ aEcs }, iComponents{ aFirstComponent, aLastComponent }, iPaused{ 0u }
        {
            (ecs().template component<ComponentData>(), ...);
            if (ecs().all_systems_paused())
                pause();
        }
    public:
        i_ecs& ecs() const override
        {
            return iEcs;
        }
    public:
        const i_set<component_id>& components() const override
        {
            return iComponents;
        }
        i_set<component_id>& components() override
        {
            return iComponents;
        }
    public:
        const i_component& component(component_id aComponentId) const override
        {
            return ecs().component(aComponentId);
        }
        i_component& component(component_id aComponentId) override
        {
            return ecs().component(aComponentId);
        }
    public:
        bool paused() const override
        {
            return iPaused != 0u;
        }
        void pause() override
        {
            ++iPaused;
        }
        void resume() override
        {
            --iPaused;
        }
        void terminate() override
        {
            // do nothing
        }
    protected:
        void yield()
        {
            if (service<neolib::i_power>().green_mode_active())
                neolib::thread::sleep(1);
        }
    private:
        i_ecs& iEcs;
        component_list iComponents;
        std::atomic<uint32_t> iPaused;
    };
}