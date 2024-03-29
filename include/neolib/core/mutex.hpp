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

// Based on code that is:
//
//          Copyright Oliver Kowalke 2016.
//
// Boost Software License - Version 1.0 - August 17th, 2003
// 
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license(the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third - parties to whom the Software is furnished to
// do so, all subject to the following :
// 
// The copyright notices in the Softwareand this entire statement, including
// the above license grant, this restrictionand the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine - executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#pragma once

#include <neolib/neolib.hpp>
#include <atomic>
#include <mutex>
#include <boost/thread/locks.hpp>
#include <boost/lockfree/detail/prefix.hpp>
#include <boost/fiber/detail/spinlock.hpp>
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

    using boost::fibers::detail::spinlock_status;

    using spinlock = boost::fibers::detail::spinlock;

    class alignas(BOOST_LOCKFREE_CACHELINE_BYTES) recursive_spinlock : public i_lockable
    {
    public:
        recursive_spinlock() :
            iState{ spinlock_status::unlocked },
            iLockingThread{},
            iLockCount{ 0u },
            iGenerator{ std::random_device{}() }
        {
        }
        ~recursive_spinlock()
        {
            assert(iState.load(std::memory_order_acquire) == spinlock_status::unlocked);
        }
    public:
        void lock() noexcept final
        {
            auto const thisThread = std::this_thread::get_id();
            if (iState.load(std::memory_order_acquire) == spinlock_status::locked && iLockingThread.load(std::memory_order_acquire) == thisThread)
            {
                ++iLockCount;
                return;
            }
            std::size_t collisions = 0;
            for (;;) 
            {
                std::size_t retries = 0;
                while (spinlock_status::locked == iState.load(std::memory_order_relaxed)) 
                {
                    if (BOOST_FIBERS_SPIN_BEFORE_SLEEP0 > retries) 
                    {
                        ++retries;
                        cpu_relax();
                    }
                    else if (BOOST_FIBERS_SPIN_BEFORE_YIELD > retries) 
                    {
                        static constexpr std::chrono::microseconds us0{ 0 };
                        std::this_thread::sleep_for(us0);
                    }
                    else 
                    {
                        std::this_thread::yield();
                    }
                }
                if (spinlock_status::locked == iState.exchange(spinlock_status::locked, std::memory_order_acquire)) 
                {
                    std::uniform_int_distribution< std::size_t > distribution
                    {
                        0, static_cast<std::size_t>(1) << (std::min)(collisions, static_cast<std::size_t>(BOOST_FIBERS_CONTENTION_WINDOW_THRESHOLD)) 
                    };
                    const std::size_t z = distribution(iGenerator);
                    ++collisions;
                    for (std::size_t i = 0; i < z; ++i) 
                    {
                        cpu_relax();
                    }
                }
                else 
                {
                    iLockingThread.store(thisThread);
                    ++iLockCount;
                    break;
                }
            }
        }
        void unlock() noexcept final
        {
            if (--iLockCount == 0u)
            {
                iLockingThread.store(std::thread::id{}, std::memory_order_release);
                iState.store(spinlock_status::unlocked, std::memory_order_release);
            }
        }
        bool try_lock() noexcept final
        {
            auto const thisThread = std::this_thread::get_id();
            if (iState.load(std::memory_order_acquire) == spinlock_status::locked && iLockingThread.load(std::memory_order_acquire) == thisThread)
            {
                ++iLockCount;
                return true;
            }
            bool locked = (spinlock_status::unlocked == iState.exchange(spinlock_status::locked, std::memory_order_acquire));
            if (locked)
            {
                iLockingThread.store(thisThread);
                ++iLockCount;
            }
            return locked;
        }
    private:
        std::atomic<spinlock_status> iState;
        std::atomic<std::thread::id> iLockingThread;
        std::atomic<uint32_t> iLockCount;
        std::minstd_rand iGenerator;
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
