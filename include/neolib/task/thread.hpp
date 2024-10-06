// thread.hpp v3.0
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
#include <stdexcept>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <boost/fiber/detail/cpu_relax.hpp>
#include <neolib/core/noncopyable.hpp>
#include <neolib/task/waitable.hpp>
#include <neolib/task/waitable_event.hpp>
#include <neolib/task/i_thread.hpp>

namespace neolib
{
    class NEOLIB_EXPORT thread : public i_thread, public waitable, private noncopyable
    {
        // types
    public:
        typedef std::thread::id id_type;
        typedef std::thread thread_object_type;
        // exceptions
    public:
        struct thread_not_started : public std::logic_error { thread_not_started() : std::logic_error("neolib::thread::thread_not_started") {} };
        struct thread_already_started : public std::logic_error { thread_already_started() : std::logic_error("neolib::thread::thread_already_started") {} };
        struct cannot_wait_on_self : public std::logic_error { cannot_wait_on_self() : std::logic_error("neolib::thread::cannot_wait_on_self") {} };
        struct no_thread_object : public std::logic_error { no_thread_object() : std::logic_error("neolib::thread::no_thread_object") {} };
        struct not_in_thread : public std::logic_error { not_in_thread() : std::logic_error("neolib::thread::not_in_thread") {} };
    private:
        typedef std::unique_ptr<thread_object_type> thread_object_pointer;
    protected:
        struct cancellation {};
        // construction
    public:
        thread(const std::string& aName = "", bool aAttachToCurrentThread = false);
        thread(std::function<void()> aExecFunction, const std::string& aName = "");
        virtual ~thread();
        // operations
    public:
        const std::string& name() const noexcept override;
        bool using_existing_thread() const noexcept;
        void start();
        void cancel();
        void abort(bool aWait = true) override;
        void wait() const;
        wait_result wait(const waitable_event_list& aEventList) const;
        bool msg_wait(const i_message_queue& aMessageQueue) const;
        wait_result msg_wait(const i_message_queue& aMessageQueue, const waitable_event_list& aEventList) const;
        void block();
        void unblock();
        thread_state state() const noexcept override;
        bool started() const noexcept;
        bool running() const noexcept;
        bool finished() const noexcept override;
        bool aborted() const noexcept;
        bool cancelled() const noexcept;
        bool error() const noexcept;
        id_type id() const noexcept;
        bool in() const;
        bool blocked() const noexcept;
        bool has_thread_object() const noexcept;
        thread_object_type& thread_object() const;
        static void sleep(const std::chrono::duration<double, std::milli>& aDuration);
        static void yield() noexcept;
        static void relax() noexcept;
        static uint64_t elapsed_ms() noexcept;
        static uint64_t elapsed_us() noexcept;
        static uint64_t elapsed_ns() noexcept;
        static uint64_t program_elapsed_ms() noexcept;
        static uint64_t program_elapsed_us() noexcept;
        static uint64_t program_elapsed_ns() noexcept;
        // implementation
    private:
        // from waitable
        bool waitable_ready() const noexcept override;
        // own
        void exec_preamble() override;
        void exec(yield_type aYieldType = yield_type::NoYield) override;
        void entry_point();
        // attributes
    private:
        mutable std::recursive_mutex iMutex;
        const std::string iName;
        bool iUsingExistingThread;
        std::optional<std::function<void()>> iExecFunction;
        std::atomic<thread_state> iState;
        thread_object_pointer iThreadObject;
        id_type iId;
        std::atomic<std::size_t> iBlockedCount;
    };

    inline void thread::relax() noexcept
    {
        cpu_relax();
    }
}
