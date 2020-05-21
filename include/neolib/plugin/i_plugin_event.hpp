// i_plugin_event.hpp
/*
 *  Copyright (c) 2015, 2020 Leigh Johnston.
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
#include <memory>
#include <neolib/core/reference_counted.hpp>
#include <neolib/task/event.hpp>

namespace neolib
{
    namespace plugin_events
    {
        using neolib::sink;

        template <typename... Arguments>
        class i_event_callback : public i_reference_counted
        {
            typedef i_event_callback<Arguments...> self_type;
        public:
            typedef self_type abstract_type;
        public:
            virtual ~i_event_callback() = default;
        public:
            ref_ptr<i_event_callback<Arguments...>> clone() const
            {
                return ref_ptr<i_event_callback<Arguments...>>{ do_clone() };
            }
        public:
            virtual void operator()(Arguments... aArguments) const = 0;
        private:
            virtual i_event_callback<Arguments...>* do_clone() const = 0;
        };

        template <typename... Arguments>
        class event_callback : public reference_counted<i_event_callback<Arguments...>>, public std::function<void(Arguments...)>
        {
            typedef std::function<void(Arguments...)> base_type;
        public:
            using base_type::base_type;
        public:
            void operator()(Arguments... aArguments) const override
            {
                base_type::operator()(aArguments...);
            }
        private:
            i_event_callback<Arguments...>* do_clone() const override
            {
                return new event_callback{ *this };
            }
        };

        template <typename... Arguments>
        class i_event
        {
        public:
            typedef i_event_callback<Arguments...> abstract_callback;
            typedef event_callback<Arguments...> callback;
        public:
            virtual ~i_event() = default;
        public:
            virtual const neolib::i_event& raw_event() const = 0;
            virtual neolib::i_event& raw_event() = 0;
        public:
            virtual void pre_trigger() const = 0;
        public:
            virtual bool trigger(Arguments... aArguments) const = 0;
            virtual bool sync_trigger(Arguments... aArguments) const = 0;
            virtual void async_trigger(Arguments... aArguments) const = 0;
            virtual bool accepted() const = 0;
            virtual void accept() const = 0;
            virtual void ignore() const = 0;
        public:
            event_handle subscribe(const callback& aCallback, const void* aUniqueId = nullptr) const
            {
                return event_handle{ do_subscribe(aCallback, aUniqueId) };
            }
            event_handle operator()(const callback& aCallback, const void* aUniqueId = nullptr) const
            {
                return event_handle{ do_subscribe(aCallback, aUniqueId) };
            }
            template <typename T>
            event_handle subscribe(const callback& aCallback, const T* aUniqueIdObject) const
            {
                return event_handle{ do_subscribe(aCallback, static_cast<const void*>(aUniqueIdObject)) };
            }
            template <typename T>
            event_handle operator()(const callback& aCallback, const T* aUniqueIdObject) const
            {
                return event_handle{ do_subscribe(aCallback, static_cast<const void*>(aUniqueIdObject)) };
            }
            template <typename T>
            event_handle subscribe(const callback& aCallback, const T& aUniqueIdObject) const
            {
                return event_handle{ do_subscribe(aCallback, static_cast<const void*>(&aUniqueIdObject)) };
            }
            template <typename T>
            event_handle operator()(const callback& aCallback, const T& aUniqueIdObject) const
            {
                return event_handle{ do_subscribe(aCallback, static_cast<const void*>(&aUniqueIdObject)) };
            }
            void unsubscribe(event_handle aHandle) const
            {
                return do_unsubscribe(aHandle);
            }
            void unsubscribe(const void* aUniqueId) const
            {
                return do_unsubscribe(aUniqueId);
            }
            template <typename T>
            void unsubscribe(const T* aUniqueIdObject) const
            {
                return do_unsubscribe(static_cast<const void*>(aUniqueIdObject));
            }
            template <typename T>
            void unsubscribe(const T& aUniqueIdObject) const
            {
                return do_unsubscribe(static_cast<const void*>(&aUniqueIdObject));
            }
        private:
            virtual event_handle do_subscribe(const abstract_callback& aCallback, const void* aUniqueId = nullptr) const = 0;
            virtual void do_unsubscribe(event_handle aHandle) const = 0;
            virtual void do_unsubscribe(const void* aUniqueId) const = 0;
        };

        #define detail_event_subscribe( declName, ... ) \
            neolib::event_handle declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, const void* aUniqueId = nullptr) const { return declName()(aCallback, aUniqueId); }\
            neolib::event_handle declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, const void* aUniqueId = nullptr) { return declName()(aCallback, aUniqueId); }\
            template <typename T>\
            neolib::event_handle declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, const T* aUniqueObject) const { return declName()(aCallback, static_cast<const void*>(aUniqueObject)); }\
            template <typename T>\
            neolib::event_handle declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, const T* aUniqueObject) { return declName()(aCallback, static_cast<const void*>(aUniqueObject)); }\
            template <typename T>\
            neolib::event_handle declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, T&& aUniqueObject) const { return declName()(aCallback, static_cast<const void*>(&aUniqueObject)); }\
            template <typename T>\
            neolib::event_handle declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, T&& aUniqueObject) { return declName()(aCallback, static_cast<const void*>(&aUniqueObject)); }

        #define declare_event( declName, ... ) \
            virtual const neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() const = 0;\
            virtual neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() = 0;\
            const neolib::plugin_events::i_event<__VA_ARGS__>& declName() const { return ev_##declName(); }\
            neolib::plugin_events::i_event<__VA_ARGS__>& declName() { return ev_##declName(); }\
            detail_event_subscribe(declName, __VA_ARGS__)

        template <typename... Arguments>
        class event;

        #define define_declared_event( name, declName, ... ) \
            neolib::plugin_events::event<__VA_ARGS__> name; \
            const neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() const override { return name; };\
            neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() override { return name; };

        #define define_event( name, declName, ... ) \
            neolib::plugin_events::event<__VA_ARGS__> name; \
            const neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() const { return name; };\
            neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() { return name; };\
            const neolib::plugin_events::i_event<__VA_ARGS__>& declName() const { return ev_##declName(); }\
            neolib::plugin_events::i_event<__VA_ARGS__>& declName() { return ev_##declName(); }\
            detail_event_subscribe(declName, __VA_ARGS__)
   }
}