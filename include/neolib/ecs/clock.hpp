// clock.hpp
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
#include <neolib/core/uuid.hpp>
#include <neolib/core/string.hpp>
#include <neolib/ecs/chrono.hpp>
#include <neolib/ecs/i_component_data.hpp>

namespace neolib::ecs
{
    struct clock
    {
        i64 time = 0ll;
        i64 timestep = chrono::to_flicks(0.01).count();
        primitives::scalar timestepGrowth = 1.75;
        i64 maximumTimestep = chrono::to_flicks(0.001).count() * 20;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x4c463f47, 0xede9, 0x4cc2, 0xb8f1, { 0x71, 0x2, 0x9e, 0x78, 0xb6, 0x1e } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Clock";
                return sName;
            }
            static uint32_t field_count()
            { 
                return 4; 
            }
            static component_data_field_type field_type(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                case 1:
                    return component_data_field_type::Int64;
                case 3:
                    return component_data_field_type::Float64;
                case 4:
                    return component_data_field_type::Int64;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] = 
                {
                    "Time",
                    "Timestep",
                    "Timestep Growth",
                    "Maximum Time Step",
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline step_time_interval to_step_time(clock const& aWorldClock, time_interval aTime)
    {
        return to_step_time(aTime, aWorldClock.timestep);
    }
}