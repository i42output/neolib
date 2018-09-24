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

#include "neolib.hpp"
#include <vector>
#include <list>
#include <unordered_map>
#include <optional>
#include <mutex>
#include "any.hpp"
#include "allocator.hpp"
#include "mutex.hpp"
#include "lifetime.hpp"
#include "async_task.hpp"
#include "timer.hpp"
#include "raii.hpp"

namespace neolib
{
	class event_system
	{
	public:
		static bool single_threaded()
		{
			return !instance().iMultiThreaded;
		}
		static bool multi_threaded()
		{
			return instance().iMultiThreaded;
		}
		static void set_single_threaded()
		{
			instance().iMultiThreaded = false;
		}
		static void set_multi_threaded()
		{
			instance().iMultiThreaded = true;
		}
	private:
		static event_system& instance()
		{
			static event_system sInstance;
			return sInstance;
		}
	private:
		bool iMultiThreaded = true;
	};

	class event_mutex : public basic_lifetime<own_flag_list<event_mutex>>
	{
	public:
		event_mutex() :
			iLockCount(0)
		{
		}
		~event_mutex()
		{
			while (iLockCount)
				unlock();
		}
	public:
		void lock()
		{
			if (event_system::multi_threaded())
			{
				++iLockCount;
				iRealMutex.lock();
			}
		}
		void unlock() noexcept
		{
			if (iLockCount > 0)
			{
				--iLockCount;
				iRealMutex.unlock();
			}
		}
		bool try_lock()
		{
			if (event_system::multi_threaded())
			{
				bool locked = iRealMutex.try_lock();
				if (locked)
					++iLockCount;
				return locked;
			}
			else
				return true;
		}
	private:
		std::atomic<uint32_t> iLockCount;
		std::recursive_mutex iRealMutex;
	};

	typedef basic_lifetime<own_flag_list<event_mutex>> event_lifetime;

	class sink;

	template <typename... Arguments>
	class event;

	class i_event_handle
	{
	public:
		virtual void add_ref() const = 0;
		virtual void release() const = 0;
	};

	template <typename... Arguments>
	class event_handle : public i_event_handle
	{
	public:
		typedef const event<Arguments...>* event_ptr;
		typedef std::shared_ptr<event_ptr> event_instance_ptr;
		typedef std::weak_ptr<event_ptr> event_instance_weak_ptr;
		typedef const void* unique_id_type;
		typedef std::function<void(Arguments...)> handler_callback;
		typedef uint32_t sink_reference_count;
		struct handler_list_item 
		{ 
			std::optional<std::thread::id> iThreadId; 
			unique_id_type iUniqueId; 
			handler_callback iHandlerCallback; 
			sink_reference_count iSinkReferenceCount = 0; 
		};
		typedef std::list<handler_list_item, thread_safe_fast_pool_allocator<handler_list_item>> handler_list;
	public:
		event_handle(event_instance_weak_ptr aEvent, typename handler_list::iterator aHandler) : 
			iEvent{ aEvent }, iHandler{ aHandler }
		{
		}
	public:
		event_handle& operator~()
		{
			iHandler->iThreadId = std::nullopt;
			return *this;
		}
	public:
		typename handler_list::iterator handler() const
		{
			return iHandler;
		}
		void add_ref() const override
		{
			if (!iEvent.expired())
				++iHandler->iSinkReferenceCount;
		}
		void release() const override
		{
			if (!iEvent.expired() && --iHandler->iSinkReferenceCount == 0 && !iEvent.expired())		
				(**iEvent.lock()).unsubscribe(*this);
		}
	private:
		event_instance_weak_ptr iEvent;
		typename handler_list::iterator iHandler;
	};

	class async_event_queue
	{
	private:
		class local_thread;
	public:
		typedef std::function<void()> callback;
	public:
		struct no_instance : std::logic_error { no_instance() : std::logic_error("neogfx::async_event_queue::no_instance") {} };
		struct instance_exists : std::logic_error { instance_exists() : std::logic_error("neogfx::async_event_queue::instance_exists") {} };
		struct event_not_found : std::logic_error { event_not_found() : std::logic_error("neogfx::async_event_queue::event_not_found") {} };
	private:
		struct instance_pointers
		{
			async_event_queue* aliased;
			std::weak_ptr<async_event_queue> counted;
		};
		typedef std::unordered_multimap<
			const void*,
			std::pair<callback, event_lifetime::destroyed_flag>,
			std::hash<const void*>,
			std::equal_to<const void*>,
			thread_safe_fast_pool_allocator<std::pair<const void* const, std::pair<callback, event_lifetime::destroyed_flag>>>> event_list;
		typedef std::vector<callback> callback_list;
		typedef std::unordered_map<
			std::thread::id,
			callback_list,
			std::hash<std::thread::id>,
			std::equal_to<std::thread::id>,
			thread_safe_fast_pool_allocator<std::pair<const std::thread::id, callback_list>>> threaded_callbacks;
	public:
		async_event_queue();
		async_event_queue(neolib::async_task& aTask);
		~async_event_queue();
		static std::shared_ptr<async_event_queue> instance();
	public:
		template<typename... Arguments>
		void add(const event<Arguments...>& aEvent, callback aCallback)
		{
			add(static_cast<const void*>(&aEvent), aCallback, event_lifetime::destroyed_flag(aEvent));
		}
		template<typename... Arguments>
		void remove(const event<Arguments...>& aEvent)
		{
			remove(static_cast<const void*>(&aEvent));
		}
		template<typename... Arguments>
		bool has(const event<Arguments...>& aEvent) const
		{
			return has(static_cast<const void*>(&aEvent));
		}
		bool exec();
		void enqueue_to_thread(std::thread::id aThreadId, callback aCallback);
		void terminate();
		void persist(std::shared_ptr<async_event_queue> aPtr, uint32_t aDuration_ms = 1000u);
	private:
		async_event_queue(std::shared_ptr<async_task> aTask);
		static std::recursive_mutex& instance_mutex();
		static instance_pointers& instance_ptrs();
		void add(const void* aEvent, callback aCallback, event_lifetime::destroyed_flag aDestroyedFlag);
		void remove(const void* aEvent);
		bool has(const void* aEvent) const;
		void publish_events();
	private:
		std::shared_ptr<async_task> iTask;
		neolib::callback_timer iTimer;
		mutable event_mutex iEventsMutex;
		event_list iEvents;
		event_mutex iThreadedCallbacksMutex;
		std::atomic<bool> iHaveThreadedCallbacks;
		threaded_callbacks iThreadedCallbacks;
		std::atomic<bool> iTerminated;
		std::pair<std::shared_ptr<async_event_queue>, std::chrono::time_point<std::chrono::steady_clock>> iCache;
	};

	enum class event_trigger_type
	{
		Default,
		Synchronous,
		Asynchronous
	};

	template <typename... Arguments>
	class event : protected event_lifetime
	{
		friend class sink;
		friend class async_event_queue;
	private:
		typedef event<Arguments...> self_type;
		typedef event_handle<Arguments...> handle;
		typedef typename handle::event_ptr ptr;
		typedef typename handle::event_instance_ptr instance_ptr;
		typedef typename handle::event_instance_weak_ptr instance_weak_ptr;
		typedef typename handle::unique_id_type unique_id_type;
		typedef typename handle::handler_callback handler_callback;
		typedef typename handle::sink_reference_count sink_reference_count;
		typedef typename handle::handler_list_item handler_list_item;
		typedef typename handle::handler_list handler_list;
		typedef std::unordered_map<
			unique_id_type, 
			typename handler_list::iterator,
			std::hash<unique_id_type>, 
			std::equal_to<unique_id_type>, 
			thread_safe_fast_pool_allocator<std::pair<const unique_id_type, typename handler_list::iterator>>> unique_id_map;
		class copyable_atomic_bool : public std::atomic<bool>
		{
		public:
			copyable_atomic_bool() : std::atomic<bool>{} {}
			copyable_atomic_bool(const std::atomic<bool>& a) : std::atomic<bool>{ a.load() } {}
			copyable_atomic_bool(const copyable_atomic_bool& other) : std::atomic<bool>{ other.load() } {}
			copyable_atomic_bool& operator=(const copyable_atomic_bool& other) { store(other.load()); return *this; }
			copyable_atomic_bool& operator=(bool value) { store(value); return *this; }
		};
		typedef std::tuple<copyable_atomic_bool, handler_callback, typename handler_list::const_iterator> notification;
		typedef std::vector<notification> notification_list;
		typedef std::shared_ptr<notification_list> notification_list_ptr;
		typedef std::vector<notification_list_ptr> notification_list_pool;
		struct state : event_lifetime
		{
			instance_ptr instancePtr;
			std::shared_ptr<async_event_queue> asyncEventQueue;
			handler_list handlers;
			unique_id_map uniqueIdMap;
			event_trigger_type triggerType;
			struct context
			{
				bool accepted;
				notification_list_ptr notifications;
			};
			typedef std::shared_ptr<context> context_ptr;
			typedef std::list<context_ptr, thread_safe_fast_pool_allocator<context_ptr>> context_list;
			context_list contexts;
			notification_list_pool notificationListPool;
		};
		typedef thread_safe_fast_pool_allocator<state> state_allocator;
	public:
		event() : iInstanceData { nullptr }, iInSync{ false }
		{
		}
		event(const event&) : iInstanceData{ nullptr }, iInSync{ false }
		{
			// do nothing.
		}
		~event()
		{
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
		template<class... Ts>
		bool trigger(Ts&&... aArguments) const
		{
			if (!has_instance_data()) // no instance date means no subscribers so no point triggering.
				return true;
			destroyable_mutex_lock_guard<event_mutex> guard{ iMutex };
			switch (trigger_type())
			{
			case event_trigger_type::Default:
			case event_trigger_type::Synchronous:
			default:
				return sync_trigger(std::forward<Ts>(aArguments)...);
			case event_trigger_type::Asynchronous:
				async_trigger(std::forward<Ts>(aArguments)...);
				return true;
			}
		}
		template<class... Ts>
		bool sync_trigger(Ts&&... aArguments) const
		{
			if (!has_instance_data()) // no instance date means no subscribers so no point triggering.
				return true;
			scoped_atomic_flag saf{ iInSync };
			destroyable_mutex_lock_guard<event_mutex> guard{ iMutex };
			destroyed_flag destroyed{ *this };
			auto& instanceData = instance_data();
			class scoped_context
			{
			public:
				scoped_context(const self_type& aOwner) : 
					iInstanceDestroyed{ aOwner }, 
					iInstanceDataDestroyed{ aOwner.instance_data() },
					iMutex{ aOwner.iMutex },
					iContexts{ aOwner.instance_data().contexts },
					iIterContext{ iContexts.insert(iContexts.end(), std::make_shared<typename state::context>()) },
					iContextPtr{ *iIterContext },
					iNotificationListPool{ aOwner.instance_data().notificationListPool }
				{
					if (iNotificationListPool.empty())
						context().notifications = std::make_shared<notification_list>();
					else
					{
						context().notifications = iNotificationListPool.back();
						iNotificationListPool.pop_back();
					}
				}
				~scoped_context()
				{
					if (!iInstanceDestroyed && !iInstanceDataDestroyed)
					{
						destroyable_mutex_lock_guard<event_mutex> guard{ iMutex };
						context().notifications->clear();
						iNotificationListPool.push_back(context().notifications);
						iContexts.erase(iIterContext);
					}
				}
			public:
				typename state::context& context() const
				{
					return *iContextPtr;
				}
			private:
				destroyed_flag iInstanceDestroyed;
				destroyed_flag iInstanceDataDestroyed;
				event_mutex& iMutex;
				typename state::context_list& iContexts;
				typename state::context_list::const_iterator iIterContext;
				typename state::context_ptr iContextPtr; // need smart pointer copy here to extend possible lifetime of context...
				notification_list_pool& iNotificationListPool;
			} sc { *this };
			auto& context = sc.context();
			context.notifications->reserve(instanceData.handlers.size());
			for (auto iterHandler = instanceData.handlers.begin(); iterHandler != instanceData.handlers.end(); ++iterHandler)
				if (iterHandler->iThreadId == std::nullopt || *iterHandler->iThreadId == std::this_thread::get_id())
					context.notifications->emplace_back(true, iterHandler->iHandlerCallback, iterHandler);
				else
					enqueue_to_thread(*iterHandler, std::forward<Ts>(aArguments)...);
			guard.unlock();
			for (auto& notification : *context.notifications)
			{
				if (!std::get<0>(notification))
					continue;
				std::get<1>(notification)(std::forward<Ts>(aArguments)...);
				if (destroyed)
					return false;
				if (context.accepted)
					return false;
			}
			return true;
		}
		template<class... Ts>
		void async_trigger(Ts&&... aArguments) const
		{
			if (!has_instance_data()) // no instance means no subscribers so no point triggering.
				return;
			destroyable_mutex_lock_guard<event_mutex> guard{ iMutex };
			instance_data().asyncEventQueue->add(*this, [this, &aArguments...]() { sync_trigger(std::forward<Ts>(aArguments)...); });
		}
		void accept() const
		{
			destroyable_mutex_lock_guard<event_mutex> guard{ iMutex };
			instance_data().contexts.back()->accepted = true;
		}
		void ignore() const
		{
			destroyable_mutex_lock_guard<event_mutex> guard{ iMutex };
			instance_data().contexts.back()->accepted = false;
		}
	public:
		handle subscribe(const handler_callback& aHandlerCallback, const void* aUniqueId = nullptr) const
		{
			destroyable_mutex_lock_guard<event_mutex> guard{ iMutex };
			auto& instanceData = instance_data();
			if (aUniqueId == nullptr)
				return handle{ instanceData.instancePtr, instanceData.handlers.insert(instanceData.handlers.end(), handler_list_item{ std::this_thread::get_id(), aUniqueId, aHandlerCallback }) };
			auto existing = instanceData.uniqueIdMap.find(aUniqueId);
			if (existing == instanceData.uniqueIdMap.end())
				existing = instanceData.uniqueIdMap.insert(std::make_pair(aUniqueId, instanceData.handlers.insert(instanceData.handlers.end(), handler_list_item{ std::this_thread::get_id(), aUniqueId, aHandlerCallback }))).first;
			else
				existing->second->iHandlerCallback = aHandlerCallback;
			return handle{ instanceData.instancePtr, existing->second };
		}
		handle operator()(const handler_callback& aHandlerCallback, const void* aUniqueId = nullptr) const
		{
			return subscribe(aHandlerCallback, aUniqueId);
		}
		template <typename T>
		handle subscribe(const handler_callback& aHandlerCallback, const T* aUniqueIdObject) const
		{
			return subscribe(aHandlerCallback, static_cast<const void*>(aUniqueIdObject));
		}
		template <typename T>
		handle operator()(const handler_callback& aHandlerCallback, const T* aUniqueIdObject) const
		{
			return subscribe(aHandlerCallback, static_cast<const void*>(aUniqueIdObject));
		}
		template <typename T>
		handle subscribe(const handler_callback& aHandlerCallback, const T& aUniqueIdObject) const
		{
			return subscribe(aHandlerCallback, static_cast<const void*>(&aUniqueIdObject));
		}
		template <typename T>
		handle operator()(const handler_callback& aHandlerCallback, const T& aUniqueIdObject) const
		{
			return subscribe(aHandlerCallback, static_cast<const void*>(&aUniqueIdObject));
		}
		void unsubscribe(handle aHandle) const
		{
			destroyable_mutex_lock_guard<event_mutex> guard{ iMutex };
			auto& instanceData = instance_data();
			for (auto& context : instanceData.contexts)
				for (auto& notification : *(*context).notifications)
				{
					if (std::get<2>(notification) == aHandle.handler())
						std::get<0>(notification) = false;
				}
			if (aHandle.handler()->iUniqueId != nullptr)
			{
				auto existing = instanceData.uniqueIdMap.find(aHandle.handler()->iUniqueId);
				if (existing != instanceData.uniqueIdMap.end())
					instanceData.uniqueIdMap.erase(existing);
			}
			instanceData.handlers.erase(aHandle.handler());
		}
		void unsubscribe(const void* aUniqueId) const
		{
			destroyable_mutex_lock_guard<event_mutex> guard{ iMutex };
			auto& instanceData = instance_data();
			auto existing = instanceData.uniqueIdMap.find(aUniqueId);
			if (existing != instanceData.uniqueIdMap.end())
				unsubscribe(handle{ instanceData.instancePtr, existing->second });
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
		template<class... Ts>
		void enqueue_to_thread(const handler_list_item& aItem, Ts&&... aArguments) const
		{
			auto callback = aItem.iHandlerCallback;
			std::tuple<Ts...> arguments{ std::forward<Ts>(aArguments)... };
			instance_data().asyncEventQueue->enqueue_to_thread(*aItem.iThreadId, [callback, arguments](){ std::apply(callback, arguments); });
		}
		void clear()
		{
			if (!has_instance_data())
				return;
			destroyable_mutex_lock_guard<event_mutex> guard{ iMutex };
			auto& instanceData = instance_data();
			if (instanceData.asyncEventQueue->has(*this))
				instanceData.asyncEventQueue->remove(*this);
			struct destroy_state
			{
				state* instanceData;
				~destroy_state()
				{
					allocator().destroy(instanceData);
					allocator().deallocate(instanceData);
				}
			} destroyer{ iInstanceData };
			iInstanceData = nullptr;
			auto queue = instanceData.asyncEventQueue;
			if (queue.use_count() == 2)
				queue->persist(queue); // keeps event queue around (cached) for a second 
		}
		bool has_instance_data() const
		{
			return iInstanceData != nullptr;
		}
		state& instance_data() const
		{
			if (has_instance_data())
				return *iInstanceData;
			destroyable_mutex_lock_guard<event_mutex> guard{ iMutex };
			if (iInstanceData == nullptr)
			{
				auto newInstance = allocator().allocate();
				try
				{
					allocator().construct(newInstance);
					newInstance->instancePtr = std::make_shared<ptr>(this);
					newInstance->asyncEventQueue = async_event_queue::instance();
				}
				catch (...)
				{
					allocator().deallocate(newInstance);
					throw;
				}
				iInstanceData = newInstance;
			}
			return *iInstanceData;
		}
		static state_allocator& allocator()
		{
			static state_allocator sAllocator;
			return sAllocator;
		}
	private:
		mutable event_mutex iMutex;
		mutable std::atomic<state*> iInstanceData;
		mutable std::atomic<bool> iInSync;
	};

	class sink
	{
	public:
		sink()
		{
		}
		template <typename... Arguments>
		sink(event_handle<Arguments...> aHandle)
		{
			iHandlers.emplace_back(aHandle);
			add_ref();
		}
		sink(const sink& aSink) :
			iHandlers{ aSink.iHandlers }
		{
			add_ref();
		}
		sink& operator=(const sink& aSink)
		{
			if (this == &aSink)
				return *this;
			release();
			iHandlers = aSink.iHandlers;
			add_ref();
			return *this;
		}
		template <typename... Arguments>
		sink& operator=(event_handle<Arguments...> aHandle)
		{
			return *this = sink{ aHandle };
		}
		template <typename... Arguments>
		sink& operator+=(event_handle<Arguments...> aHandle)
		{
			sink s{ aHandle };
			s.add_ref();
			iHandlers.insert(iHandlers.end(), s.iHandlers.begin(), s.iHandlers.end());
			return *this;
		}
		~sink()
		{
			release();
		}
	private:
		void add_ref() const
		{
			for (auto& h : iHandlers)
				unsafe_any_cast<const i_event_handle&>(h).add_ref();
		}
		void release() const
		{
			for (auto& h : iHandlers)
				unsafe_any_cast<const i_event_handle&>(h).release();
		}
	private:
		mutable std::vector<any> iHandlers;
	};
}