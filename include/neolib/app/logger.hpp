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
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <memory>
#include <chrono>
#include <neolib/core/lifetime.hpp>
#include <neolib/app/i_logger.hpp>

namespace neolib
{
    namespace logger
    {
        template <std::size_t Instance = 0>
        class logger : public i_logger, public lifetime<>
        {
            typedef logger<Instance> self_type;
        public:
            define_declared_event(NewLogMessage, new_log_message, i_string const&)
        protected:
            typedef std::string buffer_t;
        private:
            typedef std::map<category_id, std::pair<bool, std::string>> category_map_t;
            typedef std::map<std::thread::id, std::pair<std::shared_ptr<buffer_t>, std::shared_ptr<buffer_t>>> buffer_list_t;
            typedef std::vector<i_logger*> copy_list_t;
        public:
            logger()
            {
            }
            ~logger()
            {
                set_destroying();
            }
        public:
            void copy_to(i_logger& aLogger) override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                copies().push_back(&aLogger);
            }
            void cancel_copy_to(i_logger& aLogger) override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                copies().erase(std::remove(copies().begin(), copies().end(), &aLogger), copies().end());
            }
            bool has_logging_thread() const override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                return iLoggingThread != std::nullopt;
            }
            void create_logging_thread() override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                if (iLoggingThread)
                    throw logging_thread_already_created();
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
            using i_logger::register_category;
            using i_logger::category_enabled;
            using i_logger::enable_category;
            using i_logger::disable_category;
            void register_category(category_id aId, i_string const& aName) override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                iCategories[aId].first = true;
                iCategories[aId].second = aName.to_std_string_view();
                for (auto& copy : copies())
                    copy->register_category(aId, aName);
            }
            bool category_enabled(category_id aId) const override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                auto existing = iCategories.find(aId);
                return existing != iCategories.end() && existing->second.first;
            }
            void enable_category(category_id aId) override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                auto existing = iCategories.find(aId);
                if (existing != iCategories.end())
                    existing->second.first = true;
                for (auto& copy : copies())
                    copy->enable_category(aId);
            }
            void disable_category(category_id aId) override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                auto existing = iCategories.find(aId);
                if (existing != iCategories.end())
                    existing->second.first = false;
                for (auto& copy : copies())
                    copy->disable_category(aId);
            }
        public:
            bool has_formatter() const override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                return iFormatter != nullptr;
            }
            i_formatter& formatter() const override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                if (iFormatter != nullptr)
                    return *iFormatter;
                throw no_formatter();
            }
            void set_formatter(i_formatter& aFormatter) override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                iFormatter = std::shared_ptr<i_formatter>{ std::shared_ptr<i_formatter>{}, &aFormatter };
            }
            void clear_formatter() override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                iFormatter = nullptr;
            }
        public:
            line_id_t line_id() const override
            {
                return iLineId;
            }
            void reset_line_id(line_id_t aLineId = DefaultInitialLineId) override
            {
                iLineId = aLineId;
            }
        public:
            using i_logger::operator<<;
            i_logger& operator<<(severity aSeverity) override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                set_message_severity(aSeverity);
                for (auto& copy : copies())
                    (*copy) << aSeverity;
                return *this;
            }
            i_logger& operator<<(category_id aCategory) override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                set_message_category(aCategory);
                for (auto& copy : copies())
                    (*copy) << aCategory;
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
            void join_logging_thread()
            {
                if (iLoggingThread)
                    iLoggingThread->join();
            }
            severity message_severity() const
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                return message_severity_ref();
            }
            void set_message_severity(severity aMessageSeverity)
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                message_severity_ref() = aMessageSeverity;
            }
            category_id message_category() const
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                return message_category_ref();
            }
            void set_message_category(category_id aid)
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                message_category_ref() = aid;
            }
            bool message_category_enabled() const
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                auto existing = iCategories.find(message_category());
                return existing == iCategories.end() || existing->second.first;
            }
        public:
            void commit() override
            {
                if (!iLoggingThread || std::this_thread::get_id() == iLoggingThread->get_id())
                {
                    {
                        std::lock_guard<std::recursive_mutex> lg{ mutex() };
                        for (auto& entry : buffers())
                        {
                            auto& buffers = entry.second;
                            std::swap(buffers.first, buffers.second);
                        }
                    }
                    thread_local buffer_t tempBuffer;
                    for (auto& entry : buffers())
                    {
                        auto& buffer = *entry.second.second;
                        if (!buffer.empty())
                        {
                            tempBuffer += buffer;
                            buffer.clear();
                        }
                    }
                    commit(tempBuffer);
                    tempBuffer.clear();
                }
                else
                    commit_signal().notify_one();
            }
            void wait() const override
            {
                if (iLoggingThread)
                {
                    while (any_available())
                    {
                        using namespace std::chrono_literals;
                        std::this_thread::sleep_for(10ms);
                    }
                }
            }
        protected:
            void flush(i_string const& aMessage) override
            {
                bool notify = false;
                { 
                    std::scoped_lock lg{ mutex(), commit_signal_mutex() };
                    if (message_severity() >= filter_severity() && message_category_enabled())
                    {
                        if (!has_formatter())
                        {
                            buffer() += aMessage.to_std_string_view();
                            NewLogMessage.trigger(aMessage);
                        }
                        else
                        {
                            thread_local string tempFormattedMessage;
                            formatter().format(*this, aMessage, tempFormattedMessage);
                            buffer() += tempFormattedMessage.to_std_string_view();
                            NewLogMessage.trigger(tempFormattedMessage);
                            tempFormattedMessage.clear();
                        }
                        ++iLineId;
                        notify = true;
                    }
                    for (auto& copy : copies())
                        copy->flush(aMessage);
                }
                if (notify)
                    commit_signal().notify_one();
            }
        protected:
            virtual void commit(buffer_t const& aBuffer) = 0;
        protected:
            void finalize()
            {
                if (has_logging_thread())
                {
                    wait();
                    {
                        std::unique_lock<std::mutex> lk(commit_signal_mutex());
                        set_destroying();
                    }
                    commit_signal().notify_one();
                    join_logging_thread();
                }
                else
                {
                    set_destroying();
                    commit();
                }
            }
        private:
            severity const& message_severity_ref() const
            {
                thread_local severity tMessageSeverity = severity::Info;
                return tMessageSeverity;
            }
            severity& message_severity_ref()
            {
                return const_cast<severity&>(const_cast<self_type const&>(*this).message_severity_ref());
            }
            category_id const& message_category_ref() const
            {
                thread_local category_id tMessageCategory = {};
                return tMessageCategory;
            }
            category_id& message_category_ref()
            {
                return const_cast<category_id&>(const_cast<self_type const&>(*this).message_category_ref());
            }
            bool any_available() const
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                for (auto& entry : buffers())
                {
                    auto& buffer = *entry.second.first;
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
                            parent.wait();
                            std::lock_guard<std::recursive_mutex> lg{ parent.mutex() };
                            parent.buffers().erase(std::this_thread::get_id());
                        }
                    }
                } cleanup{ *this, *this };
                auto existing = iBuffers.find(std::this_thread::get_id());
                if (existing != iBuffers.end())
                    return *existing->second.first;
                existing = iBuffers.insert(buffer_list_t::value_type{ std::this_thread::get_id(), buffer_list_t::mapped_type{ std::make_shared<buffer_t>(), std::make_shared<buffer_t>() } }).first;
                return *existing->second.first;
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
            copy_list_t const& copies() const
            {
                return iCopies;
            }
            copy_list_t& copies()
            {
                return iCopies;
            }
        private:
            mutable std::recursive_mutex iMutex;
            mutable std::mutex iCommitSignalMutex;
            mutable std::condition_variable iCommitSignal;
            std::optional<std::thread> iLoggingThread;
            severity iFilterSeverity = severity::Info;
            category_map_t iCategories;
            std::shared_ptr<i_formatter> iFormatter;
            line_id_t iLineId = DefaultInitialLineId;
            mutable buffer_list_t iBuffers;
            copy_list_t iCopies;
        public:
            static uuid const& iid() { static uuid const sIid{ Instance + 0x442ed95b, 0x215c, 0x4b6e, 0xb945, { 0xf9, 0x61, 0xc4, 0xca, 0xd8, 0x7b } }; return sIid; }
        };
    }
}
