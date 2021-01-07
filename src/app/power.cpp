// power.cpp
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

#include <neolib/neolib.hpp>
#include <neolib/task/async_task.hpp>
#include <neolib/app/power.hpp>

namespace neolib
{
    template<> i_power& services::start_service<i_power>()
    {
        static power sPower;
        return sPower;
    }

    power::power() :
        iUpdater{ service<i_async_task>(), [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            if (is_green_mode_enabled() && std::chrono::steady_clock::now() - iLastActivityTime >= activity_timeout())
                set_active_mode(power_mode::Green);
        }, std::chrono::milliseconds{1000} },
        iActiveMode{ power_mode::Normal },
        iActivityTimeout{ 5 },
        iGreenModeEnabled{ true },
        iLastActivityTime{ std::chrono::steady_clock::now() }
    {
    }

    power_mode power::active_mode() const
    {
        return iActiveMode;
    }

    void power::register_activity()
    {
        iLastActivityTime = std::chrono::steady_clock::now();
        ActivityRegistered.trigger();
        if (green_mode_active())
            set_active_mode(power_mode::Normal);
    }

    std::chrono::seconds power::activity_timeout() const
    {
        return iActivityTimeout;
    }

    void power::set_activity_timeout(std::chrono::seconds aTimeout)
    {
        iActivityTimeout = aTimeout;
    }

    bool power::is_green_mode_enabled() const
    {
        return iGreenModeEnabled;
    }

    void power::enable_green_mode()
    {
        if (!iGreenModeEnabled)
        {
            iGreenModeEnabled = true;
            GreenModeEnabled.trigger();
            TurboModeDisabled.trigger();
            set_active_mode(power_mode::Normal);
        }
    }

    void power::disable_green_mode()
    {
        if (iGreenModeEnabled)
        {
            iGreenModeEnabled = false;
            GreenModeDisabled.trigger();
            TurboModeEnabled.trigger();
            set_active_mode(power_mode::Turbo);
        }
    }

    void power::set_active_mode(power_mode aMode)
    {
        if (iActiveMode != aMode)
        {
            auto const previousMode = iActiveMode;
            iActiveMode = aMode;
            switch (active_mode())
            {
            case power_mode::Green:
                GreenModeEntered.trigger();
                break;
            case power_mode::Normal:
                break;
            case power_mode::Turbo:
                TurboModeEntered.trigger();
                break;
            }
            switch (previousMode)
            {
            case power_mode::Green:
                GreenModeLeft.trigger();
                break;
            case power_mode::Normal:
                break;
            case power_mode::Turbo:
                TurboModeLeft.trigger();
                break;
            }
        }
    }
}