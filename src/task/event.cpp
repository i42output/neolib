// event.cpp
/*
 *  Copyright (c) 2018, 2020 Leigh Johnston.
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
#include <neolib/core/scoped.hpp>
#include <neolib/task/i_async_task.hpp>
#include <neolib/task/timer.hpp>
#include <neolib/task/event.hpp>

namespace neolib
{ 
    async_event_queue& async_event_queue::instance()
    {
        return get_instance(nullptr);
    }

    async_event_queue& async_event_queue::instance(i_async_task& aTask)
    {
        return get_instance(&aTask);
    }

    class queue_list
    {
    public:
        void add(async_event_queue& aQueue)
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            iQueues.push_back(&aQueue);
        }
        void remove(async_event_queue& aQueue)
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            auto existing = std::find(iQueues.begin(), iQueues.end(), &aQueue);
            if (existing != iQueues.end())
                iQueues.erase(existing);
        }
        void unqueue(const i_event& aEvent)
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            for (auto queue : iQueues)
                queue->unqueue(aEvent);
        }
    private:
        std::vector<async_event_queue*> iQueues;
    } sQueueList;

    void unqueue_event(const i_event& aEvent)
    {
        sQueueList.unqueue(aEvent);
    }

    async_event_queue::async_event_queue(i_async_task& aTask) :
        iTask{ aTask },
        iTimer
        {
            new callback_timer{
                aTask,
                [this](neolib::callback_timer& aTimer)
                {
                    if (terminated())
                        return;
                    publish_events();
                    if (!iEvents.empty() && !aTimer.waiting())
                        aTimer.again();
                }, std::chrono::milliseconds{1}, false}
        },
        iTerminated { false },
        iTaskDestroyed{ aTask },
        iPublishNestingLevel{ 0u },
        iNextTransaction{ 0ull }
    {
        sQueueList.add(*this);
    }

    async_event_queue::~async_event_queue()
    {
        sQueueList.remove(*this);
        terminate();
    }

    async_event_queue& async_event_queue::get_instance(i_async_task* aTask)
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
        if (alreadyInstantiated && aTask != &tLocalInstance.iTask)
            throw async_event_queue_already_instantiated();
        return tLocalInstance;
    }

    bool async_event_queue::exec()
    {
        return publish_events();
    }

    async_event_queue::transaction async_event_queue::enqueue(callback_ptr aCallback, bool aStatelessHandler, const optional_transaction& aTransaction)
    {
        if (aStatelessHandler)
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            for (auto& e : iEvents)
            {
                if (!e.callback->valid())
                    continue;
                if (*e.callback == *aCallback)
                    return {};
            }
        }
        return add(std::move(aCallback), aTransaction);
    }

    void async_event_queue::terminate()
    {
        std::scoped_lock<switchable_mutex> lock{ event_mutex() };
        if (!iTerminated)
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
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            aEvent.filter_added();
            iFilters.emplace(&aEvent, &aFilter);
        }
        void uninstall_event_filter(i_event_filter& aFilter, const i_event& aEvent) override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            aEvent.filter_removed();
            for (auto f = iFilters.equal_range(&aEvent).first; f != iFilters.equal_range(&aEvent).second; ++f)
                if (f->second == &aFilter)
                {
                    iFilters.erase(f);
                    return;
                }
        }
        void uninstall_event_filter(const i_event& aEvent) override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            aEvent.filters_removed();
            iFilters.erase(iFilters.equal_range(&aEvent).first, iFilters.equal_range(&aEvent).second);
        }
    public:
        void pre_filter_event(const i_event& aEvent) const override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            for (auto f = iFilters.equal_range(&aEvent).first; f != iFilters.equal_range(&aEvent).second; ++f)
                f->second->pre_filter_event(*f->first);
        }
        void filter_event(const i_event& aEvent) const override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            for (auto f = iFilters.equal_range(&aEvent).first; f != iFilters.equal_range(&aEvent).second; ++f)
                f->second->filter_event(*f->first);
        }
    private:
        std::unordered_multimap<const i_event*, i_event_filter*> iFilters;
    };

    i_event_filter_registry& async_event_queue::filter_registry()
    {
        static event_filter_registry sFilterRegistry;
        return sFilterRegistry;
    }

    bool async_event_queue::debug() const
    {
#if !defined(NDEBUG) || defined(DEBUG_EVENTS)
        return iDebug;
#else
        return false;
#endif
    }

    void async_event_queue::set_debug(bool aDebug)
    {
#if !defined(NDEBUG) || defined(DEBUG_EVENTS)
        iDebug = aDebug;
        if (iTimer)
            iTimer->set_debug(aDebug);
#endif
    }

    bool async_event_queue::terminated() const
    {
        return iTerminated || iTask.thread().finished();
    }

    void async_event_queue::unqueue(const i_event& aEvent)
    {
        remove(aEvent);
    }

    async_event_queue::transaction async_event_queue::add(callback_ptr aCallback, const optional_transaction& aTransaction)
    {
        std::scoped_lock<switchable_mutex> lock{ event_mutex() };
        if (terminated())
            return {};
        iEvents.push_back(event_list_entry{ aTransaction == std::nullopt ? ++iNextTransaction : *aTransaction, aCallback->event(), std::move(aCallback) });
        if (iTimer && !iTimer->waiting())
            iTimer->again();
        return iEvents.back().transaction;
    }

    void async_event_queue::remove(const i_event& aEvent)
    {
        std::scoped_lock<switchable_mutex> lock{ event_mutex() };
        for (auto e = iEvents.begin(); e != iEvents.end();)
        {
            if (e->callback != nullptr && &e->callback->event() == &aEvent)
                e = iEvents.erase(e);
            else
                ++e;
        }
    }

    bool async_event_queue::has(const i_event& aEvent) const
    {
        return std::find_if(iEvents.begin(), iEvents.end(), [&aEvent](auto const& e) { return &e.callback->event() == &aEvent; }) != iEvents.end();
    }

    bool async_event_queue::publish_events()
    {
        bool didSome = false;
        std::optional<std::scoped_lock<switchable_mutex>> lock{ event_mutex() };
        scoped_counter<std::atomic<uint32_t>> sc{ iPublishNestingLevel };
        if (iPublishNestingLevel > iPublishCache.size())
        {
            iPublishCache.resize(iPublishNestingLevel);
            iPublishCache[iPublishNestingLevel - 1u] = std::make_unique<event_list_t>();
        }
        auto& currentContext = *iPublishCache[iPublishNestingLevel - 1u];
        currentContext.clear();
        currentContext.swap(iEvents);
        optional_transaction currentTransaction;
        for (auto e = currentContext.begin(); !terminated() && e != currentContext.end(); ++e)
        {
            lock.reset();
            lock.emplace(event_mutex());
            if (e->destroyed || e->callback == nullptr)
                continue;
            auto const& ec = *e->callback;
            if (!ec.valid())
                continue;
            if (currentTransaction == std::nullopt || *currentTransaction != e->transaction)
            {
                currentTransaction = e->transaction;
                ec.event().push_context();
            }
            if (!ec.event().accepted())
            {
                if (ec.event().filtered())
                    filter_registry().filter_event(ec.event());
                if (!ec.event().accepted())
                {
                    didSome = true;
                    lock.reset();
                    ec.call();
                    while (!event_mutex().try_lock())
                    {
                        if (terminated())
                            return didSome;
                        std::this_thread::sleep_for(std::chrono::microseconds{ 0 });
                    }
                    lock.emplace(event_mutex());
                    event_mutex().unlock();
                }
            }
            if (std::next(e) == currentContext.end() || std::next(e)->transaction != *currentTransaction)
                ec.event().pop_context();
        }
        return didSome;
    }
}