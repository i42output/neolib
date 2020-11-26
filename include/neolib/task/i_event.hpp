// event.hpp
/*
 *  Copyright (c) 2015, 2018, 2020 Leigh Johnston.
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
#include <neolib/core/mutex.hpp>
#include <neolib/core/i_reference_counted.hpp>
#include <neolib/core/jar.hpp>
#include <neolib/core/i_map.hpp>

namespace neolib
{
    struct event_callable_expired : std::logic_error { event_callable_expired() : std::logic_error{ "neolib::event_callable_expired" } {} };
    struct event_destroyed : std::logic_error { event_destroyed() : std::logic_error{ "neolib::event_destroyed" } {} };
    struct event_queue_destroyed : std::logic_error { event_queue_destroyed() : std::logic_error{ "neolib::event_queue_destroyed" } {} };
    struct event_handler_not_found : std::logic_error { event_handler_not_found() : std::logic_error{ "neolib::event_handler_not_found" } {} };

    inline switchable_mutex& event_mutex()
    {
        static switchable_mutex sMutex;
        return sMutex;
    }

    namespace event_system
    {
        inline void set_single_threaded()
        {
            event_mutex().set_single_threaded();
        }

        inline void set_multi_threaded()
        {
            event_mutex().set_multi_threaded_spinlock();
        }
    }

    class i_event : public i_cookie_consumer
    {
    public:
        virtual ~i_event() = default;
    public:
        virtual void release_control() = 0;
        virtual void handle_in_same_thread_as_emitter(cookie aHandleId) = 0;
        virtual void handler_is_stateless(cookie aHandleId) = 0;
    public:
        virtual void pre_trigger() const = 0;
    public:
        virtual void push_context() const = 0;
        virtual void pop_context() const = 0;
    public:
        virtual bool accepted() const = 0;
        virtual void accept() const = 0;
        virtual void ignore() const = 0;
    public:
        virtual bool filtered() const = 0;
        virtual void filter_added() const = 0;
        virtual void filter_removed() const = 0;
        virtual void filters_removed() const = 0;
    };

    class i_event_control
    {
    public:
        virtual ~i_event_control() = default;
    public:
        virtual void add_ref() = 0;
        virtual void release() = 0;
        virtual bool valid() const = 0;
        virtual i_event& get() const = 0;
    public:
        virtual void reset() = 0;
    };

    class i_event_callback : public i_reference_counted
    {
    public:
        typedef i_event_callback abstract_type;
    public:
        virtual ~i_event_callback() = default;
    public:
        virtual bool operator==(const i_event_callback& aRhs) const = 0;
    public:
        virtual const i_event& event() const = 0;
        virtual const void* identity() const = 0;
        virtual bool valid() const = 0;
        virtual void call() const = 0;
    };

    class i_event_filter
    {
    public:
        virtual ~i_event_filter() = default;
    public:
        virtual void pre_filter_event(const i_event& aEvent) = 0;
        virtual void filter_event(const i_event& aEvent) = 0;
    };

    class i_event_filter_registry
    {
    public:
        virtual void install_event_filter(i_event_filter& aFilter, const i_event& aEvent) = 0;
        virtual void uninstall_event_filter(i_event_filter& aFilter, const i_event& aEvent) = 0;
        virtual void uninstall_event_filter(const i_event& aEvent) = 0;
    public:
        virtual void pre_filter_event(const i_event& aEvent) const = 0;
        virtual void filter_event(const i_event& aEvent) const = 0;
    };
}
