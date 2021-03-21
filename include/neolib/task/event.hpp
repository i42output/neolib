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
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <mutex>
#include <atomic>

#include <neolib/core/scoped.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/lifetime.hpp>
#include <neolib/core/jar.hpp>
#include <neolib/task/i_event.hpp>

namespace neolib
{
    class event_handle : public i_event_handle
    {
    public:
        typedef i_event_handle abstract_type;
    public:
        event_handle() noexcept : 
            iControl{ nullptr }, iId{ invalid_cookie<cookie> }, iActive{ false }
        {
        }
        event_handle(i_event_control& aControl, cookie aId) noexcept :
            iControl{ &aControl }, iId{ aId }, iActive{ false }
        {
            if (have_control())
                control().add_ref();
        }
        event_handle(event_handle const& aOther) noexcept :
            event_handle{ static_cast<i_event_handle const&>(aOther) }
        {
        }
        event_handle(event_handle&& aOther) noexcept :
            event_handle{ static_cast<i_event_handle&&>(aOther) }
        {
        }
        event_handle(i_event_handle const& aOther) noexcept :
            iControl{ aOther.have_control() ? &aOther.control() : nullptr }, iId{ aOther.id() }, iActive{ aOther.active() }
        {
            if (have_control())
                control().add_ref();
        }
        event_handle(i_event_handle&& aOther) noexcept :
            iControl{ aOther.have_control() ? &aOther.control() : nullptr }, iId{ aOther.id() }, iActive{ aOther.active() }
        {
            aOther.detach();
        }
        ~event_handle() noexcept
        {
            if (have_control() && active())
            {
                if (control().valid())
                    control().get().remove_handler(id());
                control().release();
            }
        }
    public:
        event_handle& operator=(event_handle const& aRhs) noexcept
        {
            return *this = static_cast<i_event_handle const&>(aRhs);
        }
        event_handle& operator=(event_handle&& aRhs) noexcept
        {
            return *this = static_cast<i_event_handle&&>(aRhs);
        }
        event_handle& operator=(i_event_handle const& aRhs) noexcept override
        {
            if (&aRhs == this)
                return *this;
            auto oldControl = iControl;
            iControl = aRhs.have_control() ? &aRhs.control() : nullptr;
            iId = aRhs.id();
            iActive = aRhs.active();
            if (have_control())
                control().add_ref();
            if (oldControl != nullptr)
                oldControl->release();
            return *this;
        }
        event_handle& operator=(i_event_handle&& aRhs) noexcept override
        {
            if (&aRhs == this)
                return *this;
            auto oldControl = iControl;
            iControl = aRhs.have_control() ? &aRhs.control() : nullptr;
            iId = aRhs.id();
            iActive = aRhs.active();
            aRhs.detach();
            if (oldControl != nullptr)
                oldControl->release();
            return *this;
        }
    public:
        bool have_control() const noexcept override
        {
            return iControl != nullptr;
        }
        i_event_control& control() const override
        {
            if (have_control())
                return *iControl;
            throw no_control();
        }
        cookie id() const noexcept override
        {
            return iId;
        }
        bool active() const noexcept override
        {
            return iActive;
        }
        void set_active() noexcept override
        {
            iActive = true;
        }
    public:
        event_handle& operator~() noexcept override
        {
            if (control().valid())
                control().get().handle_in_same_thread_as_emitter(id());
            return *this;
        } 
        event_handle& operator!() noexcept override
        {
            if (control().valid())
                control().get().handler_is_stateless(id());
            return *this;
        }
    public:
        void detach() noexcept override
        {
            iControl = nullptr;
            iId = invalid_cookie<cookie>;
            iActive = false;
        }
    private:
        i_event_control* iControl;
        cookie iId;
        bool iActive;
    };

    class event_control : public i_event_control
    {
    public:
        struct no_event : std::logic_error { no_event() : std::logic_error{ "neolib::event_control::no_event" } {} };
    public:
        event_control(i_event& aEvent) noexcept :
            iEvent{ &aEvent }, iRefCount{ 0u }
        {
        }
        ~event_control() noexcept
        {
            if (valid())
                get().release_control();
        }
    public:
        void add_ref() noexcept override
        {
            ++iRefCount;
        }
        void release() noexcept override
        {
            if (--iRefCount == 0u)
                delete this;
        }
        bool valid() const noexcept override
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
        void reset() noexcept override
        {
            iEvent = nullptr;
        }
    private:
        std::atomic<i_event*> iEvent;
        std::atomic<uint32_t> iRefCount;
    };

    template <typename... Args>
    class i_event_callable : public i_reference_counted
    {
        typedef i_event_callable<Args...> self_type;
    public:
        typedef self_type abstract_type;
    };

    template <typename... Args>
    class event_callable : public std::function<void(Args...)>, public reference_counted<i_event_callable<Args...>>
    {
    public:
        typedef std::function<void(Args...)> concrete_callable;
    public:
        template <typename Callable>
        event_callable(const Callable& aCallable) :
            concrete_callable{ aCallable }
        {
        }
        event_callable(const concrete_callable& aCallable) :
            concrete_callable{ aCallable }
        {
        }
    };

    template <typename... Args>
    class event_callback : public reference_counted<i_event_callback>
    {
        template <typename...>
        friend class event;
        friend class async_event_queue;
    private:
        typedef event_callable<Args...> callable;
        typedef typename callable::concrete_callable concrete_callable;
        typedef std::tuple<Args...> argument_pack;
    public:
        event_callback(const i_event& aEvent, const ref_ptr<callable>& aCallable, Args... aArguments) :
            iEvent{ &aEvent }, iCallable{ aCallable }, iArguments{ aArguments... }
        {
        }
    public:
        bool operator==(const i_event_callback& aRhs) const override
        {
            return &event() == &aRhs.event() && identity() == aRhs.identity();
        }
    public:
        const i_event& event() const override
        {
            return *iEvent;
        }
        const void* identity() const override
        {
            return &*iCallable;
        }
        bool valid() const override
        {
            return iCallable.valid();
        }
        void call() const override
        {
            if (valid())
                std::apply(*iCallable, iArguments);
            else
                throw event_callable_expired();
        }
    private:
        const i_event* iEvent;
        weak_ref_ptr<callable> iCallable;
        argument_pack iArguments;
    };

    class i_async_task;
    class callback_timer;

    NEOLIB_EXPORT void unqueue_event(const i_event& aEvent);

    class NEOLIB_EXPORT async_event_queue : public lifetime<>
    {
        template <typename...>
        friend class event;
    public:
        struct async_event_queue_needs_a_task : std::logic_error { async_event_queue_needs_a_task() : std::logic_error("neolib::async_event_queue::async_event_queue_needs_a_task") {} };
        struct async_event_queue_already_instantiated : std::logic_error { async_event_queue_already_instantiated() : std::logic_error("neolib::async_event_queue::async_event_queue_already_instantiated") {} };
        struct event_not_found : std::logic_error { event_not_found() : std::logic_error("neolib::async_event_queue::event_not_found") {} };
    private:
        typedef uint64_t transaction;
        typedef std::optional<transaction> optional_transaction;
        typedef ref_ptr<i_event_callback> callback_ptr;
        struct event_list_entry
        {
            async_event_queue::transaction transaction;
            destroyed_flag destroyed;
            callback_ptr callback;
        };
        typedef std::deque<event_list_entry> event_list_t;
    public:
        static async_event_queue& instance();
        static async_event_queue& instance(i_async_task& aTask);
        ~async_event_queue();
    private:
        async_event_queue(i_async_task& aTask);
        static async_event_queue& get_instance(i_async_task* aTask);
    public:
        bool exec();
        transaction enqueue(callback_ptr aCallback, bool aStatelessHandler, const optional_transaction& aTransaction = {});
        void unqueue(const i_event& aEvent);
        void terminate();
    public:
        i_event_filter_registry& filter_registry();
    public:
        bool debug() const;
        void set_debug(bool aDebug);
    private:
        bool terminated() const;
        transaction add(callback_ptr aCallback, const optional_transaction& aTransaction);
        void remove(const i_event& aEvent);
        bool has(const i_event& aEvent) const;
        bool publish_events();
    private:
        i_async_task& iTask;
        std::unique_ptr<callback_timer> iTimer;
        event_list_t iEvents;
        std::atomic<bool> iTerminated;
        destroyed_flag iTaskDestroyed;
        std::atomic<uint32_t> iPublishNestingLevel;
        std::vector<std::unique_ptr<event_list_t>> iPublishCache;
        transaction iNextTransaction;
#if !defined(NDEBUG) || defined(DEBUG_EVENTS)
        bool iDebug = false;
#endif
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
    class event : public i_event, public lifetime<>
    {
        typedef event<Args...> self_type;
        friend class sink;
        friend class async_event_queue;
    private:
        typedef event_callback<Args...> callback;
        typedef typename callback::callable callback_callable;
        typedef typename callback_callable::concrete_callable concrete_callable;
        typedef async_event_queue::optional_transaction optional_async_transaction;
        struct handler
        {
            typedef handler abstract_type;
            async_event_queue* queue;
            destroyed_flag queueDestroyed;
            uint32_t referenceCount;
            const void* clientId;
            ref_ptr<callback_callable> callable;
            bool handleInSameThreadAsEmitter;
            bool handlerIsStateless;
            uint64_t triggerId = 0ull;

            handler(
                async_event_queue& queue, 
                const void* clientId, 
                const ref_ptr<callback_callable>& callable,
                bool handleInSameThreadAsEmitter = false,
                bool handlerIsStateless = false) :
                queue{ &queue },
                queueDestroyed{ queue },
                referenceCount{ 0u },
                clientId{ clientId },
                callable{ callable },
                handleInSameThreadAsEmitter{ handleInSameThreadAsEmitter },
                handlerIsStateless{ handlerIsStateless }
            {}
        };
        typedef jar<handler> handler_list_t;
        struct context
        {
            bool accepted;
            std::atomic<bool> handlersChanged;
            context() : 
                accepted{ false },
                handlersChanged{ false }
            {
            }
            context(const context& aOther) : 
                accepted{ aOther.accepted },
                handlersChanged{ aOther.handlersChanged.load() }
            {
            }
            context& operator=(const context& aOther)
            {
                accepted = aOther.accepted;
                handlersChanged = aOther.handlersChanged.load();
                return *this;
            }
        };
        typedef std::vector<context> context_list_t;
        struct instance_data
        {
            bool ignoreErrors = false;
            event_trigger_type triggerType = event_trigger_type::Default;
            handler_list_t handlers;
            context_list_t contexts;
            bool triggering = false;
            uint64_t triggerId = 0ull;
            std::atomic<bool> handlersChanged = false;
            std::atomic<uint32_t> filterCount;
        };
        typedef std::optional<std::scoped_lock<switchable_mutex>> optional_lock;
    public:
        event() : iAlias{ *this }, iControl{ nullptr }, iInstanceDataPtr{ nullptr }
        {
        }
        event(const event&) = delete;
        virtual ~event()
        {
            if (filtered())
                async_event_queue::instance().filter_registry().uninstall_event_filter(*this);
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            if (is_controlled())
            {
                control().reset();
                control().release();
            }
            set_destroying();
            clear();
        }
    public:
        self_type& operator=(const self_type&) = delete;
    public:
        void release_control() override
        {
            if (iControl != nullptr)
            {
                iControl.load()->reset();
                iControl.store(nullptr);
            }
        }
        void remove_handler(cookie aHandlerId) override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            instance().handlers.remove(aHandlerId);
        }
        void handle_in_same_thread_as_emitter(cookie aHandlerId) override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            get_handler(aHandlerId).handleInSameThreadAsEmitter = true;
        }
        void handler_is_stateless(cookie aHandlerId) override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            get_handler(aHandlerId).handlerIsStateless = true;
        }
    public:
        void push_context() const override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            instance().contexts.emplace_back();
        }
        void pop_context() const override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            instance().contexts.pop_back();
        }
    public:
        void pre_trigger() const override
        {
            if (filtered())
                async_event_queue::instance().filter_registry().pre_filter_event(*this);
        }
    public:
        void ignore_errors()
        {
            instance().ignoreErrors = true;
        }
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
            if (!has_instance()) // no instance means no subscribers so no point triggering.
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
            if (!has_instance()) // no instance means no subscribers so no point triggering.
                return true;
            if (trigger_type() == event_trigger_type::SynchronousDontQueue)
                unqueue();
            optional_lock lock{ event_mutex() };
            auto& handlers = instance().handlers;
            if (handlers.empty() && !filtered())
                return true;
            destroyed_flag destroyed{ *this };
            push_context();
            if (filtered())
            {
                async_event_queue::instance().filter_registry().filter_event(*this);
                if (destroyed)
                    return true;
                if (instance().contexts.back().accepted)
                {
                    pop_context();
                    return false;
                }
            }
            if (handlers.empty())
                return true;
            scoped_flag sf{ instance().triggering };
            if (!sf.saved())
            {
                instance().triggerId = 0ull;
                for (auto& handler : handlers)
                    handler.triggerId = 0ull;
            }
            auto triggerId = ++instance().triggerId;
            optional_async_transaction transaction;
            for (std::size_t handlerIndex = {}; handlerIndex < handlers.size();)
            {
                auto& handler = *std::next(handlers.begin(), handlerIndex++);
                if (handler.triggerId < triggerId)
                    handler.triggerId = triggerId;
                else if (handler.triggerId == triggerId)
                    continue;
                try
                {
                    transaction = enqueue(lock, handler, false, transaction, aArguments...);
                    if (destroyed)
                        return true;
                }
                catch (...)
                {
                    pop_context();
                    throw;
                }
                if (destroyed)
                    return true;
                if (instance().contexts.back().accepted)
                {
                    pop_context();
                    return false;
                }
                if (instance().handlersChanged.exchange(false))
                    handlerIndex = {};
            }
            pop_context();
            return true;
        }
        void async_trigger(Args... aArguments) const
        {
            if (!has_instance()) // no instance means no subscribers so no point triggering.
                return;
            if (trigger_type() == event_trigger_type::AsynchronousDontQueue)
                unqueue();
            optional_lock lock{ event_mutex() };
            auto& handlers = instance().handlers;
            if (handlers.empty())
                return;
            destroyed_flag destroyed{ *this };
            scoped_flag sf{ instance().triggering };
            if (!sf.saved())
            {
                instance().triggerId = 0ull;
                for (auto& handler : handlers)
                    handler.triggerId = 0ull;
            }
            auto triggerId = ++instance().triggerId;
            optional_async_transaction transaction;
            for (std::size_t handlerIndex = {}; handlerIndex < handlers.size();)
            {
                auto& handler = *std::next(handlers.begin(), handlerIndex++);
                if (handler.triggerId < triggerId)
                    handler.triggerId = triggerId;
                else if (handler.triggerId == triggerId)
                    continue;
                transaction = enqueue(lock, handler, true, transaction, aArguments...);
                if (destroyed)
                    return;
                if (instance().handlersChanged.exchange(false))
                    handlerIndex = {};
            }
        }
        bool accepted() const override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            return !instance().contexts.empty() ? instance().contexts.back().accepted : false;
        }
        void accept() const override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            instance().contexts.back().accepted = true;
        }
        void ignore() const override
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            instance().contexts.back().accepted = false;
        }
    public:
        bool filtered() const override
        {
            return has_instance() && instance().filterCount > 0u;
        }
        void filter_added() const override
        {
            ++instance().filterCount;
        }
        void filter_removed() const override
        {
            --instance().filterCount;
        }
        void filters_removed() const override
        {
            instance().filterCount = 0u;
        }
    public:
        event_handle subscribe(const concrete_callable& aCallable, const void* aUniqueId = nullptr) const
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            invalidate_handler_list();
            auto id = instance().handlers.emplace(async_event_queue::instance(), aUniqueId, make_ref<callback_callable>(aCallable));
            return event_handle{ control(), id };
        }
        event_handle operator()(const concrete_callable& aCallable, const void* aUniqueId = nullptr) const
        {
            return subscribe(aCallable, aUniqueId);
        }
        template <typename T>
        event_handle subscribe(const concrete_callable& aCallable, const T* aClientId) const
        {
            return subscribe(aCallable, static_cast<const void*>(aClientId));
        }
        template <typename T>
        event_handle operator()(const concrete_callable& aCallable, const T* aClientId) const
        {
            return subscribe(aCallable, static_cast<const void*>(aClientId));
        }
        template <typename T>
        event_handle subscribe(const concrete_callable& aCallable, const T& aClientId) const
        {
            return subscribe(aCallable, static_cast<const void*>(&aClientId));
        }
        template <typename T>
        event_handle operator()(const concrete_callable& aCallable, const T& aClientId) const
        {
            return subscribe(aCallable, static_cast<const void*>(&aClientId));
        }
        void unsubscribe(event_handle aHandle) const
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            invalidate_handler_list();
            auto existing = find_handler(aHandle.id());
            if (existing != instance().handlers.end())
                erase_handler(existing);
            else
                throw event_handler_not_found();
        }
        void unsubscribe(const void* aClientId) const
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            invalidate_handler_list();
            auto& handlers = instance().handlers;
            for (auto h = handlers.begin(); h != handlers.end();)
                if ((*h).clientId == aClientId)
                    h = erase_handler(h);
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
        bool has_subscribers() const
        {
            if (!has_instance())
                return false;
            optional_lock lock{ event_mutex() };
            return !instance().handlers.empty();
        }
    private:
        typename handler_list_t::iterator erase_handler(typename handler_list_t::const_iterator aHandler) const
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            auto& handlers = instance().handlers;
            auto callable = aHandler->callable;
            return handlers.erase(aHandler);
        }
        void invalidate_handler_list() const
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            instance().handlersChanged = true;
            for (auto& context : instance().contexts)
                context.handlersChanged = true;
        }
        optional_async_transaction enqueue(optional_lock& aLock, handler& aHandler, bool aAsync, const optional_async_transaction& aAsyncTransaction, Args... aArguments) const
        {
            optional_async_transaction transaction;
            auto& emitterQueue = async_event_queue::instance();
            if (!aAsync && (aHandler.handleInSameThreadAsEmitter || (!aHandler.queueDestroyed && aHandler.queue == &emitterQueue)))
            {
                auto callable = aHandler.callable;
                bool wasLocked = !!aLock;
                aLock.reset();
                (*callable)(aArguments...);
                if (wasLocked)
                    aLock.emplace(event_mutex());
            }
            else
            {
                auto ecb = make_ref<callback>(*this, aHandler.callable, aArguments...);
                if (aHandler.handleInSameThreadAsEmitter)
                    transaction = emitterQueue.enqueue(ecb, aHandler.handlerIsStateless, aAsyncTransaction);
                else
                {
                    if (!aHandler.queueDestroyed)
                        transaction = aHandler.queue->enqueue(ecb, aHandler.handlerIsStateless, aAsyncTransaction);
                    else if (!instance().ignoreErrors)
                        throw event_queue_destroyed();
                }
            }
            return transaction;
        }
        void unqueue() const
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            unqueue_event(*this);
        }
        void clear()
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            unqueue_event(*this);
            iInstanceDataPtr = nullptr;
            iInstanceData = std::nullopt;
        }
        bool is_controlled() const
        {
            return iControl != nullptr;
        }
        i_event_control& control() const
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            if (iControl == nullptr)
            {
                iControl = new event_control{ iAlias };
                iControl.load()->add_ref();
            }
            return *iControl;
        }
        bool has_instance() const
        {
            return iInstanceDataPtr != nullptr;
        }
        instance_data& instance() const
        {
            if (iInstanceDataPtr != nullptr)
                return *iInstanceDataPtr;
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            iInstanceData.emplace();
            iInstanceDataPtr = &*iInstanceData;
            return *iInstanceDataPtr;
        }
        typename handler_list_t::iterator find_handler(cookie aHandlerId) const
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            auto& handlers = instance().handlers;
            return handlers.find(aHandlerId);
        }
        handler& get_handler(cookie aHandlerId) const
        {
            std::scoped_lock<switchable_mutex> lock{ event_mutex() };
            auto existing = find_handler(aHandlerId);
            if (existing != instance().handlers.end())
                return *existing;
            throw event_handler_not_found();
        }
    private:
        self_type& iAlias; // bit of a hack: most event operations are logically const as we want to be able to trigger events from const methods of the containing object
        mutable std::atomic<i_event_control*> iControl;
        mutable std::optional<instance_data> iInstanceData;
        mutable std::atomic<instance_data*> iInstanceDataPtr;
    };

    class sink : public i_sink
    {
    public:
        sink()
        {
        }
        sink(sink const& aSink) :
            iHandles{ aSink.iHandles }
        {
        }
        sink(i_sink const& aSink) :
            iHandles{ aSink.handles() }
        {
        }
        virtual ~sink()
        {
            clear();
        }
    public:
        sink& operator=(i_sink const& aSink) override
        {
            if (this == &aSink)
                return *this;
            iHandles = aSink.handles();
            return *this;
        }
        sink& operator=(i_sink&& aSink) override
        {
            if (this == &aSink)
                return *this;
            iHandles = std::move(aSink.handles());
            return *this;
        }
    public:
        sink& operator=(i_event_handle const& aHandle) override
        {
            clear();
            (*this) += aHandle;
            return *this;
        }
        sink& operator=(i_event_handle&& aHandle) override
        {
            clear();
            (*this) += std::move(aHandle);
            return *this;
        }
        sink& operator+=(i_event_handle const& aHandle) override
        {
            iHandles.container().insert(iHandles.container().end(), aHandle)->set_active();
            return *this;
        }
        sink& operator+=(i_event_handle&& aHandle) override
        {
            iHandles.container().insert(iHandles.container().end(), std::move(aHandle))->set_active();
            return *this;
        }
    public:
        bool empty() const override
        {
            return iHandles.empty();
        }
        void clear() override
        {
            iHandles.clear();
        }
        vector<event_handle> const& handles() const override
        {
            return iHandles;
        }
        vector<event_handle>& handles() override
        {
            return iHandles;
        }
    private:
        mutable vector<event_handle> iHandles;
    };
}
