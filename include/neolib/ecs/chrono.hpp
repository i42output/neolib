// chrono.hpp
/*
 *  Copyright (c) 2015, 2020 Leigh Johnston.
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
#include <neolib/core/numerical.hpp>
#include <neolib/ecs/3rdparty/facebook/flicks.h>

namespace neolib::ecs
{
    namespace chrono
    {
        using namespace facebook::util;

        inline constexpr double to_milliseconds(const flicks ns)
        {
            return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(ns)).count());
        }
    }

    typedef scalar time_interval;
    typedef std::optional<time_interval> optional_time_interval;
    typedef int64_t step_time_interval;
    typedef std::optional<step_time_interval> optional_step_time_interval;
    typedef step_time_interval step_time;
    typedef std::optional<step_time> optional_step_time;

    inline step_time_interval to_step_time(time_interval aTime, step_time_interval aStepInterval)
    {
        auto fs = chrono::to_flicks(aTime).count();
        return fs - (fs % aStepInterval);
    }

    inline step_time_interval to_step_time(optional_time_interval& aTime, step_time_interval aStepInterval)
    {
        if (aTime)
            return to_step_time(*aTime, aStepInterval);
        else
            return 0;
    }

    inline time_interval from_step_time(step_time_interval aStepTime)
    {
        return chrono::to_seconds(chrono::flicks{ aStepTime });
    }
}