// i_timer_object.hpp
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
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
#include <iostream>
#endif
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/i_lifetime.hpp>

namespace neolib
{
    class i_timer_object;

    class i_timer_subscriber : public i_reference_counted, public i_lifetime
    {
    public:
        typedef i_timer_subscriber abstract_type;
    public:
        virtual ~i_timer_subscriber() = default;
    public:
        virtual void timer_expired(i_timer_object& aTimerObject) = 0;
        virtual bool attached() const = 0;
        virtual void detach() = 0;
    public:
        friend bool operator<(const i_timer_subscriber& aLeft, const i_timer_subscriber& aRight)
        {
            return &aLeft < &aRight;
        }
    };

    class i_timer_object : public i_reference_counted, public i_lifetime
    {
    public:
        typedef i_timer_object abstract_type;
    public:
        struct subscriber_not_found : std::logic_error { subscriber_not_found() : std::logic_error{ "i_timer_object::subscriber_not_found" } {} };
    private:
        class subscriber_wrapper : public lifetime<reference_counted<i_timer_subscriber>>
        {
        public:
            subscriber_wrapper(i_timer_object& aTimerObject, std::function<void()> aCallback) :
                iTimerObject{ &aTimerObject }, iCallback { aCallback }
            {
            }
            ~subscriber_wrapper()
            {
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
                if (iTimerObject != nullptr)
                {
                    if (iTimerObject->debug())
                    {
                        std::cerr << "i_timer_object::subscriber_wrapper::~subscriber_wrapper()" << std::endl;
                    }
                }
#endif
            }
        protected:
            void timer_expired(i_timer_object&) override
            {
#if !defined(NDEBUG) || defined(DEBUG_TIMER_OBJECTS)
                if (attached())
                {
                    if (iTimerObject->debug())
                    {
                        std::cerr << "i_timer_object::subscriber_wrapper::timer_expired(...)" << std::endl;
                    }
                }
#endif
                iCallback();
            }
            bool attached() const override
            {
                return iTimerObject != nullptr;
            }
            void detach() override
            {
                if (attached())
                {
                    if (iTimerObject->debug())
                    {
                        std::cerr << "i_timer_object::subscriber_wrapper::detach()" << std::endl;
                    }
                    iTimerObject = nullptr;
                }
            }
        private:
            i_timer_object* iTimerObject;
            std::function<void()> iCallback;
        };
    public:
        virtual ~i_timer_object() = default;
    public:
        virtual void expires_at(const std::chrono::steady_clock::time_point& aDeadline) = 0;
        virtual void async_wait(i_timer_subscriber& aSubscriber) = 0;
        virtual void unsubscribe(i_timer_subscriber& aSubscriber) = 0;
        virtual void cancel() = 0;
    public:
        virtual bool poll() = 0;
    public:
        virtual bool debug() const = 0;
        virtual void set_debug(bool aDebug) = 0;
    public:
        template <typename Duration>
        void expires_from_now(const Duration& aDuration)
        {
            expires_at(std::chrono::steady_clock::now() + std::chrono::duration_cast<std::chrono::nanoseconds>(aDuration));
        }
        i_timer_subscriber& async_wait(const std::function<void()>& aSubscriber)
        {
            auto subscriber = make_ref<subscriber_wrapper>(*this, [aSubscriber]() { aSubscriber(); });
            async_wait(*subscriber);
            return *subscriber;
        }
        i_timer_subscriber& async_wait(const std::function<void(i_timer_object&)>& aSubscriber)
        {
            auto subscriber = make_ref<subscriber_wrapper>(*this, [this, aSubscriber]() { aSubscriber(*this); });
            async_wait(*subscriber);
            return *subscriber;
        }
    };
}
