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
#include <neolib/scoped.hpp>
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
        iTask{ &aTask },
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
        iTaskDestroyed{ aTask },
        iPublishNestingLevel{ 0u },
        iNextTransaction{ 0ull }
    {
    }

    async_event_queue::~async_event_queue()
    {
        terminate();
    }

    async_event_queue& async_event_queue::get_instance(async_task* aTask)
    {
        thread_local bool tInstantiated = false;
        bool const alreadyInstantiated = tInstantiated;
        if (!alreadyInstantiated && aTask == nullptr)
            throw async_event_queue_needs_a_task();
        thread_local async_event_queue tLocalInstance{ *aTask };
        tInstantiated = true;
        if (tLocalInstance.iTaskDestroyed)
            throw async_event_queue_needs_a_task();
        if (aTask == nullptr)
            return tLocalInstance;
        if (alreadyInstantiated && aTask != tLocalInstance.iTask)
            throw async_event_queue_already_instantiated();
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
            iTimer = nullptr;
            iEvents.clear();
        }
    }

    class event_filter_registry : public i_event_filter_registry
    {
    public:
        void install_event_filter(i_event_filter& aFilter, const i_event& aEvent) override
        {
            std::scoped_lock lock{ iMutex };
            aEvent.filter_added();
            iFilters.emplace(&aEvent, &aFilter);
        }
        void uninstall_event_filter(i_event_filter& aFilter, const i_event& aEvent) override
        {
            std::scoped_lock lock{ iMutex };
            aEvent.filter_removed();
            for (auto f = iFilters.lower_bound(&aEvent); f != iFilters.upper_bound(&aEvent); ++f)
                if (f->second == &aFilter)
                {
                    iFilters.erase(f);
                    return;
                }
        }
        void uninstall_event_filter(const i_event& aEvent) override
        {
            std::scoped_lock lock{ iMutex };
            aEvent.filters_removed();
            iFilters.erase(iFilters.lower_bound(&aEvent), iFilters.upper_bound(&aEvent));
        }
    public:
        void pre_filter_event(const i_event& aEvent) const override
        {
            std::scoped_lock lock{ iMutex };
            for (auto f = iFilters.lower_bound(&aEvent); f != iFilters.upper_bound(&aEvent); ++f)
                f->second->pre_filter_event(*f->first);
        }
        void filter_event(const i_event& aEvent) const override
        {
            std::scoped_lock lock{ iMutex };
            for (auto f = iFilters.lower_bound(&aEvent); f != iFilters.upper_bound(&aEvent); ++f)
                f->second->filter_event(*f->first);
        }
    private:
        mutable std::recursive_mutex iMutex;
        std::unordered_multimap<const i_event*, i_event_filter*> iFilters;
    };

    i_event_filter_registry& async_event_queue::filter_registry()
    {
        static event_filter_registry sFilterRegistry;
        return sFilterRegistry;
    }

    bool async_event_queue::terminated() const
    {
        return iTerminated;
    }

    void async_event_queue::unqueue(const i_event& aEvent)
    {
        remove(aEvent);
    }

    async_event_queue::transaction async_event_queue::add(callback_ptr aCallback, const optional_transaction& aTransaction)
    {
        std::scoped_lock lock{ iMutex };
        if (terminated())
            throw async_event_queue_terminated();
        iEvents.push_back(std::make_pair(aTransaction == std::nullopt ? ++iNextTransaction : *aTransaction, std::move(aCallback)));
        if (!iTimer->waiting())
            iTimer->again();
        return iEvents.back().first;
    }

    void async_event_queue::remove(const i_event& aEvent)
    {
        std::scoped_lock lock{ iMutex };
        for (auto& e : iEvents)
            if (e.second != nullptr && &e.second->event() == &aEvent)
                e.second = nullptr;
    }

    bool async_event_queue::has(const i_event& aEvent) const
    {
        return std::find_if(iEvents.begin(), iEvents.end(), [&aEvent](auto const& e) { return &e.second->event() == &aEvent; }) != iEvents.end();
    }

    bool async_event_queue::publish_events()
    {
        bool didSome = false;
        std::optional<std::scoped_lock<std::recursive_mutex>> lock{ iMutex };
        scoped_counter<std::atomic<uint32_t>> sc{ iPublishNestingLevel };
        if (iPublishNestingLevel > iPublishCache.size())
        {
            iPublishCache.resize(iPublishNestingLevel);
            iPublishCache[iPublishNestingLevel - 1u] = std::make_unique<event_list_t>();
        }
        auto& currentContext = *iPublishCache[iPublishNestingLevel - 1u];
        currentContext.clear();
        currentContext.swap(iEvents);
        lock = std::nullopt;
        optional_transaction currentTransaction;
        for (auto e = currentContext.begin(); e != currentContext.end(); ++e)
        {
            if (e->second == nullptr)
                continue;
            auto const& ec = *e->second;
            if (currentTransaction == std::nullopt || *currentTransaction != e->first)
            {
                currentTransaction = e->first;
                ec.event().push_context();
            }
            if (!ec.event().accepted())
            {
                if (ec.event().filtered())
                    filter_registry().filter_event(ec.event());
                if (!ec.event().accepted())
                {
                    didSome = true;
                    ec.call();
                }
            }
            if (std::next(e) == currentContext.end() || std::next(e)->first != *currentTransaction)
                ec.event().pop_context();
        }
        return didSome;
    }
}