// event.hpp
/*
Transplanted from neogfx C++ GUI Library
Copyright (c) 2015-2018 Leigh Johnston.  All Rights Reserved.

This program is free software: you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <neolib/neolib.hpp>
#include <neolib/jar.hpp>

namespace neolib
{
    struct event_destroyed : std::logic_error { event_destroyed() : std::logic_error{ "neolib::event_destroyed" } {} };

    class i_event : public i_cookie_consumer
    {
    public:
        virtual ~i_event() {}
    public:
        virtual void release_control() = 0;
        virtual void handle_in_same_thread_as_emitter(cookie aHandleId) = 0;
    };

    class i_event_control
    {
    public:
        virtual ~i_event_control() {}
    public:
        virtual void add_ref() = 0;
        virtual void release() = 0;
        virtual bool valid() const = 0;
        virtual i_event& get() const = 0;
    public:
        virtual void reset() = 0;
    };

    class i_event_callback
    {
    public:
        virtual ~i_event_callback() {}
    public:
        virtual const i_event& event() const = 0;
        virtual void call() const = 0;
    };
}