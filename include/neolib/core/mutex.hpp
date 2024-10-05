// mutex.hpp
/*
 *  Copyright (c) 2020 Leigh Johnston.
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
#include <boost/thread/locks.hpp>
#include <boost/fiber/detail/cpu_relax.hpp>
#include <boost/lockfree/detail/freelist.hpp>
#include <neolib/core/i_mutex.hpp>

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

    class alignas(BOOST_LOCKFREE_CACHELINE_BYTES) recursive_spinlock : public i_lockable
    {
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
        void lock() noexcept final
        {
            auto const thisThread = std::this_thread::get_id();
            if (iState.test(std::memory_order_acquire) && iLockingThread.load(std::memory_order_acquire) == thisThread)
            {
                ++iLockCount;
                return;
            }
            while (iState.test_and_set(std::memory_order_acquire))
                iState.wait(true, std::memory_order_relaxed);
            iLockingThread.store(thisThread);
            ++iLockCount;
        }
        void unlock() noexcept final
        {
            if (--iLockCount == 0u)
            {
                iLockingThread.store(std::thread::id{}, std::memory_order_release);
                iState.clear(std::memory_order_release);
                iState.notify_one();
            }
        }
        bool try_lock() noexcept final
        {
            auto const thisThread = std::this_thread::get_id();
            if (iState.test(std::memory_order_acquire) && iLockingThread.load(std::memory_order_acquire) == thisThread)
            {
                ++iLockCount;
                return true;
            }
            if (iState.test_and_set(std::memory_order_acquire))
                return false;
            iLockingThread.store(thisThread);
            ++iLockCount;
            return true;
        }
    private:
        std::atomic_flag iState;
        std::atomic<std::uint32_t> iLockCount;
        std::atomic<std::thread::id> iLockingThread;
    };

    class alignas(BOOST_LOCKFREE_CACHELINE_BYTES) switchable_mutex : public i_lockable
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
            iActiveMutex.emplace<neolib::recursive_spinlock>();
        }
    public:
        void lock() noexcept final
        {
            if (std::holds_alternative<std::recursive_mutex>(iActiveMutex))
                std::get<std::recursive_mutex>(iActiveMutex).lock();
            else if (std::holds_alternative<neolib::recursive_spinlock>(iActiveMutex))
                std::get<neolib::recursive_spinlock>(iActiveMutex).lock();
            else
                std::get<neolib::null_mutex>(iActiveMutex).lock();
        }
        void unlock() noexcept final
        {
            if (std::holds_alternative<std::recursive_mutex>(iActiveMutex))
                std::get<std::recursive_mutex>(iActiveMutex).unlock();
            else if (std::holds_alternative<neolib::recursive_spinlock>(iActiveMutex))
                std::get<neolib::recursive_spinlock>(iActiveMutex).unlock();
            else
                std::get<neolib::null_mutex>(iActiveMutex).unlock();
        }
        bool try_lock() noexcept final
        {
            if (std::holds_alternative<std::recursive_mutex>(iActiveMutex))
                return std::get<std::recursive_mutex>(iActiveMutex).try_lock();
            else if (std::holds_alternative<neolib::recursive_spinlock>(iActiveMutex))
                return std::get<neolib::recursive_spinlock>(iActiveMutex).try_lock();
            else
                return std::get<neolib::null_mutex>(iActiveMutex).try_lock();
        }
    private:
        std::variant<std::recursive_mutex, neolib::recursive_spinlock, neolib::null_mutex> iActiveMutex;
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
