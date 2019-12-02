// plugin_event.hpp
/*
  Transplanted from neogfx C++ GUI Library
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
#include <neolib/event.hpp>
#include <neolib/i_plugin_event.hpp>

namespace neolib
{
    namespace plugin_events
    {
        using neolib::async_event_queue;
        using neolib::event_trigger_type;

        template <typename... Args>
        class event_handle : public i_event_handle<Args...>, public neolib::event_handle<Args...>
        {
            typedef neolib::event_handle<Args...> base_type;
        public:
            using base_type::base_type;
            event_handle(const base_type& aOther) :
                base_type{ aOther }
            {
            }
        };

        class sink : public neolib::sink
        {
            typedef neolib::sink base_type;
        public:
            using base_type::base_type;
        public:
            using base_type::operator=;
            using base_type::operator+=;
            template <typename... Args>
            sink& operator=(const i_event_handle<Args...>& aHandle)
            {
                base_type::operator=(static_cast<const event_handle<Args...>&>(aHandle));
                return *this;
            }
            template <typename... Args>
            sink& operator+=(const i_event_handle<Args...>& aHandle)
            {
                base_type::operator+=(static_cast<const event_handle<Args...>&>(aHandle));
                return *this;
            }
            template <typename... Args>
            sink& operator=(const std::unique_ptr<i_event_handle<Args...>>& aHandle)
            {
                base_type::operator=(static_cast<const event_handle<Args...>&>(*aHandle));
                return *this;
            }
            template <typename... Args>
            sink& operator+=(const std::unique_ptr<i_event_handle<Args...>>& aHandle)
            {
                base_type::operator+=(static_cast<const event_handle<Args...>&>(*aHandle));
                return *this;
            }
        };

        template <typename... Args>
        class event : public i_event<Args...>, public neolib::event<Args...>
        {
            typedef neolib::event<Args...> base_type;
        public:
            typedef typename i_event<Args...>::handle handle;
            typedef typename i_event<Args...>::callback callback;
            typedef event_handle<Args...> concrete_handle;
        public:
            using base_type::base_type;
        public:
            using base_type::subscribe;
            using base_type::operator();
            using base_type::unsubscribe;
        public:
            bool trigger(Args... aArguments) const override
            {
                return base_type::trigger(aArguments...);
            }
            bool sync_trigger(Args... aArguments) const override
            {
                return base_type::sync_trigger(aArguments...);
            }
            void async_trigger(Args... aArguments) const override
            {
                base_type::async_trigger(rvalue_cast<Args>(aArguments)...);
            }
            void accept() const override
            {
                base_type::accept();
            }
            void ignore() const override
            {
                base_type::ignore();
            }
        private:
            handle* do_subscribe(const callback& aCallback, const void* aUniqueId = nullptr) const override
            {
                std::shared_ptr<callback> cb = std::move(aCallback.clone());
                return new concrete_handle(
                    base_type::subscribe(
                        [cb](Args&& ... aArguments)
                        {
                            (*cb)(std::forward<Args>(aArguments)...);
                        },
                        aUniqueId));
            }
            void do_unsubscribe(handle& aHandle) const override
            {
                return base_type::unsubscribe(static_cast<concrete_handle&>(aHandle));
            }
            void do_unsubscribe(const void* aUniqueId) const override
            {
                return base_type::unsubscribe(aUniqueId);
            }
        };
    }
}