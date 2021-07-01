// timer_object.hpp
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
#include <neolib/task/timer_object.hpp>

namespace neolib
{
    timer_object::timer_object(i_timer_service& aService) : 
        iService{ aService }, iDirtySubscriberList{ iMutex }
    {
    }

    timer_object::~timer_object()
    {
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
        if (iDebug)
            std::cerr << "timer_object::~timer_object()" << std::endl;
#endif
        std::unique_lock lock{ iMutex };
        for (auto& s : iSubscribers)
            s->detach();
        iService.remove_timer_object(*this);
    }

    void timer_object::expires_at(const std::chrono::steady_clock::time_point& aDeadline)
    {
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
        if (iDebug)
            std::cerr << "timer_object::expires_at(...)" << std::endl;
#endif
        iExpiryTime = aDeadline;
    }

    void timer_object::async_wait(i_timer_subscriber& aSubscriber)
    {
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
        if (iDebug)
            std::cerr << "timer_object::async_wait(...)" << std::endl;
#endif
        std::unique_lock lock{ iMutex };
        bool inserted = iSubscribers.insert(aSubscriber).second;
        if (inserted)
            iDirtySubscriberList.dirty();
    }

    void timer_object::unsubscribe(i_timer_subscriber& aSubscriber)
    {
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
        if (iDebug)
            std::cerr << "timer_object::unsubscribe(...)" << std::endl;
#endif
        std::unique_lock lock{ iMutex };
        auto existing = iSubscribers.find(aSubscriber);
        if (existing == iSubscribers.end())
            throw subscriber_not_found();
        (**existing).detach();
        iSubscribers.erase(existing);
        iDirtySubscriberList.dirty();
    }

    void timer_object::cancel()
    {
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
        if (iDebug)
            std::cerr << "timer_object::cancel()" << std::endl;
#endif
        iExpiryTime = std::nullopt;
    }

    bool timer_object::poll()
    {
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
        if (iDebug)
            std::cerr << "timer_object::poll()" << std::endl;
#endif
        if (!iExpiryTime || std::chrono::steady_clock::now() < *iExpiryTime)
            return false;
        iExpiryTime = std::nullopt;
        std::unique_lock lock{ iMutex };
        scoped_dirty sd{ iDirtySubscriberList };
        for (auto s = iSubscribers.begin(); s != iSubscribers.end();)
        {
            auto& subscriber = **s;
            subscriber.timer_expired(*this);
            if (iDirtySubscriberList.is_dirty())
            {
                iDirtySubscriberList.clean();
                s = iSubscribers.begin();
            }
            else
                ++s;
        }
        return true;
    }

    bool timer_object::debug() const
    {
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
        return iDebug;
#else
        return false;
#endif
    }

    void timer_object::set_debug(bool aDebug)
    {
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
        iDebug = aDebug;
#endif
    }
}
