// event.cpp
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

#include <neolib/neolib.hpp>
#include <neolib/async_thread.hpp>
#include <neolib/async_task.hpp>
#include <neolib/timer.hpp>
#include <neolib/event.hpp>

namespace neolib
{ 
    async_event_queue& async_event_queue::instance()
    {
        return get_instance(nullptr);
    }

    async_event_queue& async_event_queue::instance(async_task& aTask)
    {
        return get_instance(&aTask);
    }

    async_event_queue::async_event_queue(async_task& aTask) :
        iTimer
        {
            new callback_timer{
                aTask,
                [this](neolib::callback_timer& aTimer)
                {
                    if (terminated())
                        return;
                    std::scoped_lock lock{ iMutex };
                    publish_events();
                    if (!iEvents.empty() && !aTimer.waiting())
                        aTimer.again();
                }, 1, false}
        },
        iTerminated { false },
        iTaskDestroyed{ aTask }
    {
    }

    async_event_queue::~async_event_queue()
    {
        terminate();
    }

    async_event_queue& async_event_queue::get_instance(async_task* aTask)
    {
        thread_local bool tInstantiated = false;
        if (!tInstantiated && aTask == nullptr)
            throw async_event_queue_needs_a_task();
        if (tInstantiated && aTask != nullptr)
            throw async_event_queue_already_instantiated();
        tInstantiated = true;
        thread_local async_event_queue tLocalInstance{ *aTask };
        return tLocalInstance;
    }

    bool async_event_queue::exec()
    {
        return publish_events();
    }

    void async_event_queue::terminate()
    {
        std::scoped_lock lock{ iMutex };
        if (!terminated())
        {
            iTerminated = true;
            if (&instance() == this)
                publish_events();
            iEvents.clear();
        }
    }

    bool async_event_queue::terminated() const
    {
        return iTerminated;
    }

    void async_event_queue::unqueue(const i_event& aEvent)
    {
        remove(aEvent);
    }

    void async_event_queue::add(callback_ptr aCallback)
    {
        std::scoped_lock lock{ iMutex };
        if (terminated())
            throw async_event_queue_terminated();
        iEvents.push_back(std::move(aCallback));
        if (!iTimer->waiting())
            iTimer->again();
    }

    void async_event_queue::remove(const i_event& aEvent)
    {
        std::scoped_lock lock{ iMutex };
        for (auto& e : iEvents)
            if (&e->event() == &aEvent)
                e = nullptr;
    }

    bool async_event_queue::has(const i_event& aEvent) const
    {
        return std::find_if(iEvents.begin(), iEvents.end(), [&aEvent](auto const& e) { return &e->event() == &aEvent; }) != iEvents.end();
    }

    bool async_event_queue::publish_events()
    {
        bool didSome = false;
        std::scoped_lock lock{ iMutex };
        event_list_t toPublish;
        toPublish.swap(iEvents);
        for (auto& e : toPublish)
            if (e != nullptr)
            {
                didSome = true;
                e->call();
            }
        return didSome;
    }
}