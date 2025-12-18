// mutex.hpp
/*
 *  Copyright (c) 2020-2025 Leigh Johnston.
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
#include <mutex>
#include <thread>
#include <boost/thread/locks.hpp>
#include <boost/lockfree/detail/freelist.hpp>
#include <neolib/core/i_mutex.hpp>

#if !defined(DISABLE_NEOS_PROFILE_MUTEXES)
    #define NEOS_PROFILE_MUTEXES
#endif

namespace neolib
{
    struct null_mutex : public i_lockable
    {
        void lock() noexcept final {}
        void unlock() noexcept final {}
        bool try_lock() noexcept final { return true; }
    };

    template <typename Subject>
    class proxy_mutex : public i_lockable
    {
    public:
        proxy_mutex(Subject& aSubject) :
            iSubject{ &aSubject }
        {
        }
    public:
        void lock() noexcept final
        {
            iSubject->lock();
        }
        void unlock() noexcept final
        {
            iSubject->unlock();
        }
        bool try_lock() noexcept final
        {
            return iSubject->try_lock();
        }
    private:
        Subject* iSubject;
    };

    template <typename ProfilerTag = void>
    class alignas(boost::lockfree::detail::cacheline_bytes) recursive_spinlock : public i_lockable
    {
    private:
#if defined(NEOS_PROFILE_MUTEXES)
        static inline int sIcfSingleton;
        void prevent_icf() const noexcept
        {
            static volatile int sink = 42;
            (void)sink;
            (void)sIcfSingleton;
        }
#else
        void prevent_icf() const noexcept
        {
        }
#endif

    public:
        recursive_spinlock() :
            iState{},
            iLockCount{ 0u },
            iLockingThread{}
        {
        }
        ~recursive_spinlock()
        {
            assert(!iState.test(std::memory_order_acquire));
        }
    public:
        void lock() final
        {
            prevent_icf();
            auto const thisThread = std::this_thread::get_id();
            if (iState.test(std::memory_order_acquire) && iLockingThread.load(std::memory_order_relaxed) == thisThread)
            {
                ++iLockCount;
                return;
            }
#if !defined(NEOS_PROFILE_MUTEXES)
            while (iState.test_and_set(std::memory_order_acquire))
                iState.wait(true, std::memory_order_relaxed);
#else
            if (!iThrowOnPathologicalContention)
            {
                while (iState.test_and_set(std::memory_order_acquire))
                    iState.wait(true, std::memory_order_relaxed);
            }
            else
            {
                auto previousLockingThread = iLockingThread.load();
                auto const start = std::chrono::high_resolution_clock::now();
                while (iState.test_and_set(std::memory_order_acquire))
                {
                    previousLockingThread = iLockingThread.load();
                    iState.wait(true, std::memory_order_relaxed);
                }
                auto const end = std::chrono::high_resolution_clock::now();
                if (end - start > iPathologicalContentionCounterTimeout.load())
                    if (++iPathologicalContentionCounter > iPathologicalContentionCounterMaxCount.load())
                        throw pathological_contention(previousLockingThread);
            }
#endif
            iLockingThread.store(thisThread, std::memory_order_relaxed);
            ++iLockCount;
        }
        void unlock() noexcept final
        {
            prevent_icf();
            if (--iLockCount == 0u)
            {
                iLockingThread.store(std::thread::id{}, std::memory_order_relaxed);
                iState.clear(std::memory_order_release);
                iState.notify_one();
            }
        }
        bool try_lock() noexcept final
        {
            prevent_icf();
            auto const thisThread = std::this_thread::get_id();
            if (iState.test(std::memory_order_acquire) && iLockingThread.load(std::memory_order_relaxed) == thisThread)
            {
                ++iLockCount;
                return true;
            }
            if (iState.test_and_set(std::memory_order_acquire))
                return false;
            iLockingThread.store(thisThread, std::memory_order_relaxed);
            ++iLockCount;
            return true;
        }
        void throw_on_pathological_contention(std::chrono::microseconds aTimeout = std::chrono::microseconds{ 100 }, std::uint32_t aMaxCount = 10u) noexcept final
        {
#if defined(NEOS_PROFILE_MUTEXES)
            iPathologicalContentionCounterTimeout = aTimeout;
            iPathologicalContentionCounterMaxCount = aMaxCount;
            iThrowOnPathologicalContention = true;
#endif
        }
    private:
        std::atomic_flag iState;
        std::atomic<std::uint32_t> iLockCount;
        std::atomic<std::thread::id> iLockingThread;
#if defined(NEOS_PROFILE_MUTEXES)
        std::atomic<bool> iThrowOnPathologicalContention = false;
        std::atomic<std::chrono::microseconds> iPathologicalContentionCounterTimeout;
        std::atomic<std::uint32_t> iPathologicalContentionCounterMaxCount;
        std::atomic<std::uint32_t> iPathologicalContentionCounter = 0u;
#endif
    };

    template <typename ProfilerTag = void>
    class alignas(boost::lockfree::detail::cacheline_bytes) switchable_mutex : public i_lockable
    {
    public:
        switchable_mutex()
        {
            set_multi_threaded();
        }
    public:
        void set_single_threaded()
        {
            iActiveMutex.emplace<neolib::null_mutex>();
        }
        void set_multi_threaded()
        {
            iActiveMutex.emplace<std::recursive_mutex>();
        }
        void set_multi_threaded_spinlock()
        {
            iActiveMutex.emplace<neolib::recursive_spinlock<ProfilerTag>>();
        }
    public:
        void lock() final
        {
            if (std::holds_alternative<std::recursive_mutex>(iActiveMutex))
                std::get<std::recursive_mutex>(iActiveMutex).lock();
            else if (std::holds_alternative<neolib::recursive_spinlock<ProfilerTag>>(iActiveMutex))
                std::get<neolib::recursive_spinlock<ProfilerTag>>(iActiveMutex).lock();
            else
                std::get<neolib::null_mutex>(iActiveMutex).lock();
        }
        void unlock() noexcept final
        {
            if (std::holds_alternative<std::recursive_mutex>(iActiveMutex))
                std::get<std::recursive_mutex>(iActiveMutex).unlock();
            else if (std::holds_alternative<neolib::recursive_spinlock<ProfilerTag>>(iActiveMutex))
                std::get<neolib::recursive_spinlock<ProfilerTag>>(iActiveMutex).unlock();
            else
                std::get<neolib::null_mutex>(iActiveMutex).unlock();
        }
        bool try_lock() noexcept final
        {
            if (std::holds_alternative<std::recursive_mutex>(iActiveMutex))
                return std::get<std::recursive_mutex>(iActiveMutex).try_lock();
            else if (std::holds_alternative<neolib::recursive_spinlock<ProfilerTag>>(iActiveMutex))
                return std::get<neolib::recursive_spinlock<ProfilerTag>>(iActiveMutex).try_lock();
            else
                return std::get<neolib::null_mutex>(iActiveMutex).try_lock();
        }
        void throw_on_pathological_contention(std::chrono::microseconds aTimeout = std::chrono::microseconds{ 100 }, std::uint32_t aMaxCount = 10u) noexcept final
        {
            if (std::holds_alternative<neolib::recursive_spinlock<ProfilerTag>>(iActiveMutex))
                std::get<neolib::recursive_spinlock<ProfilerTag>>(iActiveMutex).throw_on_pathological_contention(aTimeout, aMaxCount);
        }
    private:
        std::variant<std::recursive_mutex, neolib::recursive_spinlock<ProfilerTag>, neolib::null_mutex> iActiveMutex;
    };

    template <typename Mutexes>
    class scoped_multi_lock
    {
    public:
        scoped_multi_lock(Mutexes& aMutexes) :
            iMutexes{ aMutexes }
        {
            boost::lock(iMutexes.begin(), iMutexes.end());
        }
        ~scoped_multi_lock()
        {
            for (auto& m : iMutexes)
                m.unlock();
        }
    private:
        Mutexes& iMutexes;
    };
}
