// timer.hpp
/*
 *  Copyright (c) 2007, 2020 Leigh Johnston.
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
#include <stdexcept>
#include <boost/bind.hpp>

#include <neolib/core/noncopyable.hpp>
#include <neolib/core/lifetime.hpp>
#include <neolib/task/event.hpp>
#include <neolib/task/i_timer_object.hpp>
#include <neolib/task/i_async_task.hpp>

namespace neolib
{
    class NEOLIB_EXPORT timer : private noncopyable, public lifetime<>
    {
        // types
    public:
        typedef std::chrono::duration<double> duration_type;
        // exceptions
    public:
        struct already_waiting : std::logic_error { already_waiting() : std::logic_error("neolib::timer::already_waiting") {} };
        struct already_enabled : std::logic_error { already_enabled() : std::logic_error("neolib::timer::already_enabled") {} };
        struct already_disabled : std::logic_error { already_disabled() : std::logic_error("neolib::timer::already_disabled") {} };
        // construction
    public:
        timer(i_async_task& aTask, const duration_type& aDuration_s, bool aInitialWait = true);
        timer(i_async_task& aTask, const i_lifetime& aContext, const duration_type& aDuration_s, bool aInitialWait = true);
        template <typename Context>
        timer(i_async_task& aTask, const Context& aContext, const duration_type& aDuration_s, bool aInitialWait = true) :
            timer{ aTask, dynamic_cast<const i_lifetime&>(aContext), aDuration_s, aInitialWait } {}
        timer(const timer& aOther);
        timer& operator=(const timer& aOther);
        virtual ~timer();
        // operations
    public:
        i_async_task& owner_task() const;
        void enable(bool aWait = true);
        void disable();
        bool enabled() const;
        bool disabled() const;
        void again();
        void again_if();
        void cancel();
        void reset();
        bool waiting() const;
        const duration_type& duration() const;
        void set_duration(const duration_type& aDuration_s, bool aEffectiveImmediately = false);
    public:
        void set_debug(bool aDebug);
        // implementation
    protected:
        void unsubscribe();
    private:
        i_timer_object& timer_object();
        void handler();
        virtual void ready() = 0;
        // attributes
    private:
        i_async_task& iTask;
        destroying_flag iTaskDestroying;
        destroyed_flag iTaskDestroyed;
        optional_destroyed_flag iContextDestroyed;
        sink iSink;
        ref_ptr<i_timer_object> iTimerObject;
        ref_ptr<i_timer_subscriber> iTimerSubscriber;
        duration_type iDuration_s;
        bool iEnabled;
        bool iWaiting;
        bool iInReady;
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
        bool iDebug = false;
#endif
    };

    class NEOLIB_EXPORT callback_timer : public timer
    {
    public:
        callback_timer(i_async_task& aTask, std::function<void(callback_timer&)> aCallback, const duration_type& aDuration_s, bool aInitialWait = true);
        callback_timer(i_async_task& aTask, const i_lifetime& aContext, std::function<void(callback_timer&)> aCallback, const duration_type& aDuration_s, bool aInitialWait = true);
        template <typename Context>
        callback_timer(i_async_task& aTask, const Context& aContext, std::function<void(callback_timer&)> aCallback, const duration_type& aDuration_s, bool aInitialWait = true) :
            callback_timer{ aTask, dynamic_cast<const i_lifetime&>(aContext), aCallback, aDuration_s, aInitialWait } {}
        ~callback_timer();
    private:
        virtual void ready();
    private:
        std::function<void(callback_timer&)> iCallback;
    };
}