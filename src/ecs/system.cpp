// system.cpp
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
#include <neolib/task/thread.hpp>
#include <neolib/app/i_power.hpp>
#include <neolib/ecs/ecs.hpp>
#include <neolib/ecs/system.hpp>

namespace neolib::ecs
{
    system::system(i_ecs& aEcs) :
        iEcs{ aEcs }, iPaused{ 0u }
    {
    }

    system::system(const system& aOther) :
        iEcs{ aOther.iEcs }, iComponents{ aOther.iComponents }, iPaused{ 0u }
    {
    }

    system::system(system&& aOther) :
        iEcs{ aOther.iEcs }, iComponents{ std::move(aOther.iComponents) }, iPaused{ 0u }
    {
    }

    i_ecs& system::ecs() const
    {
        return iEcs;
    }

    const i_set<component_id>& system::components() const
    {
        return iComponents;
    }

    i_set<component_id>& system::components()
    {
        return iComponents;
    }

    const i_component& system::component(component_id aComponentId) const
    {
        return ecs().component(aComponentId);
    }

    i_component& system::component(component_id aComponentId)
    {
        return ecs().component(aComponentId);
    }

    bool system::paused() const
    {
        return iPaused != 0u;
    }

    void system::pause()
    {
        ++iPaused;
    }

    void system::resume()
    {
        --iPaused;
    }

    void system::terminate()
    {
        // do nothing
    }

    void system::yield()
    {
        if (service<neolib::i_power>().green_mode_active())
            neolib::thread::sleep(1);
    }
}
