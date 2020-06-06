// time.cpp
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

#include <neolib/neolib.hpp>
#include <neolib/task/async_task.hpp>
#include <neolib/task/async_thread.hpp>
#include <neolib/ecs/chrono.hpp>
#include <neolib/ecs/ecs.hpp>
#include <neolib/ecs/time.hpp>
#include <neolib/ecs/clock.hpp>

namespace neolib::ecs
{
    class time::thread : public async_task, public async_thread
    {
    public:
        thread(time& aOwner) : async_task{ "neolib::ecs::time::thread" }, async_thread{ *this, "neolib::ecs::time::thread" }, iOwner{ aOwner }
        {
            start();
        }
    public:
        bool do_work(neolib::yield_type aYieldType = neolib::yield_type::NoYield) override
        {
            bool didWork = async_task::do_work(aYieldType);
            didWork = iOwner.apply() || didWork;
            iOwner.yield();
            return didWork;
        }
    private:
        time& iOwner;
    };

    time::time(ecs::i_ecs& aEcs) :
        system<>{ aEcs }
    {
        if (!ecs().shared_component_registered<clock>())
        {
            ecs().register_shared_component<clock>();
            ecs().populate_shared<clock>("World Clock", clock{});
        }
        iThread = std::make_unique<thread>(*this);
    }

    const system_id& time::id() const
    {
        return meta::id();
    }

    const i_string& time::name() const
    {
        return meta::name();
    }

    bool time::apply()
    {
        if (!ecs().component_instantiated<entity_info>())
            return false;
        else if (paused())
            return false;
        else if (!iThread->in()) // ignore ECS apply request (we have our own thread that does this)
            return false;

        scoped_component_lock<entity_info> lock{ ecs() };

        for (auto entity : ecs().component<entity_info>().entities())
        {
            auto const& info = ecs().component<entity_info>().entity_record(entity);
            if (info.destroyed)
                continue;
            if (info.lifeSpan && (world_time() - info.creationTime > *info.lifeSpan))
                ecs().async_destroy_entity(entity, false);
        }

        return true;
    }

    step_time time::system_time() const
    {
        auto systemTime = to_step_time(ecs(), chrono::to_seconds(std::chrono::duration_cast<chrono::flicks>(std::chrono::high_resolution_clock::now().time_since_epoch())));
        if (iSystemTimeOffset == std::nullopt)
            iSystemTimeOffset = systemTime;
        return systemTime - *iSystemTimeOffset;
    }

    step_time time::world_time() const
    {
        auto& worldClock = ecs().shared_component<clock>()[0];
        return worldClock.time;
    }
}