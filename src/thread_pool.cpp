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
#include <neolib/destroyable.hpp>
#include <neolib/thread.hpp>
#include <neolib/thread_pool.hpp>

namespace neolib
{
	class thread_pool_thread : public thread
	{
	public:
		struct already_have_task : std::logic_error { already_have_task() : std::logic_error("neolib::thread_pool_thread::already_have_task") {} };
		struct no_task : std::logic_error { no_task() : std::logic_error("neolib::thread_pool_thread::no_task") {} };
	public:
		thread_pool_thread(thread_pool& aThreadPool) : thread{ "neolib::thread_pool_thread" }, iThreadPool{ aThreadPool }, iTask { nullptr }
		{
			start();
		}
	public:
		virtual void task()
		{
			while (!finished())
			{
				std::unique_lock<std::mutex> lk(iMutex);
				iConditionVariable.wait(lk, [this] { return iTask != nullptr; });
				lk.unlock();
				iTask->run();
				if (!release())
					break;
				else
					iThreadPool.next_task();
			}
		}
	public:
		bool acquired() const
		{
			std::lock_guard<std::mutex> lk(iMutex);
			return iTask != nullptr;
		}
		void acquire(i_task& aTask)
		{
			{
				std::lock_guard<std::mutex> lk(iMutex);
				if (iTask != nullptr)
					throw already_have_task();
				iTask = &aTask;
			}
			iConditionVariable.notify_one();
		}
		bool release()
		{
			i_task* currentTask = nullptr;
			{
				std::lock_guard<std::mutex> lk(iMutex);
				if (iTask == nullptr)
					throw no_task();
				currentTask = iTask;
				iTask = nullptr;
			}
			iThreadPool.delete_task(*currentTask);
			return !iThreadPool.too_many_threads();
		}
	private:
		thread_pool& iThreadPool;
		mutable std::mutex iMutex;
		std::condition_variable iConditionVariable;
		i_task* iTask;
	};

	thread_pool::thread_pool() : iMaxThreads{ 0 }, iPaused{ false }
	{
	}

	void thread_pool::reserve(std::size_t aMaxThreads)
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		iMaxThreads = aMaxThreads;
		while (iThreads.size() < iMaxThreads)
			iThreads.push_back(std::make_unique<thread_pool_thread>(*this));
	}

	std::size_t thread_pool::active_threads() const
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		std::size_t result = 0;
		for (auto& t : iThreads)
			if (static_cast<thread_pool_thread&>(*t).acquired())
				++result;
		return result;
	}

	std::size_t thread_pool::available_threads() const
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		return max_threads() - active_threads();
	}

	std::size_t thread_pool::total_threads() const
	{
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

	std::size_t thread_pool::waiting_tasks() const
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		return iWaitingTasks.size();
	}

	std::size_t thread_pool::active_tasks() const
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		return iActiveTasks.size();
	}

	bool thread_pool::paused() const
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		return iPaused;
	}

	void thread_pool::pause()
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		iPaused = true;
	}

	void thread_pool::resume()
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		iPaused = false;
		next_task();
	}

	void thread_pool::start(i_task& aTask, int32_t aPriority)
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		iWaitingTasks.emplace(free_slot(aPriority), std::shared_ptr<i_task>(std::shared_ptr<i_task>(), &aTask), aPriority);
		next_task();
	}

	void thread_pool::start(std::shared_ptr<i_task> aTask, int32_t aPriority)
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		iWaitingTasks.emplace(free_slot(aPriority), aTask, aPriority);
		next_task();
	}

	bool thread_pool::try_start(i_task& aTask)
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		if (available_threads() == 0)
			return false;
		start(aTask);
		return true;
	}

	bool thread_pool::try_start(std::shared_ptr<i_task> aTask)
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		if (available_threads() == 0)
			return false;
		start(aTask);
		return true;
	}

	namespace
	{
		template <typename T>
		class function_task : public i_task
		{
		public:
			function_task(std::function<T()> aFunction) : iFunction{ aFunction }
			{
			}
		public:
			std::future<T> get_future()
			{
				return iPromise.get_future();
			}
		public:
			const std::string& name() const override
			{
				static std::string sName = "neogfx::{}::function_task";
				return sName;
			}
			void run() override
			{
				iPromise.set_value(iFunction());
			}
		private:
			std::function<T()> iFunction;
			std::promise<T> iPromise;
		};

		template <>
		inline void function_task<void>::run()
		{
			iFunction();
			iPromise.set_value();
		}
	}

	std::future<void> thread_pool::run(std::function<void()> aFunction)
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		auto newTask = std::make_shared<function_task<void>>(aFunction);
		start(newTask);
		return newTask->get_future();
	}

	thread_pool& thread_pool::default_thread_pool()
	{
		static thread_pool sDefaultThreadPool;
		return sDefaultThreadPool;
	}

	bool thread_pool::too_many_threads() const
	{
		return max_threads() < total_threads();
	}

	thread_pool::task_queue::const_iterator thread_pool::free_slot(int32_t aPriority) const
	{
		return std::upper_bound(iWaitingTasks.begin(), iWaitingTasks.end(), task_queue_entry{ std::shared_ptr<i_task>{}, aPriority }, 
			[](const task_queue_entry& aLeft, const task_queue_entry& aRight)
		{
			return aLeft.second < aRight.second;
		});
	}

	void thread_pool::delete_task(i_task& aTask)
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		for (auto iterTask = iActiveTasks.begin(); iterTask != iActiveTasks.end(); ++iterTask)
			if (&*iterTask->first == &aTask)
			{
				iActiveTasks.erase(iterTask);
				break;
			}
	}

	void thread_pool::next_task()
	{
		std::lock_guard<std::recursive_mutex> lk{ iMutex };
		if (iPaused)
			return;
		bool exhuasted = false;
		while (!iWaitingTasks.empty() && !exhuasted)
		{
			exhuasted = true;
			for (auto iterThread = iThreads.begin(); iterThread != iThreads.end();)
			{
				if ((**iterThread).finished())
					iterThread = iThreads.erase(iterThread);
				else if (!static_cast<thread_pool_thread&>(**iterThread).acquired())
				{
					auto nextTask = iWaitingTasks.back();
					iWaitingTasks.pop_back();
					iActiveTasks.push_back(nextTask);
					static_cast<thread_pool_thread&>(**iterThread).acquire(*iActiveTasks.back().first);
					exhuasted = false;
					break;
				}
				else
					++iterThread;
			}
		}
	}
}
