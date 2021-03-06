// timer.cpp
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

#include <neolib/neolib.hpp>
#include <neolib/task/timer.hpp>

namespace neolib
{
    timer::timer(i_async_task& aTask, const duration_type& aDuration_s, bool aInitialWait) :
        iTask{ aTask },
        iTaskDestroying{ aTask },
        iTaskDestroyed{ aTask },
        iDuration_s{ aDuration_s },
        iEnabled{ true },
        iWaiting{ false },
        iInReady{ false }
    {
        if (aInitialWait)
            again();
    }

    timer::timer(i_async_task& aTask, const i_lifetime& aContext, const duration_type& aDuration_s, bool aInitialWait) :
        iTask{ aTask },
        iTaskDestroying{ aTask },
        iTaskDestroyed{ aTask },
        iContextDestroyed{ aContext },
        iDuration_s{ aDuration_s },
        iEnabled{ true },
        iWaiting{ false },
        iInReady{ false }
    {
        if (aInitialWait)
            again();
    }

    timer::timer(const timer& aOther) :
        iTask{ aOther.iTask },
        iTaskDestroying{ aOther.iTask },
        iTaskDestroyed{ aOther.iTask },
        iContextDestroyed{ aOther.iContextDestroyed },
        iDuration_s{ aOther.iDuration_s },
        iEnabled{ aOther.iEnabled },
        iWaiting{ false },
        iInReady{ false }
    {
        if (aOther.waiting())
            again();
    }
    
    timer& timer::operator=(const timer& aOther)
    {
        if (waiting())
            cancel();
        iDuration_s = aOther.iDuration_s;
        iEnabled = aOther.iEnabled;
        if (aOther.waiting())
            again();
        return *this;
    }
    
    timer::~timer()
    {
        cancel();
        unsubscribe();
    }

    i_async_task& timer::owner_task() const
    {
        return iTask;
    }

    void timer::enable(bool aWait)
    {
        if (iEnabled)
            throw already_enabled();
        iEnabled = true;
        if (aWait)
            again();
    }

    void timer::disable()
    {
        if (!iEnabled)
            throw already_disabled();
        if (waiting())
            cancel();
        iEnabled = false;
    }

    bool timer::enabled() const
    {
        return iEnabled;
    }

    bool timer::disabled() const
    {
        return !iEnabled;
    }

    void timer::again()
    {
        if (iTaskDestroying || iTaskDestroyed)
            return;
        if (disabled())
            enable(false);
        if (waiting())
            throw already_waiting();
        timer_object().expires_from_now(iDuration_s);
        if (iTimerSubscriber)
            timer_object().async_wait(*iTimerSubscriber);
        else
            iTimerSubscriber = timer_object().async_wait([this]() { handler(); });
        iWaiting = true;
    }

    void timer::again_if()
    {
        if (!waiting())
            again();
    }

    void timer::cancel()
    {
        if (!waiting())
            return;
        if (!iTaskDestroying && !iTaskDestroyed)
            timer_object().cancel();
    }

    void timer::reset()
    {
        cancel();
        again();
    }

    bool timer::waiting() const
    {
        return iWaiting;
    }

    const timer::duration_type& timer::duration() const
    {
        return iDuration_s;
    }

    void timer::set_duration(const duration_type& aDuration_s, bool aEffectiveImmediately)
    {
        iDuration_s = aDuration_s;
        if (aEffectiveImmediately && waiting())
        {
            destroyed_flag destroyed{ *this };
            cancel();
            if (destroyed)
                return;
            again();
        }
    }

    void timer::set_debug(bool aDebug)
    {
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
        iDebug = aDebug;
        if (iTimerObject)
            timer_object().set_debug(aDebug);
#endif
    }

    void timer::unsubscribe()
    {
        if (iTimerObject && iTimerSubscriber && iTimerSubscriber->attached())
        {
            timer_object().unsubscribe(*iTimerSubscriber);
        }
    }

    i_timer_object& timer::timer_object()
    {
        if (iTimerObject == nullptr)
        {
            iTimerObject = iTask.timer_service().create_timer_object();
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
            if (iDebug)
                timer_object().set_debug(iDebug);
#endif
            iSink += iTask.destroying([this]() { iTimerObject = nullptr; });
        }
        return *iTimerObject;
    }

    void timer::handler()
    {
        bool ok = enabled() && (iContextDestroyed == std::nullopt || !*iContextDestroyed);
        if (ok && iInReady && !waiting())
        {
            again();
            return;
        }
        iWaiting = false;
        if (ok)
        {
            try
            {
                iInReady = true;
                destroyed_flag destroyed{ *this };
                if (std::uncaught_exceptions() == 0)
                    ready();
                else
                    again();
                if (destroyed)
                    return;
                iInReady = false;
            }
            catch (...)
            {
                iInReady = false;
                throw;
            }
        }
    }

    callback_timer::callback_timer(i_async_task& aTask, std::function<void(callback_timer&)> aCallback, const duration_type& aDuration_s, bool aInitialWait) :
        timer{ aTask, aDuration_s, aInitialWait },
        iCallback{ aCallback }
    {
    }

    callback_timer::callback_timer(i_async_task& aTask, const i_lifetime& aContext, std::function<void(callback_timer&)> aCallback, const duration_type& aDuration_s, bool aInitialWait) :
        timer{ aTask, aContext, aDuration_s, aInitialWait },
        iCallback{ aCallback }
    {
    }

    callback_timer::~callback_timer()
    {
        cancel();
        unsubscribe();
    }

    void callback_timer::ready()
    {
        iCallback(*this);
    }
}