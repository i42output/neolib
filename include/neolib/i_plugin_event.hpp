// i_plugin_event.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <neolib/neolib.hpp>
#include <memory>

namespace neolib
{
    namespace plugin_events
    {
        template <typename... Arguments>
        class i_event_handle
        {
        public:
            virtual ~i_event_handle() {}
        };

        template <typename... Arguments>
        class i_event_callback
        {
        public:
            virtual ~i_event_callback() {}
        public:
            std::unique_ptr<i_event_callback<Arguments...>> clone() const
            {
                return std::unique_ptr<i_event_callback<Arguments...>>{ do_clone() };
            }
        public:
            virtual void operator()(Arguments... aArguments) const = 0;
        private:
            virtual i_event_callback<Arguments...>* do_clone() const = 0;
        };

        template <typename... Arguments>
        class event_callback : public i_event_callback<Arguments...>, public std::function<void(Arguments...)>
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
            typedef i_event_handle<Arguments...> handle;
            typedef std::unique_ptr<handle> handle_ptr;
            typedef i_event_callback<Arguments...> callback;
            typedef event_callback<Arguments...> concrete_callback;
        public:
            virtual ~i_event() {}
        public:
            virtual bool trigger(Arguments... aArguments) const = 0;
            virtual bool sync_trigger(Arguments... aArguments) const = 0;
            virtual void async_trigger(Arguments... aArguments) const = 0;
            virtual void accept() const = 0;
            virtual void ignore() const = 0;
        public:
            handle_ptr subscribe(const concrete_callback& aCallback, const void* aUniqueId = nullptr) const
            {
                return handle_ptr{ do_subscribe(aCallback, aUniqueId) };
            }
            handle_ptr operator()(const concrete_callback& aCallback, const void* aUniqueId = nullptr) const
            {
                return handle_ptr{ do_subscribe(aCallback, aUniqueId) };
            }
            template <typename T>
            handle_ptr subscribe(const concrete_callback& aCallback, const T* aUniqueIdObject) const
            {
                return handle_ptr{ do_subscribe(aCallback, static_cast<const void*>(aUniqueIdObject)) };
            }
            template <typename T>
            handle_ptr operator()(const concrete_callback& aCallback, const T* aUniqueIdObject) const
            {
                return handle_ptr{ do_subscribe(aCallback, static_cast<const void*>(aUniqueIdObject)) };
            }
            template <typename T>
            handle_ptr subscribe(const concrete_callback& aCallback, const T& aUniqueIdObject) const
            {
                return handle_ptr{ do_subscribe(aCallback, static_cast<const void*>(&aUniqueIdObject)) };
            }
            template <typename T>
            handle_ptr operator()(const concrete_callback& aCallback, const T& aUniqueIdObject) const
            {
                return handle_ptr{ do_subscribe(aCallback, static_cast<const void*>(&aUniqueIdObject)) };
            }
            void unsubscribe(handle aHandle) const
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
            virtual handle* do_subscribe(const callback& aCallback, const void* aUniqueId = nullptr) const = 0;
            virtual void do_unsubscribe(handle& aHandle) const = 0;
            virtual void do_unsubscribe(const void* aUniqueId) const = 0;
        };

        #define detail_event_subscribe( declName, ... ) \
            std::unique_ptr<neolib::plugin_events::i_event_handle<__VA_ARGS__>> declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, const void* aUniqueId = nullptr) const { return declName()(aCallback, aUniqueId); }\
            std::unique_ptr<neolib::plugin_events::i_event_handle<__VA_ARGS__>> declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, const void* aUniqueId = nullptr) { return declName()(aCallback, aUniqueId); }\
            template <typename T>\
            std::unique_ptr<neolib::plugin_events::i_event_handle<__VA_ARGS__>> declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, const T* aUniqueObject) const { return declName()(aCallback, static_cast<const void*>(aUniqueObject)); }\
            template <typename T>\
            std::unique_ptr<neolib::plugin_events::i_event_handle<__VA_ARGS__>> declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, const T* aUniqueObject) { return declName()(aCallback, static_cast<const void*>(aUniqueObject)); }\
            template <typename T>\
            std::unique_ptr<neolib::plugin_events::i_event_handle<__VA_ARGS__>> declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, T&& aUniqueObject) const { return declName()(aCallback, static_cast<const void*>(&aUniqueObject)); }\
            template <typename T>\
            std::unique_ptr<neolib::plugin_events::i_event_handle<__VA_ARGS__>> declName(const neolib::plugin_events::event_callback<__VA_ARGS__>& aCallback, T&& aUniqueObject) { return declName()(aCallback, static_cast<const void*>(&aUniqueObject)); }

        #define declare_event( declName, ... ) \
            virtual const neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() const = 0;\
            virtual neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() = 0;\
            const neolib::plugin_events::i_event<__VA_ARGS__>& declName() const { return ev_##declName(); }\
            neolib::plugin_events::i_event<__VA_ARGS__>& declName() { return ev_##declName(); }\
            detail_event_subscribe(declName, __VA_ARGS__)

        #define declare_event_2( declName, ... ) \
            virtual const neolib::plugin_events::i_event<__VA_ARGS__>& ev_2_##declName() const = 0;\
            virtual neolib::plugin_events::i_event<__VA_ARGS__>& ev__2##declName() = 0;\
            const neolib::plugin_events::i_event<__VA_ARGS__>& declName() const { return ev_2_##declName(); }\
            neolib::plugin_events::i_event<__VA_ARGS__>& declName() { return ev_2_##declName(); }\
            detail_event_subscribe(declName, __VA_ARGS__)

        template <typename... Arguments>
        class event;

        #define define_declared_event( name, declName, ... ) \
            neolib::plugin_events::event<__VA_ARGS__> name; \
            const neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() const override { return name; };\
            neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() override { return name; };

        #define define_declared_event_2( name, declName, ... ) \
            neolib::plugin_events::event<__VA_ARGS__> name; \
            const neolib::plugin_events::i_event<__VA_ARGS__>& ev_2_##declName() const override { return name; };\
            neolib::plugin_events::i_event<__VA_ARGS__>& ev_2_##declName() override { return name; };

        #define define_event( name, declName, ... ) \
            neolib::plugin_events::event<__VA_ARGS__> name; \
            const neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() const { return name; };\
            neolib::plugin_events::i_event<__VA_ARGS__>& ev_##declName() { return name; };\
            const neolib::plugin_events::i_event<__VA_ARGS__>& declName() const { return ev_##declName(); }\
            neolib::plugin_events::i_event<__VA_ARGS__>& declName() { return ev_##declName(); }\
            detail_event_subscribe(declName, __VA_ARGS__)

        #define define_event_2( name, declName, ... ) \
            neolib::plugin_events::event<__VA_ARGS__> name; \
            const neolib::plugin_events::i_event<__VA_ARGS__>& ev_2_##declName() const { return name; };\
            neolib::plugin_events::i_event<__VA_ARGS__>& ev_2_##declName() { return name; };\
            const neolib::plugin_events::i_event<__VA_ARGS__>& declName() const { return ev_2_##declName(); }\
            neolib::plugin_events::i_event<__VA_ARGS__>& declName() { return ev_2_##declName(); }\
            detail_event_subscribe(declName, __VA_ARGS__)
   }
}