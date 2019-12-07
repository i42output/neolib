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
#include <atomic>

#include <neolib/scoped.hpp>
#include <neolib/lifetime.hpp>
#include <neolib/jar.hpp>
#include <neolib/i_event.hpp>

namespace neolib
{
    class sink;

    class event_handle
    {
    public:
        struct no_control : std::logic_error { no_control() : std::logic_error{ "neolib::event_handle::no_control" } {} };
    public:
        event_handle(i_event_control& aControl, cookie aId) : 
            iControl{ &aControl }, iRef{ aControl.get(), aId }, iPrimary{ true }
        {
            control().add_ref();
        }
        event_handle(const event_handle& aOther) :
            iControl{ aOther.iControl }, iRef{ aOther.iRef }, iPrimary{ false }
        {
            if (have_control())
                control().add_ref();
        }
        event_handle(event_handle&& aOther) :
            iControl{ aOther.iControl }, iRef{ aOther.iRef }, iPrimary{ false }
        {
            aOther.iPrimary = false;
            if (have_control())
                control().add_ref();
        }
        ~event_handle()
        {
            if (have_control())
            {
                if (!control().valid() || primary())
                    iRef.reset();
                control().release();
            }
        }
    public:
        event_handle& operator=(const event_handle& aRhs)
        {
            if (&aRhs == this)
                return *this;
            auto oldControl = iControl;
            iControl = aRhs.iControl;
            if (have_control())
                control().add_ref();
            if (oldControl != nullptr)
                oldControl->release();
            iRef = aRhs.iRef;
            return *this;
        }
    public:
        bool have_control() const
        {
            return iControl != nullptr;
        }
        i_event_control& control() const
        {
            if (have_control())
                return *iControl;
            throw no_control();
        }
        cookie id() const
        {
            return iRef.cookie();
        }
        bool primary() const
        {
            return iPrimary;
        }
        event_handle& operator~()
        {
            if (control().valid())
                control().get().handle_in_same_thread_as_emitter(id());
            return *this;
        }
    private:
        i_event_control* iControl;
        cookie_ref_ptr iRef;
        bool iPrimary;
    };

    class event_control : public i_event_control
    {
    public:
        struct no_event : std::logic_error { no_event() : std::logic_error{ "neolib::event_control::no_event" } {} };
    public:
        event_control(i_event& aEvent) :
            iEvent{ &aEvent }, iRefCount{ 0u }
        {
        }
        ~event_control()
        {
            if (valid())
                get().release_control();
        }
    public:
        void add_ref() override
        {
            ++iRefCount;
        }
        void release() override
        {
            if (--iRefCount == 0u)
                delete this;
        }
        bool valid() const override
        {
            return iEvent != nullptr;
        }
        i_event& get() const override
        {
            if (valid())
                return *iEvent;
            throw no_event();
        }
    public:
        void reset() override
        {
            iEvent = nullptr;
        }
    private:
        std::atomic<i_event*> iEvent;
        std::atomic<uint32_t> iRefCount;
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

    class async_task;
    class callback_timer;

    class async_event_queue : public lifetime
    {
        template <typename...>
        friend class event;
    public:
        struct async_event_queue_needs_a_task : std::logic_error { async_event_queue_needs_a_task() : std::logic_error("neogfx::async_event_queue::async_event_queue_needs_a_task") {} };
        struct async_event_queue_already_instantiated : std::logic_error { async_event_queue_already_instantiated() : std::logic_error("neogfx::async_event_queue::async_event_queue_already_instantiated") {} };
        struct async_event_queue_terminated : std::logic_error { async_event_queue_terminated() : std::logic_error("neogfx::async_event_queue::async_event_queue_terminated") {} };
        struct event_not_found : std::logic_error { event_not_found() : std::logic_error("neogfx::async_event_queue::event_not_found") {} };
    private:
        typedef std::unique_ptr<i_event_callback> callback_ptr;
        typedef std::deque<callback_ptr> event_list_t;
    public:
        static async_event_queue& instance();
        static async_event_queue& instance(async_task& aTask);
        ~async_event_queue();
    private:
        async_event_queue(async_task& aTask);
        static async_event_queue& get_instance(async_task* aTask);
    public:
        bool exec();
        void enqueue(callback_ptr aCallback)
        {
            add(std::move(aCallback));
        }
        void unqueue(const i_event& aEvent);
        void terminate();
    private:
        bool terminated() const;
        void add(callback_ptr aCallback);
        void remove(const i_event& aEvent);
        bool has(const i_event& aEvent) const;
        bool publish_events();
    private:
        std::recursive_mutex iMutex;
        std::unique_ptr<callback_timer> iTimer;
        event_list_t iEvents;
        std::atomic<bool> iTerminated;
        destroyed_flag iTaskDestroyed;
        std::atomic<uint32_t> iPublishNestingLevel;
        std::vector<std::unique_ptr<event_list_t>> iPublishCache;
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
        typedef std::optional<std::scoped_lock<std::recursive_mutex>> optional_scoped_lock;
        typedef typename event_callback<Args...>::function_type function_type;
        typedef typename event_callback<Args...>::handler_ptr handler_ptr;
        struct handler
        {
            async_event_queue* queue;
            uint32_t referenceCount;
            const void* clientId;
            handler_ptr callback;
            bool handleInSameThreadAsEmitter;
            uint64_t triggerId = 0ull;

            handler(
                async_event_queue& queue, 
                uint32_t referenceCount, 
                const void* clientId, 
                handler_ptr callback,
                bool handleInSameThreadAsEmitter = false) : 
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
            bool accepted = false;
            bool handlersChanged = false;
        };
        typedef std::vector<context> context_list_t;
        struct instance_data
        {
            event_trigger_type triggerType = event_trigger_type::Default;
            handler_list_t handlers;
            context_list_t contexts;
            bool triggering = false;
            uint64_t triggerId = 0ull;
            bool handlersChanged = false;
        };
    public:
        event() : iAlias{ *this }, iMutex{ std::make_shared<std::recursive_mutex>() }, iControl{ nullptr }, iInstanceDataPtr{ nullptr }
        {
        }
        event(const event&) : iAlias{ *this }, iMutex{ std::make_shared<std::recursive_mutex>() }, iControl{ nullptr }, iInstanceDataPtr{ nullptr }
        {
        }
        virtual ~event()
        {
            std::scoped_lock lock{ *iMutex };
            if (is_controlled())
                control().reset();
            set_destroying();
            clear();
        }
    public:
        self_type& operator=(const self_type&) = delete;
        self_type& operator=(const self_type&& aOther)
        {
            std::scoped_lock lock1{ *iMutex };
            std::scoped_lock lock2{ *aOther.iMutex };
            clear();
            iInstanceData = std::move(aOther.iInstanceData);
            aOther.clear();
            iInstanceDataPtr = &*iInstanceData;
            return *this;
        }
    public:
        void release_control() override
        {
            if (iControl != nullptr)
            {
                iControl.load()->reset();
                iControl.store(nullptr);
            }
        }
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
            if (!has_instance_data()) // no instance means no subscribers so no point triggering.
                return false;
            if (trigger_type() == event_trigger_type::SynchronousDontQueue)
                unqueue();
            auto mutex = iMutex;
            optional_scoped_lock lock{ *mutex };
            destroyed_flag destroyed{ *this };
            instance().contexts.emplace_back();
            scoped_flag sf{ instance().triggering };
            if (!instance().triggering)
            {
                instance().triggering = true;
                instance().triggerId = 0ull;
                for (auto& handler : instance().handlers)
                    handler.triggerId = 0ull;
            }
            auto triggerId = ++instance().triggerId;
            for (auto h = instance().handlers.begin(); h != instance().handlers.end();)
            {
                auto& handler = *h++;
                if (handler.triggerId < triggerId)
                    handler.triggerId = triggerId;
                else if (handler.triggerId == triggerId)
                    continue;
                try
                {
                    enqueue(lock, handler, false, aArguments...);
                    if (destroyed)
                        return false;
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
                if (instance().contexts.back().handlersChanged)
                {
                    instance().contexts.back().handlersChanged = false;
                    h = instance().handlers.begin();
                }
            }
            instance().contexts.pop_back();
            return true;
        }
        void async_trigger(Args... aArguments) const
        {
            if (!has_instance_data()) // no instance means no subscribers so no point triggering.
                return;
            if (trigger_type() == event_trigger_type::AsynchronousDontQueue)
                unqueue();
            auto mutex = iMutex;
            optional_scoped_lock lock{ *mutex };
            destroyed_flag destroyed{ *this };
            scoped_flag sf{ instance().triggering };
            if (!instance().triggering)
            {
                instance().triggering = true;
                instance().triggerId = 0ull;
                for (auto& handler : instance().handlers)
                    handler.triggerId = 0ull;
            }
            auto triggerId = ++instance().triggerId;
            for (auto h = instance().handlers.begin(); h != instance().handlers.end();)
            {
                auto& handler = *h++;
                if (handler.triggerId < triggerId)
                    handler.triggerId = triggerId;
                else if (handler.triggerId == triggerId)
                    continue;
                enqueue(lock, handler, true, aArguments...);
                if (destroyed)
                    return;
                if (instance().handlersChanged)
                {
                    instance().handlersChanged = false;
                    h = instance().handlers.begin();
                }
            }
        }
        void accept() const
        {
            std::scoped_lock lock{ *iMutex };
            instance_data().contexts.back().accepted = true;
        }
        void ignore() const
        {
            std::scoped_lock lock{ *iMutex };
            instance_data().contexts.back().accepted = false;
        }
    public:
        event_handle subscribe(const function_type& aHandlerCallback, const void* aUniqueId = nullptr) const
        {
            std::scoped_lock lock{ *iMutex };
            invalidate_handler_list();
            return event_handle{ control(), instance().handlers.emplace(async_event_queue::instance(), 0u, aUniqueId, std::make_shared<function_type>(aHandlerCallback)) };
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
            std::scoped_lock lock{ *iMutex };
            invalidate_handler_list();
            instance().handlers.remove(aHandle.id());
        }
        void unsubscribe(const void* aClientId) const
        {
            std::scoped_lock lock{ *iMutex };
            invalidate_handler_list();
            for (auto h = instance().handlers.begin(); h != instance().handlers.end();)
                if (h->clientId == aClientId)
                    h = instance().handlers.erase(h);
                else
                    ++h;
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
            std::scoped_lock lock{ *iMutex };
            ++instance().handlers[aCookie].referenceCount;
        }
        void release(cookie aCookie) override
        {
            std::scoped_lock lock{ *iMutex };
            if (--instance().handlers[aCookie].referenceCount == 0u)
                instance().handlers.remove(aCookie);
        }
        long use_count(cookie aCookie) const override
        {
            std::scoped_lock lock{ *iMutex };
            return instance().handlers[aCookie].referenceCount;
        }
    private:
        void invalidate_handler_list() const
        {
            std::scoped_lock lock{ *iMutex };
            instance().handlersChanged = true;
            for (auto& context : instance().contexts)
                context.handlersChanged = true;
        }
        void enqueue(optional_scoped_lock& lock, handler& aHandler, bool aAsync, Args... aArguments) const
        {
            auto& emitterQueue = async_event_queue::instance();
            if (!aAsync && aHandler.queue == &emitterQueue)
            {
                auto mutex = iMutex;
                auto callback = aHandler.callback;
                lock = std::nullopt;
                (*callback)(aArguments...);
                lock.emplace(*mutex);
            }
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
            std::scoped_lock lock{ *iMutex };
            std::unordered_set<async_event_queue*> queues;
            for (auto const& h : instance().handlers)
                queues.insert(h.queue);
            for (auto const& q : queues)
                q->unqueue(*this);
        }
        void clear()
        {
            std::scoped_lock lock{ *iMutex };
            for (auto& h : instance().handlers)
                h.queue->remove(*this);
            iInstanceDataPtr = nullptr;
            iInstanceData = std::nullopt;
        }
        bool is_controlled() const
        {
            return iControl != nullptr;
        }
        i_event_control& control() const
        {
            std::scoped_lock lock{ *iMutex };
            if (iControl == nullptr)
                iControl = new event_control{ iAlias };
            return *iControl;
        }
        bool has_instance_data() const
        {
            return iInstanceDataPtr != nullptr;
        }
        instance_data& instance() const
        {
            if (iInstanceDataPtr != nullptr)
                return *iInstanceDataPtr;
            std::scoped_lock lock{ *iMutex };
            iInstanceData.emplace();
            iInstanceDataPtr = &*iInstanceData;
            return *iInstanceDataPtr;
        }
    private:
        self_type& iAlias; // bit of a hack: most event operations are logically const as we want to be able to trigger events from const methods of the containing object
        mutable std::shared_ptr<std::recursive_mutex> iMutex;
        mutable std::atomic<i_event_control*> iControl;
        mutable std::optional<instance_data> iInstanceData;
        mutable std::atomic<instance_data*> iInstanceDataPtr;
    };

    class sink
    {
    public:
        sink()
        {
        }
        sink(const event_handle& aHandle)
        {
            iHandles.push_back(aHandle);
        }
        sink(event_handle&& aHandle)
        {
            iHandles.push_back(std::move(aHandle));
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
        sink& operator=(const event_handle& aHandle)
        {
            return *this = sink{ aHandle };
        }
        sink& operator=(event_handle&& aHandle)
        {
            return *this = sink{ std::move(aHandle) };
        }
        sink& operator+=(const event_handle& aHandle)
        {
            sink s{ aHandle };
            iHandles.insert(iHandles.end(), s.iHandles.begin(), s.iHandles.end());
            return *this;
        }
        sink& operator+=(event_handle&& aHandle)
        {
            sink s{ std::move(aHandle) };
            iHandles.insert(iHandles.end(), s.iHandles.begin(), s.iHandles.end());
            return *this;
        }
    public:
        void clear()
        {
            iHandles.clear();
        }
    private:
        mutable std::vector<event_handle> iHandles;
    };
}