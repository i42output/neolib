// i_logger.hpp
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
#include <functional>
#include <sstream>
#include <neolib/core/string.hpp>
#include <neolib/app/services.hpp>

namespace neolib
{
    namespace logger
    {
        enum class severity : uint32_t
        {
            Trace       = 0,
            Debug       = 1,
            Info        = 2,
            Warning     = 3,
            Error       = 4,
            Fatal       = 5
        };

        enum class category_id : int32_t {};

        struct category
        {
            category_id id;
            template <typename IdType>
            category(IdType aId) :
                id{ static_cast<category_id>(aId) }
            {
            }
        };

        typedef uint64_t line_id_t;
        constexpr line_id_t DefaultInitialLineId = 1ull;

        struct endl_t {};
        struct flush_t {};

        const endl_t endl;
        const flush_t flush;

        class client_logger_buffers
        {
        protected:
            typedef std::ostringstream buffer_t;
        private:
            typedef std::map<std::thread::id, buffer_t> buffer_list_t;
        public:
            static client_logger_buffers& instance()
            {
                static client_logger_buffers sIntance;
                return sIntance;
            }
        public:
            buffer_t& buffer()
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                thread_local struct cleanup
                {
                    client_logger_buffers& parent;
                    ~cleanup()
                    {
                        std::lock_guard<std::recursive_mutex> lg{ parent.mutex() };
                        parent.buffers().erase(std::this_thread::get_id());
                    }
                } cleanup{ *this };
                return iBuffers[std::this_thread::get_id()];
            }
            buffer_list_t& buffers()
            {
                return iBuffers;
            }
        private:
            std::recursive_mutex& mutex() const
            {
                return iMutex;
            }
        private:
            mutable std::recursive_mutex iMutex;
            buffer_list_t iBuffers;
        };

        class i_logger;

        class i_formatter
        {
        public:
            virtual ~i_formatter() = default;
        public:
            virtual void format(i_logger const& aLogger, i_string const& aUnformattedMessage, i_string& aFormattedMessage) = 0;
        };

        class i_logger : public i_service
        {
            template <std::size_t Instance>
            friend class logger;
        public:
            struct logging_thread_already_created : std::logic_error { logging_thread_already_created() : std::logic_error{ "neolib::logger::i_logger::logging_thread_already_created" } {} };
            struct no_formatter : std::logic_error { no_formatter() : std::logic_error{ "neolib::logger::i_logger::no_formatter" } {} };
        public:
            virtual ~i_logger() = default;
        public:
            virtual void copy_to(i_logger& aLogger) = 0;
            virtual void cancel_copy_to(i_logger& aLogger) = 0;
            virtual bool has_logging_thread() const = 0;
            virtual void create_logging_thread() = 0;
        public:
            virtual severity filter_severity() const = 0;
            virtual void set_filter_severity(severity aSeverity) = 0;
            virtual void register_category(category_id aId, i_string const& aName) = 0;
            virtual bool category_enabled(category_id aId) const = 0;
            virtual void enable_category(category_id aId) = 0;
            virtual void disable_category(category_id aId) = 0;
        public:
            virtual bool has_formatter() const = 0;
            virtual i_formatter& formatter() const = 0;
            virtual void set_formatter(i_formatter& aFormatter) = 0;
            virtual void clear_formatter() = 0;
        public:
            virtual line_id_t line_id() const = 0;
            virtual void reset_line_id(line_id_t aLineId = DefaultInitialLineId) = 0;
        public:
            virtual i_logger& operator<<(severity aSeverity) = 0;
            virtual i_logger& operator<<(category_id aCategory) = 0;
        public:
            template <typename IdType>
            void register_category(IdType aId, std::string const& aName = {})
            {
                register_category(static_cast<category_id>(aId), string{ aName });
            }
            template <typename IdType>
            void category_enabled(IdType aId)
            {
                category_enabled(static_cast<category_id>(aId));
            }
            template <typename IdType>
            void enable_category(IdType aId)
            {
                enable_category(static_cast<category_id>(aId));
            }
            template <typename IdType>
            void disable_category(IdType aId)
            {
                disable_category(static_cast<category_id>(aId));
            }
        public:
            i_logger& operator<<(category aCategory)
            {
                return (*this) << aCategory.id;
            }
            i_logger& operator<<(endl_t)
            {
                auto& buffer = client_logger_buffers::instance().buffer();
                buffer << std::endl;
                flush(string{ buffer.str() });
                buffer.str({});
                return *this;
            }
            i_logger& operator<<(flush_t)
            {
                auto& buffer = client_logger_buffers::instance().buffer();
                buffer << std::flush;
                flush(string{ buffer.str() });
                buffer.str({});
                return *this;
            }
            template<typename T>
            i_logger& operator<<(T const& aValue)
            {
                client_logger_buffers::instance().buffer() << aValue;
                return *this;
            }
        public:
            virtual void commit() = 0;
            virtual void wait() const = 0;
        protected:
            virtual void flush(i_string const& aMessage) = 0;
        };

        class formatter : public i_formatter
        {
        public:
            typedef std::function<void(i_logger const&, i_string const&, i_string&)> function_type;
        public:
            formatter(function_type aFormattingFunction) :
                iFormattingFunction{ aFormattingFunction }
            {
            }
        public:
            void format(i_logger const& aLogger, i_string const& aUnformattedMessage, i_string& aFormattedMessage) override
            {
                iFormattingFunction(aLogger, aUnformattedMessage, aFormattedMessage);
            }
        private:
            function_type iFormattingFunction;
        };
    }
}
