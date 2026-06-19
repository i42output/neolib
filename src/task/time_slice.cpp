// time_slice.cpp
/*
 *  Copyright (c) 2026 Leigh Johnston.
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
#include <neolib/core/service.hpp>
#include <neolib/task/time_slice.hpp>

namespace neolib
{
    template<> i_time_slice& services::start_service<i_time_slice>()
    {
        static time_slice sTimeSlice;
        return sTimeSlice;
    }

    bool time_slice::active() const
    {
        return !slices().empty();
    }

    void time_slice::push(std::chrono::microseconds const& aDuration)
    {
        slices().push_back(std::make_unique<slice>(aDuration));
    }

    void time_slice::pop()
    {
        (void)current_slice();
        slices().pop_back();
    }

    void time_slice::enter(time_slice_task& aTask)
    {
        if (!active())
            return;
        auto const now = std::chrono::steady_clock::now();
        if (!tasks().empty())
        {
            current_task().slice += (now - current_task().entered.value_or(now));
            current_task().entered = std::nullopt;
        }
        tasks().push_back(&aTask);
        current_slice().tasks[&aTask];
        current_task().slice += (now - current_task().entered.value_or(now));
        current_task().entered = now;
    }

    void time_slice::leave()
    {
        if (!active())
            return;
        auto const now = std::chrono::steady_clock::now();
        current_task().slice += (now - current_task().entered.value_or(now));
        current_task().entered = std::nullopt;
        tasks().pop_back();
        if (!tasks().empty())
        {
            current_task().slice += (now - current_task().entered.value_or(now));
            current_task().entered = now;
        }
    }

    void time_slice::split(std::size_t aSlices)
    {
        if (!active())
            return;
        current_slice().splitCount = current_slice().splitCount.value_or(0u) + aSlices;
    }

    bool time_slice::expired() const
    {
        if (!active())
            return false;
        auto const now = std::chrono::steady_clock::now();
        auto const used = current_task().slice + (now - current_task().entered.value_or(now));
        auto const n = std::max<std::size_t>(1u, current_slice().splitCount.value_or(1u));
        return used > current_slice().duration / n;
    }

    time_slice::slice& time_slice::current_slice() const
    {
        if (!slices().empty())
            return *slices().back();
        throw std::logic_error("neolib::time_slice::current_slice");
    }

    time_slice::task& time_slice::current_task() const
    {
        if (!tasks().empty())
            return current_slice().tasks.at(tasks().back());
        throw std::logic_error("neolib::time_slice::current_task");
    }

    std::vector<std::unique_ptr<time_slice::slice>>& time_slice::slices()
    {
        thread_local std::vector<std::unique_ptr<slice>> tSlices;
        return tSlices;
    }

    std::vector<time_slice_task*>& time_slice::tasks()
    {
        thread_local std::vector<time_slice_task*> tTasks;
        return tTasks;
    }
}

