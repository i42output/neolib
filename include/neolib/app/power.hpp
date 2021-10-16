// power.hpp
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
#include <neolib/task/timer.hpp>
#include <neolib/task/event.hpp>
#include <neolib/app/i_power.hpp>

namespace neolib
{
    class NEOLIB_EXPORT power : public i_power
    {
    public:
        define_declared_event(ActivityRegistered, activity_registered)
        define_declared_event(GreenModeEnabled, green_mode_enabled)
        define_declared_event(GreenModeDisabled, green_mode_disabled)
        define_declared_event(GreenModeEntered, green_mode_entered)
        define_declared_event(GreenModeLeft, green_mode_left)
        define_declared_event(TurboModeEnabled, turbo_mode_enabled)
        define_declared_event(TurboModeDisabled, turbo_mode_disabled)
        define_declared_event(TurboModeEntered, turbo_mode_entered)
        define_declared_event(TurboModeLeft, turbo_mode_left)
    public:
        power();
    public:
        power_mode active_mode() const override;
    public:
        void register_activity() override;
        std::chrono::seconds activity_timeout() const override;
        void set_activity_timeout(std::chrono::seconds aTimeout) override;
    public:
        bool is_green_mode_enabled() const override;
        void enable_green_mode() override;
        void disable_green_mode() override;
    private:
        void set_active_mode(power_mode aMode);
    private:
        neolib::callback_timer iUpdater;
        power_mode iActiveMode;
        bool iGreenModeEnabled;
        std::chrono::seconds iActivityTimeout;
        std::chrono::steady_clock::time_point iLastActivityTime;
    };
}