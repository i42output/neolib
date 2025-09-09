// time.hpp
/*
 *  Copyright (c) 2025 Leigh Johnston.
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
#include <format>
#include <sstream>
#include <string>
#include <stdexcept>

namespace neolib 
{
    inline std::string to_iso8601(std::chrono::utc_clock::time_point const& aTimePoint, bool aFractionalSeconds = true) 
    {
        if (aFractionalSeconds)
        {
            return std::format("{:%Y-%m-%dT%H:%M:%S}Z", aTimePoint);
        }
        else
        {
            auto const nowSeconds = std::chrono::floor<std::chrono::seconds>(aTimePoint);
            auto const nowSysSeconds = std::chrono::clock_cast<std::chrono::system_clock>(nowSeconds);
            auto const sysDays = std::chrono::floor<std::chrono::days>(nowSysSeconds);
            auto const ymd = std::chrono::year_month_day{ sysDays };
            auto const utcDays = std::chrono::floor<std::chrono::days>(nowSeconds);
            auto const hms = std::chrono::hh_mm_ss{ nowSeconds - utcDays };

            return std::format("{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z",
                static_cast<int>(ymd.year()),
                static_cast<unsigned>(ymd.month()),
                static_cast<unsigned>(ymd.day()),
                hms.hours().count(),
                hms.minutes().count(),
                hms.seconds().count());
        }
    }

    inline std::chrono::utc_clock::time_point from_iso8601(std::string const& aDateTime) 
    {
        std::chrono::utc_clock::time_point result;

        bool const zuluTime = (aDateTime.back() == 'Z');

        if (!(std::istringstream{ aDateTime } >> std::chrono::parse(zuluTime ? "%FT%TZ" : "%FT%T%z", result)))
            throw std::runtime_error("Failed to parse ISO 8601 string");

        return result;
    }
}
