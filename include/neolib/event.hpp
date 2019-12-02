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
#include <list>
#include <unordered_map>
#include <optional>
#include <mutex>

#include <neolib/any.hpp>
#include <neolib/allocator.hpp>
#include <neolib/mutex.hpp>
#include <neolib/lifetime.hpp>
#include <neolib/async_task.hpp>
#include <neolib/timer.hpp>
#include <neolib/raii.hpp>

namespace neolib
{
    class sink;

    class i_event_handle;

    class i_event
    {
    public:
        virtual ~i_event() {}
    public:
        virtual void handle_add_ref(const i_event_handle& aHandle) const = 0;
        virtual void handle_release(const i_event_handle& aHandle) const = 0;
        virtual void handle_updated(const i_event_handle& aHandle) const = 0;
    };

    class i_event_handle
    {
    public:
        virtual ~i_event_handle() {}
    public:
        virtual i_event_handle& operator=(const i_event_handle& aOther) = 0;
    public:
        virtual void add_ref() = 0;
        virtual void release() = 0;
    public:
        virtual const i_event& event() const = 0;
        virtual uint64_t id() const = 0;
    public:
        virtual bool is_handling_in_same_thread_as_emitter() const = 0;
        virtual void handle_in_same_thread_as_emitter() = 0;
    };

    struct event_destroyed : std::logic_error { event_destroyed() : std::logic_error{ "neolib::event_destroyed" } {} };

    template <typename... Args>
    class event_handle : public i_event_handle, std::enable_shared_from_this<i_event_handle>
    {
        typedef event_handle<Args...> self_type;
    public:
        event_handle(const i_event& aEvent, uint64_t aId) :
            iEvent{ aEvent }, iId{ aId }, iHandleInSameThreadAsEmitter{ false }
        {
        }
        event_handle(const i_event_handle& aOther) :
            iEvent{ aOther.event() }, iId{ aOther.id() }, iHandleInSameThreadAsEmitter{ aOther.is_handling_in_same_thread_as_emitter() }
        {
        }
    public:
        self_type& operator=(const self_type& aOther)
        {
            iId = aOther.id();
            iHandleInSameThreadAsEmitter = aOther.is_handling_in_same_thread_as_emitter();
            return *this;
        }
        self_type& operator=(const i_event_handle& aOther) override
        {
            iId = aOther.id();
            iHandleInSameThreadAsEmitter = aOther.is_handling_in_same_thread_as_emitter();
            return *this;
        }
    public:
        void add_ref() override
        {
            event().handle_add_ref(*this);
        }
        void release() override
        {
            event().handle_release(*this);
        }
    public:
        const i_event& event() const override
        {
            return iEvent;
        }
        uint64_t id() const override
        {
            return iId;
        }
        bool is_handling_in_same_thread_as_emitter() const override
        {
            return iHandleInSameThreadAsEmitter;
        }
        void handle_in_same_thread_as_emitter() override
        {
            iHandleInSameThreadAsEmitter = true;
            event().handle_updated(*this);
        }
        event_handle& operator~()
        {
            handle_in_same_thread_as_emitter();
            return *this;
        }
    public:
        const i_event& iEvent;
        uint64_t iId;
        bool iHandleInSameThreadAsEmitter;
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
    private:
        typedef std::function<void(Args&&...)> function_type;
        typedef std::shared_ptr<function_type> handler_ptr;
        typedef std::tuple<Args&&...> argument_pack;
    public:
        event_callback(const i_event& aEvent, handler_ptr aHandler, Args&&... aArguments) :
            iEvent{ aEvent }, iArguments { aArguments... }
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
        template <typename Handler, typename... Args>
        void enqueue(const i_event& aEvent, Handler aHandler, Args&&... aArguments)
        {
            add(std::make_unique<event_callback<Args...>>(aEvent, aHandler, std::forward<Args>(aArguments)...));
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
        typedef event_handle<Args...> handle_type;
        typedef typename event_callback<Args...>::function_type function_type;
        struct handler
        {
            async_event_queue* queue;
            uint32_t referenceCount;
            const void* id;
            handle_type handle;
            std::shared_ptr<function_type> callback;
        };
        typedef std::vector<handler> handler_list_t;
        struct context
        {
            bool accepted;
        };
        typedef std::deque<context> context_list_t;
        struct instance_data
        {
            event_trigger_type triggerType = event_trigger_type::Default;
            handler_list_t handlers;
            context_list_t contexts;
        };
    public:
        event() : iInstanceDataPtr { nullptr }, iNextId{ 0ull }
        {
        }
        event(const event&) = delete;
        virtual ~event()
        {
            set_destroying();
            clear();
        }
    public:
        event & operator=(const event&)
        {
            clear();
            return *this;
        }
    public:
        event_trigger_type trigger_type() const
        {
            return instance_data().triggerType;
        }
        void set_trigger_type(event_trigger_type aTriggerType)
        {
            instance_data().triggerType = aTriggerType;
        }
        bool trigger(Args... aArguments) const
        {
            if (!has_instance_data()) // no instance date means no subscribers so no point triggering.
                return true;
            std::lock_guard lg{ iMutex };
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
            std::lock_guard lg{ iMutex };
            destroyed_flag destroyed{ *this };
            instance().contexts.emplace_back();
            auto const& context = instance().contexts.back();
            for (auto const& h : instance().handlers)
            {
                enqueue(h, false, aArguments...);
                if (destroyed)
                    return false;
                if (context.accepted)
                    return false;
            }
            return true;
        }
        void async_trigger(Args... aArguments) const
        {
            if (!has_instance_data()) // no instance means no subscribers so no point triggering.
                return;
            std::lock_guard lg{ iMutex };
            destroyed_flag destroyed{ *this };
            for (auto const& h : instance().handlers)
            {
                enqueue(h, true, aArguments...);
                if (destroyed)
                    return;
            }
        }
        void accept() const
        {
            std::lock_guard lg{ iMutex };
            instance_data().contexts.back().accepted = true;
        }
        void ignore() const
        {
            std::lock_guard lg{ iMutex };
            instance_data().contexts.back().accepted = false;
        }
    public:
        handle_type subscribe(const function_type& aHandlerCallback, const void* aUniqueId = nullptr) const
        {
            std::lock_guard lg{ iMutex };
            instance().handlers.push_back(handler{ &async_event_queue::instance(), 0u, aUniqueId, handle_type{ *this, iNextId++ }, std::make_shared<function_type>(aHandlerCallback) });
            return instance().handlers.back().handle;
        }
        handle_type operator()(const function_type& aHandlerCallback, const void* aUniqueId = nullptr) const
        {
            return subscribe(aHandlerCallback, aUniqueId);
        }
        template <typename T>
        handle_type subscribe(const function_type& aHandlerCallback, const T* aUniqueIdObject) const
        {
            return subscribe(aHandlerCallback, static_cast<const void*>(aUniqueIdObject));
        }
        template <typename T>
        handle_type operator()(const function_type& aHandlerCallback, const T* aUniqueIdObject) const
        {
            return subscribe(aHandlerCallback, static_cast<const void*>(aUniqueIdObject));
        }
        template <typename T>
        handle_type subscribe(const function_type& aHandlerCallback, const T& aUniqueIdObject) const
        {
            return subscribe(aHandlerCallback, static_cast<const void*>(&aUniqueIdObject));
        }
        template <typename T>
        handle_type operator()(const function_type& aHandlerCallback, const T& aUniqueIdObject) const
        {
            return subscribe(aHandlerCallback, static_cast<const void*>(&aUniqueIdObject));
        }
        void unsubscribe(handle_type aHandle) const
        {
            std::lock_guard lg{ iMutex };
            auto handler = find_handler(aHandle);
            if (handler != instance().handlers.end())
                instance().handlers.erase(handler);
        }
        void unsubscribe(const void* aUniqueId) const
        {
            std::lock_guard lg{ iMutex };
            for (auto h = instance().handlers.begin(); h != instance().handlers.end();)
                if (h->id == aUniqueId)
                    h = instance().handlers.erase(h);
                else
                    ++h;
        }
        template <typename T>
        void unsubscribe(const T* aUniqueIdObject) const
        {
            return unsubscribe(static_cast<const void*>(aUniqueIdObject));
        }
        template <typename T>
        void unsubscribe(const T& aUniqueIdObject) const
        {
            return unsubscribe(static_cast<const void*>(&aUniqueIdObject));
        }
    private:
        void handle_add_ref(const i_event_handle& aHandle) const override
        {
            std::lock_guard lg{ iMutex };
            auto handler = find_handler(aHandle);
            if (handler != instance().handlers.end())
                ++handler->referenceCount;
        }
        void handle_release(const i_event_handle& aHandle) const override
        {
            std::lock_guard lg{ iMutex };
            auto handler = find_handler(aHandle);
            if (handler != instance().handlers.end())
                if (--handler->referenceCount == 0u)
                    instance().handlers.erase(handler);
        }
        void handle_updated(const i_event_handle& aHandle) const override
        {
            std::lock_guard lg{ iMutex };
            auto handler = find_handler(aHandle);
            if (handler != instance().handlers.end())
                handler->handle = aHandle;
        }
    private:
        typename handler_list_t::iterator find_handler(const i_event_handle& aHandle) const
        {
            std::lock_guard lg{ iMutex };
            // todo: faster ID lookup?
            return std::find_if(instance().handlers.begin(), instance().handlers.end(), [&aHandle](auto const& h) { return h.handle.id() == aHandle.id(); });
        }
    private:
        void enqueue(const handler& aHandler, bool aAsync, Args... aArguments) const
        {
            std::lock_guard lg{ iMutex };
            auto& emitterQueue = async_event_queue::instance();
            if (!aAsync && aHandler.queue == &emitterQueue)
                (*aHandler.callback)(aArguments...);
            else if (aHandler.handle.is_handling_in_same_thread_as_emitter())
                emitterQueue.enqueue(*this, aHandler.callback, aArguments...);
            else
                aHandler.queue->enqueue(*this, aHandler.callback, aArguments...);
        }
        void unqueue() const
        {
            std::lock_guard lg{ iMutex };
            std::unordered_set<async_event_queue*> queues;
            for (auto const& h : instance().handlers)
                queues.insert(h.queue);
            for (auto const& q : queues)
                q->unqueue(*this);
        }
        void clear()
        {
            std::lock_guard lg{ iMutex };
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
            std::lock_guard lg{ iMutex };
            iInstanceData.emplace();
            iInstanceDataPtr = &*iInstanceData;
        }
    private:
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
        template <typename... Args>
        sink(const event_handle<Args...>& aHandle)
        {
            iHandles.emplace_back(aHandle);
            add_ref();
        }
        sink(const sink& aSink) :
            iHandles{ aSink.iHandles }
        {
            add_ref();
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
            release();
            iHandles = aSink.iHandles;
            add_ref();
            return *this;
        }
    public:
        template <typename... Args>
        sink& operator=(const event_handle<Args...>& aHandle)
        {
            return *this = sink{ aHandle };
        }
        template <typename... Args>
        sink& operator+=(const event_handle<Args...>& aHandle)
        {
            sink s{ aHandle };
            s.add_ref();
            iHandles.insert(iHandles.end(), s.iHandles.begin(), s.iHandles.end());
            return *this;
        }
    public:
        void clear()
        {
            release();
            iHandles.clear();
        }
    private:
        void add_ref() const
        {
            for (auto h : iHandles)
                h->add_ref();
        }
        void release() const
        {
            for (auto h : iHandles)
                h->release();
        }
    private:
        mutable std::vector<i_event_handle*> iHandles;
    };
}