// thread_pool.cpp
/*
 *  Copyright (c) 2017 Leigh Johnston.
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
#include <condition_variable>
#include <neolib/raii.hpp>
#include <neolib/destroyable.hpp>
#include <neolib/thread.hpp>
#include <neolib/thread_pool.hpp>

namespace neolib
{
	class thread_pool_thread : public thread
	{
	public:
		typedef std::shared_ptr<i_task> task_pointer;
		typedef std::pair<task_pointer, int32_t> task_queue_entry;
		typedef std::deque<task_queue_entry> task_queue;
	public:
		struct no_active_task : std::logic_error { no_active_task() : std::logic_error("neolib::thread_pool_thread::no_active_task") {} };
		struct already_active : std::logic_error { already_active() : std::logic_error("neolib::thread_pool_thread::already_active") {} };
	public:
		thread_pool_thread(thread_pool& aThreadPool) : thread{ "neolib::thread_pool_thread" }, iThreadPool{ aThreadPool }, iPoolMutex{ aThreadPool.mutex() }
		{
			start();
		}
	public:
		virtual void task()
		{
			while (!finished())
			{
				std::unique_lock<std::mutex> lk(iCondVarMutex);
				iConditionVariable.wait(lk, [this] { return iActiveTask != nullptr; });
				lk.unlock();
				if (!iActiveTask->cancelled())
					iActiveTask->run();
				std::lock_guard<std::recursive_mutex> lk2(iPoolMutex);
				release();
				next_task();
			}
		}
	public:
		bool active() const
		{
			std::lock_guard<std::mutex> lk(iCondVarMutex);
			return iActiveTask != nullptr;
		}
		bool idle() const
		{
			std::lock_guard<std::recursive_mutex> lk(iPoolMutex);
			std::lock_guard<std::mutex> lk2(iCondVarMutex);
			return iActiveTask == nullptr && iWaitingTasks.empty();
		}
		void add(task_pointer aTask, int32_t aPriority)
		{
			std::lock_guard<std::recursive_mutex> lk(iPoolMutex);
			auto where = std::upper_bound(iWaitingTasks.begin(), iWaitingTasks.end(), task_queue_entry{ task_pointer{}, aPriority },
				[](const task_queue_entry& aLeft, const task_queue_entry& aRight)
			{
				return aLeft.second > aRight.second;
			});
			iWaitingTasks.emplace(where, aTask, aPriority);
			if (!active())
				next_task();
		}
		bool steal_work(thread_pool_thread& aIdleThread)
		{
			std::unique_lock<std::recursive_mutex> lk(iPoolMutex);
			if (!iWaitingTasks.empty())
			{
				auto newTask = iWaitingTasks.front();
				iWaitingTasks.pop_front();
				aIdleThread.add(newTask.first, newTask.second);
				return true;
			}
			return false;
		}
	private:
		void next_task()
		{
			std::unique_lock<std::recursive_mutex> lk(iPoolMutex);
			if (active())
				throw already_active();
			if (iWaitingTasks.empty())
				iThreadPool.steal_work(*this);
			if (!iWaitingTasks.empty())
			{
				{
					std::lock_guard<std::mutex> lk2(iCondVarMutex);
					iActiveTask = iWaitingTasks.front().first;
					iWaitingTasks.pop_front();
				}
				iConditionVariable.notify_one();
			}
			else
				iThreadPool.thread_gone_idle();
		}
		void release()
		{
			task_pointer currentTask;
			{
				std::lock_guard<std::mutex> lk(iCondVarMutex);
				if (iActiveTask == nullptr)
					throw no_active_task();
				currentTask = iActiveTask;
				iActiveTask = nullptr;
			}
		}
	private:
		thread_pool& iThreadPool;
		std::recursive_mutex& iPoolMutex;
		mutable std::mutex iCondVarMutex;
		std::condition_variable iConditionVariable;
		task_queue iWaitingTasks;
		task_pointer iActiveTask;
	};

	thread_pool::thread_pool() : iMaxThreads{ 0 }
	{
		reserve(std::thread::hardware_concurrency());
	}

	void thread_pool::reserve(std::size_t aMaxThreads)
	{
		std::lock_guard<std::recursive_mutex> lk(iMutex);
		iMaxThreads = aMaxThreads;
		while (iThreads.size() < iMaxThreads)
			iThreads.push_back(std::make_unique<thread_pool_thread>(*this));
	}

	std::size_t thread_pool::active_threads() const
	{
		std::lock_guard<std::recursive_mutex> lk(iMutex);
		std::size_t result = 0;
		for (auto& t : iThreads)
			if (static_cast<thread_pool_thread&>(*t).active())
				++result;
		return result;
	}

	std::size_t thread_pool::available_threads() const
	{
		std::lock_guard<std::recursive_mutex> lk(iMutex);
		return max_threads() - active_threads();
	}

	std::size_t thread_pool::total_threads() const
	{
		std::lock_guard<std::recursive_mutex> lk(iMutex);
		std::size_t result = 0;
		for (auto& t : iThreads)
			if (!static_cast<thread_pool_thread&>(*t).finished())
				++result;
		return result;
	}

	std::size_t thread_pool::max_threads() const
	{
		return iMaxThreads;
	}

	void thread_pool::start(i_task& aTask, int32_t aPriority)
	{
		start(task_pointer{ task_pointer{}, &aTask }, aPriority);
	}

	void thread_pool::start(task_pointer aTask, int32_t aPriority)
	{
		std::lock_guard<std::recursive_mutex> lk(iMutex);
		if (iThreads.empty())
			throw no_threads();
		for (auto& t : iThreads)
		{
			auto& tpt = static_cast<thread_pool_thread&>(*t);
			if (!tpt.active())
			{
				tpt.add(aTask, aPriority);
				return;
			}
		}
		static_cast<thread_pool_thread&>(*iThreads[0]).add(aTask, aPriority);
	}

	bool thread_pool::try_start(i_task& aTask, int32_t aPriority)
	{
		if (available_threads() == 0)
			return false;
		start(aTask, aPriority);
		return true;
	}

	bool thread_pool::try_start(task_pointer aTask, int32_t aPriority)
	{
		if (available_threads() == 0)
			return false;
		start(aTask, aPriority);
		return true;
	}

	std::pair<std::future<void>, thread_pool::task_pointer> thread_pool::run(std::function<void()> aFunction, int32_t aPriority)
	{
		auto newTask = std::make_shared<function_task<void>>(aFunction);
		start(newTask, aPriority);
		return std::make_pair(newTask->get_future(), newTask);
	}

	bool thread_pool::idle() const
	{
		std::lock_guard<std::recursive_mutex> lk(iMutex);
		for (auto& t : iThreads)
		{
			if (!static_cast<thread_pool_thread&>(*t).idle())
				return false;
		}
		return true;
	}

	bool thread_pool::busy() const
	{
		return !idle();
	}

	void thread_pool::wait() const
	{
		std::unique_lock<std::mutex> lk(iWaitMutex);
		iWaitConditionVariable.wait(lk, [this] { return idle(); });
	}

	thread_pool& thread_pool::default_thread_pool()
	{
		static thread_pool sDefaultThreadPool;
		return sDefaultThreadPool;
	}

	std::recursive_mutex& thread_pool::mutex() const
	{
		return iMutex;
	}

	void thread_pool::steal_work(thread_pool_thread& aIdleThread)
	{
		std::lock_guard<std::recursive_mutex> lk(iMutex);
		if (iThreads.empty())
			throw no_threads();
		for (auto& t : iThreads)
		{
			if (&*t == &aIdleThread)
				continue;
			auto& tpt = static_cast<thread_pool_thread&>(*t);
			if (tpt.steal_work(aIdleThread))
				return;
		}
	}

	void thread_pool::thread_gone_idle()
	{
		iWaitConditionVariable.notify_one();
	}
}
