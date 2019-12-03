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
#include <vector>
#include <optional>
#include <mutex>

#include <neolib/lifetime.hpp>
#include <neolib/jar.hpp>
#include <neolib/async_task.hpp>
#include <neolib/timer.hpp>

namespace neolib
{
    struct event_destroyed : std::logic_error { event_destroyed() : std::logic_error{ "neolib::event_destroyed" } {} };

    class sink;

    class i_event : public i_cookie_consumer
    {
    public:
        virtual ~i_event() {}
    public:
        virtual void handle_in_same_thread_as_emitter(cookie aHandleId) = 0;
    };

    struct event_handle
    {
        i_event& event;
        cookie id;

        event_handle& operator~()
        {
            event.handle_in_same_thread_as_emitter(id);
            return *this;
        }
    };

    class i_event_callback
    {
    public:
        virtual ~i_event_callback() {}
    public:
        virtual const i_event& event() const = 0;
        virtual void call() const = 0;
    };

    template <typename... Args>
    class event_callback : public i_event_callback
    {
        template <typename...>
        friend class event;
        friend class async_event_queue;
    private:
        typedef std::function<void(Args...)> function_type;
        typedef std::shared_ptr<function_type> handler_ptr;
        typedef std::tuple<Args...> argument_pack;
    public:
        event_callback(const i_event& aEvent, handler_ptr aHandler, Args... aArguments) :
            iEvent{ aEvent }, iHandler{ aHandler }, iArguments{ aArguments... }
        {
        }
    public:
        const i_event& event() const override
        {
            return iEvent;
        }
        void call() const override
        {
            std::apply(*iHandler, iArguments);
        }
    private:
        const i_event& iEvent;
        handler_ptr iHandler;
        argument_pack iArguments;
    };

    class async_event_queue : public lifetime
    {
        template <typename...>
        friend class event;
    public:
        struct async_event_queue_already_instantiated : std::logic_error { async_event_queue_already_instantiated() : std::logic_error("neogfx::async_event_queue::async_event_queue_already_instantiated") {} };
        struct event_not_found : std::logic_error { event_not_found() : std::logic_error("neogfx::async_event_queue::event_not_found") {} };
    private:
        typedef std::unique_ptr<i_event_callback> callback_ptr;
        typedef std::deque<callback_ptr> event_list_t;
    public:
        static async_event_queue& instance();
        static async_event_queue& instance(neolib::async_task& aTask);
        ~async_event_queue();
    private:
        async_event_queue(neolib::async_task& aTask);
        async_event_queue(std::shared_ptr<async_task> aTask);
        static async_event_queue& get_instance(neolib::async_task* aTask);
    public:
        bool exec();
        void enqueue(callback_ptr aCallback)
        {
            add(std::move(aCallback));
        }
        void unqueue(const i_event& aEvent);
        void terminate();
    private:
        void add(callback_ptr aCallback);
        void remove(const i_event& aEvent);
        bool has(const i_event& aEvent) const;
        bool publish_events();
    private:
        std::shared_ptr<async_task> iTask;
        neolib::callback_timer iTimer;
        event_list_t iEvents;
        std::recursive_mutex iMutex;
    };

    enum class event_trigger_type
    {
        Default,
        Synchronous,
        SynchronousDontQueue,
        Asynchronous,
        AsynchronousDontQueue
    };

    template <typename... Args>
    class event : public i_event, protected lifetime
    {
        typedef event<Args...> self_type;
        friend class sink;
        friend class async_event_queue;
    private:
        typedef typename event_callback<Args...>::function_type function_type;
        typedef typename event_callback<Args...>::handler_ptr handler_ptr;
        struct handler : i_jar_item
        {
            neolib::cookie id;
            async_event_queue* queue;
            uint32_t referenceCount;
            const void* clientId;
            handler_ptr callback;
            bool handleInSameThreadAsEmitter;

            neolib::cookie cookie() const override { return id; }

            handler(
                neolib::cookie id, 
                async_event_queue& queue, 
                uint32_t referenceCount, 
                const void* clientId, 
                handler_ptr callback,
                bool handleInSameThreadAsEmitter = true) : 
                id{ id },
                queue{ &queue },
                referenceCount{ referenceCount },
                clientId{ clientId },
                callback{ callback },
                handleInSameThreadAsEmitter{ handleInSameThreadAsEmitter }
            {}
        };
        typedef neolib::jar<handler> handler_list_t;
        struct context
        {
            bool accepted;
        };
        typedef std::vector<context> context_list_t;
        struct instance_data
        {
            event_trigger_type triggerType = event_trigger_type::Default;
            handler_list_t handlers;
            context_list_t contexts;
        };
    public:
        event() : iAlias{ *this }, iInstanceDataPtr { nullptr }, iNextId{ 0ull }
        {
        }
        event(const event&) : iAlias{ *this }, iInstanceDataPtr{ nullptr }, iNextId{ 0ull }
        {
        }
        virtual ~event()
        {
            set_destroying();
            clear();
        }
    public:
        self_type& operator=(const self_type&) = delete;
        self_type& operator=(const self_type&& aOther)
        {
            std::scoped_lock lock1{ iMutex };
            std::scoped_lock lock2{ aOther.iMutex };
            clear();
            iInstanceData = std::move(aOther.iInstanceData);
            iInstanceDataPtr = &*iInstanceData;
            return *this;
        }

    public:
        void handle_in_same_thread_as_emitter(cookie aHandleId)
        {
            instance().handlers[aHandleId].handleInSameThreadAsEmitter = true;
        }
    public:
        event_trigger_type trigger_type() const
        {
            return instance().triggerType;
        }
        void set_trigger_type(event_trigger_type aTriggerType)
        {
            instance().triggerType = aTriggerType;
        }
        bool trigger(Args... aArguments) const
        {
            if (!has_instance_data()) // no instance date means no subscribers so no point triggering.
                return true;
            std::scoped_lock lock{ iMutex };
            switch (trigger_type())
            {
            case event_trigger_type::Default:
            case event_trigger_type::Synchronous:
            case event_trigger_type::SynchronousDontQueue:
            default:
                return sync_trigger(aArguments...);
            case event_trigger_type::Asynchronous:
            case event_trigger_type::AsynchronousDontQueue:
                async_trigger(aArguments...);
                return true;
            }
        }
        bool sync_trigger(Args... aArguments) const
        {
            if (trigger_type() == event_trigger_type::SynchronousDontQueue && trigger_type() == event_trigger_type::AsynchronousDontQueue)
                unqueue();
            std::scoped_lock lock{ iMutex };
            destroyed_flag destroyed{ *this };
            instance().contexts.emplace_back();
            for (auto& h : instance().handlers)
            {
                try
                {
                    enqueue(h, false, aArguments...);
                }
                catch (...)
                {
                    instance().contexts.pop_back();
                    throw;
                }
                if (destroyed)
                    return false;
                if (instance().contexts.back().accepted)
                {
                    instance().contexts.pop_back();
                    return false;
                }
            }
            instance().contexts.pop_back();
            return true;
        }
        void async_trigger(Args... aArguments) const
        {
            if (!has_instance_data()) // no instance means no subscribers so no point triggering.
                return;
            std::scoped_lock lock{ iMutex };
            destroyed_flag destroyed{ *this };
            for (auto& h : instance().handlers)
            {
                enqueue(h, true, aArguments...);
                if (destroyed)
                    return;
            }
        }
        void accept() const
        {
            std::scoped_lock lock{ iMutex };
            instance_data().contexts.back().accepted = true;
        }
        void ignore() const
        {
            std::scoped_lock lock{ iMutex };
            instance_data().contexts.back().accepted = false;
        }
    public:
        event_handle subscribe(const function_type& aHandlerCallback, const void* aUniqueId = nullptr) const
        {
            std::scoped_lock lock{ iMutex };
            return event_handle{ iAlias, instance().handlers.emplace(async_event_queue::instance(), 0u, aUniqueId, std::make_shared<function_type>(aHandlerCallback)) };
        }
        event_handle operator()(const function_type& aHandlerCallback, const void* aUniqueId = nullptr) const
        {
            return subscribe(aHandlerCallback, aUniqueId);
        }
        template <typename T>
        event_handle subscribe(const function_type& aHandlerCallback, const T* aClientId) const
        {
            return subscribe(aHandlerCallback, static_cast<const void*>(aClientId));
        }
        template <typename T>
        event_handle operator()(const function_type& aHandlerCallback, const T* aClientId) const
        {
            return subscribe(aHandlerCallback, static_cast<const void*>(aClientId));
        }
        template <typename T>
        event_handle subscribe(const function_type& aHandlerCallback, const T& aClientId) const
        {
            return subscribe(aHandlerCallback, static_cast<const void*>(&aClientId));
        }
        template <typename T>
        event_handle operator()(const function_type& aHandlerCallback, const T& aClientId) const
        {
            return subscribe(aHandlerCallback, static_cast<const void*>(&aClientId));
        }
        void unsubscribe(event_handle aHandle) const
        {
            std::scoped_lock lock{ iMutex };
            instance().handlers.remove(aHandle.id);
        }
        void unsubscribe(const void* aClientId) const
        {
            std::scoped_lock lock{ iMutex };
            for (auto const& h : instance().handlers)
                if (h.clientId == aClientId)
                    instance().handlers.remove(h.id);
        }
        template <typename T>
        void unsubscribe(const T* aClientId) const
        {
            return unsubscribe(static_cast<const void*>(aClientId));
        }
        template <typename T>
        void unsubscribe(const T& aClientId) const
        {
            return unsubscribe(static_cast<const void*>(&aClientId));
        }
    private:
        void add_ref(cookie aCookie) override
        {
            std::scoped_lock lock{ iMutex };
            ++instance().handlers[aCookie].referenceCount;
        }
        void release(cookie aCookie) override
        {
            std::scoped_lock lock{ iMutex };
            if (--instance().handlers[aCookie].referenceCount == 0u)
                instance().handlers.remove(aCookie);
        }
        long use_count(cookie aCookie) const override
        {
            std::scoped_lock lock{ iMutex };
            return instance().handlers[aCookie].referenceCount;
        }
    private:
        void enqueue(handler& aHandler, bool aAsync, Args... aArguments) const
        {
            std::scoped_lock lock{ iMutex };
            auto& emitterQueue = async_event_queue::instance();
            if (!aAsync && aHandler.queue == &emitterQueue)
                (*aHandler.callback)(aArguments...);
            else
            {
                auto ecb = std::make_unique<event_callback<Args...>>(*this, aHandler.callback, aArguments...);
                if (aHandler.handleInSameThreadAsEmitter)
                    emitterQueue.enqueue(std::move(ecb));
                else
                    aHandler.queue->enqueue(std::move(ecb));
            }
        }
        void unqueue() const
        {
            std::scoped_lock lock{ iMutex };
            std::unordered_set<async_event_queue*> queues;
            for (auto const& h : instance().handlers)
                queues.insert(h.queue);
            for (auto const& q : queues)
                q->unqueue(*this);
        }
        void clear()
        {
            std::scoped_lock lock{ iMutex };
            for (auto& h : instance().handlers)
                h.queue->remove(*this);
            iInstanceDataPtr = nullptr;
            iInstanceData = std::nullopt;
        }
        bool has_instance_data() const
        {
            return iInstanceDataPtr != nullptr;
        }
        instance_data& instance() const
        {
            if (iInstanceDataPtr != nullptr)
                return *iInstanceDataPtr;
            std::scoped_lock lock{ iMutex };
            iInstanceData.emplace();
            iInstanceDataPtr = &*iInstanceData;
            return *iInstanceDataPtr;
        }
    private:
        self_type& iAlias;
        mutable std::recursive_mutex iMutex;
        mutable std::optional<instance_data> iInstanceData;
        mutable std::atomic<instance_data*> iInstanceDataPtr;
        mutable uint64_t iNextId;
    };

    class sink
    {
    public:
        sink()
        {
        }
        sink(event_handle aHandle)
        {
            iHandles.emplace_back(aHandle.event, aHandle.id);
        }
        sink(const sink& aSink) :
            iHandles{ aSink.iHandles }
        {
        }
        virtual ~sink()
        {
            clear();
        }
    public:
        sink& operator=(const sink& aSink)
        {
            if (this == &aSink)
                return *this;
            iHandles = aSink.iHandles;
            return *this;
        }
    public:
        sink& operator=(event_handle aHandle)
        {
            return *this = sink{ aHandle };
        }
        sink& operator+=(event_handle aHandle)
        {
            sink s{ aHandle };
            iHandles.insert(iHandles.end(), s.iHandles.begin(), s.iHandles.end());
            return *this;
        }
    public:
        void clear()
        {
            iHandles.clear();
        }
    private:
        mutable std::vector<cookie_ref_ptr> iHandles;
    };
}