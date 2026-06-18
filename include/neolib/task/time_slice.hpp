// time_slice.hpp
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

namespace neolib
{
    enum class time_slice_task {};

    class i_time_slice : public i_service
    {
    public:
        virtual bool active() const = 0;
    public:
        virtual void push(std::chrono::microseconds const& aDuration) = 0;
        virtual void pop() = 0;
    public:
        virtual void enter(time_slice_task& aTask) = 0;
        virtual void leave() = 0;
    public:
        virtual void split(std::size_t aSlices) = 0;
        virtual bool expired() const = 0;
    public:
        template <typename Rep, typename Period>
        void push(std::chrono::duration<Rep, Period> duration)
        {
            push(std::chrono::duration_cast<std::chrono::microseconds>(duration));
        }
    public:
        static uuid const& iid() { static uuid const sIid{ 0x24684991, 0xe4ae, 0x44b9, 0xb691, { 0x7b, 0x2e, 0x77, 0xcd, 0xf3, 0xa7 } }; return sIid; }
    };

    class time_slice : public i_time_slice
    {
    private:
        struct task
        {
            std::optional<std::chrono::steady_clock::time_point> entered;
            std::chrono::steady_clock::duration slice = {};
        };
        struct slice
        {
            std::chrono::microseconds duration = {};
            std::optional<std::size_t> splitCount;
            std::unordered_map<time_slice_task*, task> tasks;
        };
    public:
        bool active() const final
        {
            return !iSlices.empty();
        }
    public:
        using i_time_slice::push;
        void push(std::chrono::microseconds const& aDuration) final
        {
            iSlices.push_back(std::make_unique<slice>(aDuration));
        }
        void pop() final
        {
            (void)current_slice();
            iSlices.pop_back();
        }
    public:
        void enter(time_slice_task& aTask) final
        {
            if (!active())
                return;
            if (!iTasks.empty())
            {
                current_task().slice += (std::chrono::steady_clock::now() - current_task().entered.value());
                current_task().entered = std::nullopt;
            }
            iTasks.push_back(&aTask);
            current_slice().tasks[&aTask];
            current_task().entered = std::chrono::steady_clock::now();
        }
        void leave() final
        {
            if (!active())
                return;
            current_task().slice += (std::chrono::steady_clock::now() - current_task().entered.value());
            current_task().entered = std::nullopt;
            iTasks.pop_back();
            if (!iTasks.empty())
                current_task().entered = std::chrono::steady_clock::now();
        }
    public:
        void split(std::size_t aSlices) final
        {
            if (!active())
                return;
            current_slice().splitCount = current_slice().splitCount.value_or(0u) + aSlices;
        }
        bool expired() const final
        {
            if (!active())
                return false;
            auto used = current_task().slice;
            if (current_task().entered)
                used += std::chrono::steady_clock::now() - *current_task().entered;
            auto const n = std::max<std::size_t>(1u, current_slice().splitCount.value_or(1u));
            return used > current_slice().duration / n;
        }
    private:
        slice& current_slice() const
        {
            if (!iSlices.empty())
                return *iSlices.back();
            throw std::logic_error("neolib::time_slice::current_slice");
        }
        task& current_task() const
        {
            if (!iTasks.empty())
                return current_slice().tasks.at(iTasks.back());
            throw std::logic_error("neolib::time_slice::current_task");
        }
    private:
        std::vector<std::unique_ptr<slice>> iSlices;
        std::vector<time_slice_task*> iTasks;
    };

    class scoped_time_slice
    {
    public:
        template <typename Rep, typename Period>
        explicit scoped_time_slice(std::chrono::duration<Rep, Period> aDuration)
        {
            service<i_time_slice>().push(aDuration);
        }
        ~scoped_time_slice()
        {
            service<i_time_slice>().pop();
        }
    };

    template <typename UniqueTag = void>
    class scoped_time_slice_task
    {
    public:
        scoped_time_slice_task()
        {
            static_assert(std::is_enum_v<UniqueTag> || std::is_class_v<UniqueTag>, "Tag must be an enum or a class");
            static time_slice_task sTask = {};
            service<i_time_slice>().enter(sTask);
        }
        scoped_time_slice_task(time_slice_task& aTask)
        {
            service<i_time_slice>().enter(aTask);
        }
        ~scoped_time_slice_task()
        {
            service<i_time_slice>().leave();
        }
    };
}
