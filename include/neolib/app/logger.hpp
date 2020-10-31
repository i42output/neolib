// logger.hpp
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
#include <map>
#include <thread>
#include <mutex>
#include <string>
#include <neolib/core/lifetime.hpp>
#include <neolib/app/i_logger.hpp>

namespace neolib
{
    namespace logger
    {
        class logger : public i_logger, public lifetime<>
        {
        protected:
            typedef std::string buffer_t;
        private:
            typedef std::map<std::thread::id, buffer_t> buffer_list_t;
        public:
            logger()
            {
            }
            ~logger()
            {
                set_destroying();
            }
        public:
            void create_logging_thread()
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                iLoggingThread.emplace([&]()
                {
                    for(;;)
                    {
                        std::unique_lock<std::mutex> lk(commit_signal_mutex());
                        iCommitSignal.wait(lk, [&]() { return any_available() || is_destroying(); });
                        commit();
                        if (is_destroying())
                            break;
                    };
                });
            }
        public:
            severity filter_severity() const override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                return iFilterSeverity;
            }
            void set_filter_severity(severity aSeverity) override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                iFilterSeverity = aSeverity;
            }
        public:
            i_logger& operator<<(severity aSeverity) override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                set_message_severity(aSeverity);
                return *this;
            }
        protected:
            std::recursive_mutex& mutex() const
            {
                return iMutex;
            }
            std::mutex& commit_signal_mutex() const
            {
                return iCommitSignalMutex;
            }
            std::condition_variable& commit_signal() const
            {
                return iCommitSignal;
            }
            severity message_severity() const
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                return iMessageSeverity;
            }
            void set_message_severity(severity aMessageSeverity)
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                iMessageSeverity = aMessageSeverity;
            }
        public:
            void commit() override
            {
                if (!iLoggingThread || std::this_thread::get_id() == iLoggingThread->get_id())
                {
                    std::lock_guard<std::recursive_mutex> lg{ mutex() };
                    for (auto& entry : buffers())
                    {
                        auto& buffer = entry.second;
                        commit(buffer);
                        buffer = {};
                    }
                }
                else
                {
                    commit_signal().notify_one();
                    iLoggingThread->join();
                }
            }
        protected:
            void flush(i_string const& aMessage) override
            {
                bool notify = false;
                { 
                    std::scoped_lock lg{ mutex(), commit_signal_mutex() };
                    if (message_severity() >= filter_severity())
                    {
                        buffer() += aMessage.to_std_string_view();
                        notify = true;
                    }
                }
                if (notify)
                    commit_signal().notify_one();
            }
        protected:
            virtual void commit(buffer_t const& aBuffer) = 0;
        private:
            bool any_available() const
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                for (auto& entry : buffers())
                {
                    auto& buffer = entry.second;
                    if (!buffer.empty())
                        return true;
                }
                return false;
            }
            buffer_t const& buffer() const
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                thread_local struct cleanup
                {
                    logger const& parent;
                    destroyed_flag parentDestroyed;
                    ~cleanup()
                    {
                        if (!parentDestroyed)
                        {
                            std::lock_guard<std::recursive_mutex> lg{ parent.mutex() };
                            parent.buffers().erase(std::this_thread::get_id());
                        }
                    }
                } cleanup{ *this, *this };
                return iBuffers[std::this_thread::get_id()];
            }
            buffer_t& buffer()
            {
                return const_cast<buffer_t&>(const_cast<logger const&>(*this).buffer());
            }
            buffer_list_t& buffers() const
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                return iBuffers;
            }
        private:
            mutable std::recursive_mutex iMutex;
            mutable std::mutex iCommitSignalMutex;
            mutable std::condition_variable iCommitSignal;
            std::optional<std::thread> iLoggingThread;
            severity iFilterSeverity = severity::Info;
            severity iMessageSeverity = severity::Info;
            mutable buffer_list_t iBuffers;
        };
    }
}
