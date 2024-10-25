// i_system.hpp
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
#include <chrono>
#include <neolib/core/i_set.hpp>
#include <neolib/core/string.hpp>
#include <neolib/ecs/ecs_ids.hpp>
#include <neolib/ecs/i_component.hpp>

namespace neolib::ecs
{
    class i_ecs;

    class i_system
    {
    public:
        struct no_thread : std::logic_error { no_thread() : std::logic_error{ "neolib::ecs::i_system::no_thread" } {} };
        struct wrong_thread : std::logic_error { wrong_thread() : std::logic_error{ "neolib::ecs::i_system::wrong_thread" } {} };
        struct cannot_apply : std::logic_error { cannot_apply() : std::logic_error{ "neolib::ecs::i_system::cannot_apply" } {} };
    public:
        virtual ~i_system() = default;
    public:
        virtual i_ecs& ecs() const = 0;
    public:
        virtual const system_id& id() const = 0;
        virtual const neolib::i_string& name() const = 0;
    public:
        virtual const neolib::i_set<component_id>& components() const = 0;
        virtual neolib::i_set<component_id>& components() = 0;
    public:
        virtual const i_component& component(component_id aComponentId) const = 0;
        virtual const i_component& component(component_id aComponentId) = 0;
    public:
        virtual bool can_apply() const = 0;
        virtual bool apply() = 0;
        virtual bool paused() const = 0;
        virtual void pause() = 0;
        virtual void resume() = 0;
        virtual void terminate() = 0;
        virtual bool waiting() const = 0;
        virtual void wait() = 0;
        virtual void wait_for(primitives::scalar aDuration) = 0;
        virtual void signal() = 0;
    public:
        virtual void start_thread_if() = 0;
        virtual void start_thread() = 0;
    public:
        virtual bool debug() const = 0;
        virtual void set_debug(bool aDebug) = 0;
        virtual std::chrono::microseconds update_time(std::size_t aMetricsIndex = 0) const = 0;
    };
}