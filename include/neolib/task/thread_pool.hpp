// thread_pool.hpp
/*
 *  Copyright (c) 2007 Leigh Johnston.
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
#include <atomic>
#include <memory>
#include <vector>
#include <deque>
#include <future>
#include <mutex>
#include <neolib/task/i_thread.hpp>
#include <neolib/task/task.hpp>

namespace neolib
{
    class thread_pool_thread;

    class NEOLIB_EXPORT thread_pool
    {
        friend class thread_pool_thread;
    public:
        typedef std::shared_ptr<i_task> task_pointer;
    public:
        struct no_threads : std::logic_error { no_threads() : std::logic_error("neolib::thread_pool::no_threads") {} };
        struct task_not_found : std::logic_error { task_not_found() : std::logic_error("neolib::thread_pool::task_not_found") {} };
    private:
        typedef std::vector<std::unique_ptr<i_thread>> thread_list;
    public:
        thread_pool();
        ~thread_pool();
    public:
        void reserve(std::size_t aMaxThreads);
        std::size_t active_threads() const;
        std::size_t available_threads() const;
        std::size_t total_threads() const;
        std::size_t max_threads() const;
    public:
        void start(i_task& aTask, int32_t aPriority = 0);
        void start(task_pointer aTask, int32_t aPriority = 0);
        bool try_start(i_task& aTask, int32_t aPriority = 0);
        bool try_start(task_pointer aTask, int32_t aPriority = 0);
        std::pair<std::future<void>, task_pointer> run(std::function<void()> aFunction, int32_t aPriority = 0);
        template <typename T>
        std::pair<std::future<T>, task_pointer> run(std::function<T()> aFunction, int32_t aPriority = 0);
    public:
        bool idle() const;
        void update_idle();
        bool busy() const;
        void wait() const;
        bool stopped() const;
        void stop();
    public:
        static thread_pool& default_thread_pool();
        std::recursive_mutex& mutex() const;
    private:
        void steal_work(thread_pool_thread& aIdleThread);
        void thread_gone_idle();
        void thread_gone_busy();
    private:
        mutable std::recursive_mutex iMutex;
        std::atomic<bool> iIdle;
        std::atomic<bool> iStopped;
        std::size_t iMaxThreads;
        thread_list iThreads;
        mutable std::mutex iWaitMutex;
        mutable std::condition_variable iWaitConditionVariable;
    };

    template <typename T>
    inline std::pair<std::future<T>, thread_pool::task_pointer> thread_pool::run(std::function<T()> aFunction, int32_t aPriority)
    {
        if (stopped())
            return {};
        auto newTask = std::make_shared<function_task<T>>(aFunction);
        start(newTask, aPriority);
        return std::make_pair(newTask->get_future(), newTask);
    }

    template <typename Container>
    inline void parallel_apply(thread_pool& aThreadPool, Container& aContainer, std::function<void(typename Container::value_type& aElement)> aFunction, std::size_t aMinimumParallelismCount = 0)
    {
        if (aThreadPool.stopped())
            return;
        if (aContainer.size() < aMinimumParallelismCount)
        {
            for (auto& e : aContainer)
                aFunction(e);
            return;
        }
        auto subrange = aContainer.size() / aThreadPool.max_threads();
        if (subrange < 1)
            subrange = 1;
        auto next = aContainer.begin();
        for (auto left = aContainer.size(); left >= subrange; left -= subrange)
        {
            auto end = std::next(next, subrange);
            aThreadPool.run([next, end, &aFunction]()
            {
                for (auto i = next; i != end; ++i)
                    aFunction(*i);
            });
            next = end;
        }
        if (next != aContainer.end())
            aThreadPool.run([next, &aContainer, &aFunction]()
            {
                for (auto i = next; i != aContainer.end(); ++i)
                    aFunction(*i);
            });
        aThreadPool.wait();
    }
}
