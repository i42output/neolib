// plugin_event.hpp
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
#include <neolib/task/event.hpp>
#include <neolib/plugin/i_plugin_event.hpp>

namespace neolib
{
    namespace plugin_events
    {
        using neolib::async_event_queue;
        using neolib::event_trigger_type;

        template <typename... Args>
        class event : public i_event<Args...>, public neolib::event<Args...>
        {
            typedef neolib::event<Args...> base_type;
        public:
            using typename i_event<Args...>::abstract_callback;
        public:
            typedef base_type event_type;
            typedef i_event<Args...> abstract_plugin_event_type;
        public:
            using event_type::event_type;
        public:
            using event_type::subscribe;
            using event_type::operator();
            using event_type::unsubscribe;
        public:
            const neolib::i_event& raw_event() const override
            {
                return *this;
            }
            neolib::i_event& raw_event() override
            {
                return *this;
            }
        public:
            void pre_trigger() const override
            {
                event_type::pre_trigger();
            }
        public:
            bool trigger(Args... aArguments) const override
            {
                return event_type::trigger(aArguments...);
            }
            bool sync_trigger(Args... aArguments) const override
            {
                return event_type::sync_trigger(aArguments...);
            }
            void async_trigger(Args... aArguments) const override
            {
                event_type::async_trigger(rvalue_cast<Args>(aArguments)...);
            }
            bool accepted() const override
            {
                return event_type::accepted();
            }
            void accept() const override
            {
                event_type::accept();
            }
            void ignore() const override
            {
                event_type::ignore();
            }
        private:
            event_handle do_subscribe(const abstract_callback& aCallback, const void* aUniqueId = nullptr) const override
            {
                auto callback = aCallback.clone();
                return event_type::subscribe(
                        [callback](Args&& ... aArguments)
                        {
                            (*callback)(std::forward<Args>(aArguments)...);
                        },
                        aUniqueId);
            }
            void do_unsubscribe(event_handle aHandle) const override
            {
                return event_type::unsubscribe(aHandle);
            }
            void do_unsubscribe(const void* aUniqueId) const override
            {
                return event_type::unsubscribe(aUniqueId);
            }
        };
    }
}