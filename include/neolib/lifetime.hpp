// lifetime.hpp
/*
 *  Copyright (c) 2019 Leigh Johnston.
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
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <optional>

#include <neolib/vecarray.hpp>
#include <neolib/null_mutex.hpp>
#include <neolib/allocator.hpp>
#include <neolib/i_lifetime.hpp>

namespace neolib
{
    template <lifetime_state RequiredState, typename Owner = void>
    class lifetime_flag : public i_lifetime_flag
    {
        template <typename Mutex>
        friend class basic_lifetime;
    public:
        using i_lifetime_flag::cookie_type;
    private:
        typedef const i_lifetime* subject_pointer;
        typedef Owner* owner_pointer;
    public:
        lifetime_flag(const i_lifetime& aSubject, owner_pointer aOwner = nullptr);
        lifetime_flag(const lifetime_flag& aOther);
        ~lifetime_flag();
    public:
        cookie_type cookie() const final;
    public:
        bool is_creating() const final;
        bool is_alive() const final;
        bool is_destroying() const final;
        bool is_destroyed() const final;
        operator bool() const final;
        void set_alive() final;
        void set_destroying() final;
        void set_destroyed() final;
    public:
        bool debug() const override;
        void set_debug(bool aDebug = true) override;
    private:
        const i_lifetime& subject() const;
    private:
        cookie_type iCookie;
        subject_pointer iSubject;
        owner_pointer iOwner;
        std::atomic<lifetime_state> iState;
        bool iDebug;
    };

    typedef lifetime_flag<lifetime_state::Destroyed> destroyed_flag;
    typedef std::optional<destroyed_flag> optional_destroyed_flag;

    template <typename FlagList>
    class basic_lifetime : public virtual i_lifetime
    {
    public:
        typedef neolib::destroyed_flag destroyed_flag;
        using i_lifetime::cookie_type;
    private:
        typedef FlagList flag_list_type;
        typedef typename flag_list_type::container_type flags_t;
        typedef typename flag_list_type::mutex_type mutex_type;
    public:
        basic_lifetime(lifetime_state aState = lifetime_state::Alive);
        virtual ~basic_lifetime();
    public:
        lifetime_state object_state() const final;
        bool is_creating() const final;
        bool is_alive() const final;
        bool is_destroying() const final;
        bool is_destroyed() const final;
        void set_alive() override;
        void set_destroying() override;
        void set_destroyed() override;
    public:
        cookie_type add_flag(i_lifetime_flag& aFlag) const final;
        void remove_flag(const i_lifetime_flag& aFlag) const final;
    private:
        flag_list_type& flags() const;
    private:
        std::atomic<lifetime_state> iState;
        mutable flag_list_type iFlagList;
    };

    typedef basic_lifetime<jar<i_lifetime_flag*, null_mutex>> single_threaded_lifetime;
    typedef basic_lifetime<jar<i_lifetime_flag*, std::recursive_mutex>> multi_threaded_lifetime;

    typedef multi_threaded_lifetime lifetime;
}

#include "lifetime.inl"
