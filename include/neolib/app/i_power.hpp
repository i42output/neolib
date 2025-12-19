// i_power.hpp
/*
 *  Copyright (c) 2020 Leigh Johnston.
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
#include <neolib/core/service.hpp>
#include <neolib/task/i_event.hpp>

namespace neolib
{
    enum class power_mode
    {
        Green,
        Normal,
        Turbo
    };

    class i_power : public i_service
    {
    public:
        declare_event(activity_registered)
        declare_event(green_mode_enabled)
        declare_event(green_mode_disabled)
        declare_event(green_mode_entered)
        declare_event(green_mode_left)
        declare_event(turbo_mode_enabled)
        declare_event(turbo_mode_disabled)
        declare_event(turbo_mode_entered)
        declare_event(turbo_mode_left)
    public:
        virtual ~i_power() = default;
    public:
        virtual power_mode active_mode() const = 0;
    public:
        virtual void register_activity() = 0;
        virtual std::chrono::seconds activity_timeout() const = 0;
        virtual void set_activity_timeout(std::chrono::seconds aTimeout) = 0;
    public:
        virtual bool is_green_mode_enabled() const = 0;
        virtual void enable_green_mode() = 0;
        virtual void disable_green_mode() = 0;
    public:
        bool green_mode_active() const
        {
            return active_mode() == power_mode::Green;
        }
        bool turbo_mode_active() const
        {
            return active_mode() == power_mode::Turbo;
        }
        bool is_turbo_mode_enabled() const
        {
            return !is_green_mode_enabled();
        }
        void enable_turbo_mode()
        {
            disable_green_mode();
        }
        void disable_turbo_mode()
        {
            enable_green_mode();
        }
    public:
        static uuid const& iid() { static uuid const sIid{ 0xed9d08d9, 0xc1fd, 0x4ccd, 0x8bab, { 0x9a, 0xd6, 0xe3, 0xaa, 0x71, 0x52 } }; return sIid; }
    };
}