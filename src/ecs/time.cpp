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
#include <neolib/ecs/chrono.hpp>
#include <neolib/ecs/ecs.hpp>
#include <neolib/ecs/time.hpp>
#include <neolib/ecs/clock.hpp>

namespace neolib::ecs
{
    time::time(ecs::i_ecs& aEcs) :
        system{ aEcs }
    {
        if (!ecs().shared_component_registered<clock>())
        {
            ecs().register_shared_component<clock>();
            ecs().populate_shared<clock>("World Clock", clock{});
        }
        start_thread_if();
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
        if (!can_apply())
            throw cannot_apply();
        if (!ecs().component_instantiated<entity_life_span>())
            return false;
        if (paused())
            return false;

        auto waitDuration = std::numeric_limits<i64>::max();

        {
            scoped_component_lock<entity_info, entity_life_span> lock{ ecs() };
            auto const& infos = ecs().component<entity_info>();
            auto const& lifeSpans = ecs().component<entity_life_span>();

            for (auto entity : lifeSpans.entities())
            {
                auto const& info = infos.entity_record(entity);
                if (info.destroyed)
                    continue;
                auto const& lifeSpan = lifeSpans.entity_record(entity);
                auto const age = world_time() - info.creationTime;
                waitDuration = std::min(waitDuration, lifeSpan.lifeSpan - age);
                if (age > lifeSpan.lifeSpan)
                    ecs().async_destroy_entity(entity, false);
            }
        }

        if (have_thread() && get_thread().in())
            wait_for(std::max(1.0, from_step_time(waitDuration)));

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