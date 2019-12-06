// lifetime.inl
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
    template <lifetime_state RequiredState, typename Owner>
    inline lifetime_flag<RequiredState, Owner>::lifetime_flag(const i_lifetime& aSubject, owner_pointer aOwner) : 
        iCookie{ invalid_cookie<cookie_type> }, iSubject { &aSubject }, iOwner{ aOwner }, iState{ aSubject.object_state() }, iDebug{ false }
    {
        iCookie = subject().add_flag(*this);
    }

    template <lifetime_state RequiredState, typename Owner>
    inline lifetime_flag<RequiredState, Owner>::lifetime_flag(const lifetime_flag& aOther) : 
        iCookie{ invalid_cookie<cookie_type> }, iSubject{ aOther.iSubject }, iOwner{ aOther.iOwner }, iState { aOther.iSubject->object_state() }, iDebug{ false }
    {
        iCookie = subject().add_flag(*this);
    }

    template <lifetime_state RequiredState, typename Owner>
    inline lifetime_flag<RequiredState, Owner>::~lifetime_flag()
    {
        if (!is_destroyed())
            subject().remove_flag(*this);
    }

    template <lifetime_state RequiredState, typename Owner>
    inline neolib::cookie lifetime_flag<RequiredState, Owner>::cookie() const
    {
        return iCookie;
    }

    template <lifetime_state RequiredState, typename Owner>
    inline bool lifetime_flag<RequiredState, Owner>::is_creating() const
    {
        return iState == lifetime_state::Creating;
    }

    template <lifetime_state RequiredState, typename Owner>
    inline bool lifetime_flag<RequiredState, Owner>::is_alive() const
    {
        return iState == lifetime_state::Alive;
    }

    template <lifetime_state RequiredState, typename Owner>
    inline bool lifetime_flag<RequiredState, Owner>::is_destroying() const
    {
        return iState == lifetime_state::Destroying;
    }

    template <lifetime_state RequiredState, typename Owner>
    inline bool lifetime_flag<RequiredState, Owner>::is_destroyed() const
    {
        return iState == lifetime_state::Destroyed;
    }

    template <lifetime_state RequiredState, typename Owner>
    inline lifetime_flag<RequiredState, Owner>::operator bool() const
    {
        return iState == RequiredState;
    }

    template <lifetime_state RequiredState, typename Owner>
    inline void lifetime_flag<RequiredState, Owner>::set_alive()
    {
        if (iState == lifetime_state::Alive)
            return;
        if (debug())
            std::cerr << "lifetime_flag::set_alive()" << std::endl;
        iState = lifetime_state::Alive;
    }

    template <lifetime_state RequiredState, typename Owner>
    inline void lifetime_flag<RequiredState, Owner>::set_destroying()
    {
        if (iState == lifetime_state::Destroying)
            return;
        if (debug())
            std::cerr << "lifetime_flag::set_destroying()" << std::endl;
        iState = lifetime_state::Destroying;
    }

    template <lifetime_state RequiredState, typename Owner>
    inline void lifetime_flag<RequiredState, Owner>::set_destroyed()
    {
        if (iState == lifetime_state::Destroyed)
            return;
        if (debug())
            std::cerr << "lifetime_flag::set_destroyed()" << std::endl;
        iState = lifetime_state::Destroyed;
    }

    template <lifetime_state RequiredState, typename Owner>
    inline bool lifetime_flag<RequiredState, Owner>::debug() const
    {
        return iDebug;
    }
    
    template <lifetime_state RequiredState, typename Owner>
    inline void lifetime_flag<RequiredState, Owner>::set_debug(bool aDebug)
    {
        iDebug = aDebug;
    }

    template <lifetime_state RequiredState, typename Owner>
    inline const i_lifetime& lifetime_flag<RequiredState, Owner>::subject() const
    {
        return *iSubject;
    }

    template <typename FlagList>
    inline basic_lifetime<FlagList>::basic_lifetime(lifetime_state aState) : iState{ aState }
    {
    }

    template <typename FlagList>
    inline basic_lifetime<FlagList>::~basic_lifetime()
    {
        std::scoped_lock<mutex_type> lk(iFlagList.mutex());
        if (!is_destroyed())
        {
            set_destroying();
            set_destroyed();
        }
    }

    template <typename FlagList>
    inline lifetime_state basic_lifetime<FlagList>::object_state() const
    {
        return iState;
    }

    template <typename FlagList>
    inline bool basic_lifetime<FlagList>::is_creating() const
    {
        return iState == lifetime_state::Creating;
    }

    template <typename FlagList>
    inline bool basic_lifetime<FlagList>::is_alive() const
    {
        return iState == lifetime_state::Alive;
    }

    template <typename FlagList>
    inline bool basic_lifetime<FlagList>::is_destroying() const
    {
        return iState == lifetime_state::Destroying;
    }

    template <typename FlagList>
    inline bool basic_lifetime<FlagList>::is_destroyed() const
    {
        return iState == lifetime_state::Destroyed;
    }

    template <typename FlagList>
    inline void basic_lifetime<FlagList>::set_alive()
    {
        std::scoped_lock<mutex_type> lk(iFlagList.mutex());
        if (!is_creating())
            throw not_creating();
        iState = lifetime_state::Alive;
        for (auto i = flags().begin(); i != flags().end();)
            (*i++)->set_alive();
    }

    template <typename FlagList>
    inline void basic_lifetime<FlagList>::set_destroying()
    {
        std::scoped_lock<mutex_type> lk(iFlagList.mutex());
        if (!is_destroying())
        {
            if (is_destroyed())
                throw already_destroyed();
            iState = lifetime_state::Destroying;
            for (auto i = flags().begin(); i != flags().end();)
                (*i++)->set_destroying();
        }
    }

    template <typename FlagList>
    inline void basic_lifetime<FlagList>::set_destroyed()
    {
        std::scoped_lock<mutex_type> lk(iFlagList.mutex());
        if (!is_destroyed())
        {
            if (iState == lifetime_state::Creating || iState == lifetime_state::Alive)
                set_destroying();
            iState = lifetime_state::Destroyed;
            for (auto i = flags().begin(); i != flags().end();)
                (*i++)->set_destroyed();
        }
    }

    template <typename FlagList>
    inline typename basic_lifetime<FlagList>::cookie_type basic_lifetime<FlagList>::add_flag(i_lifetime_flag& aFlag) const
    {
        std::scoped_lock<mutex_type> lk(flags().mutex());
        cookie_type result = flags().insert(&aFlag);
        switch (iState)
        {
        case lifetime_state::Creating:
        case lifetime_state::Alive:
            break;
        case lifetime_state::Destroying:
            aFlag.set_destroying();
            break;
        case lifetime_state::Destroyed:
        default:
            aFlag.set_destroying();
            aFlag.set_destroyed();
            break;
        }
        return result;
    }

    template <typename FlagList>
    inline void basic_lifetime<FlagList>::remove_flag(const i_lifetime_flag& aFlag) const
    {
        std::scoped_lock<mutex_type> lk(flags().mutex());
        flags().remove(aFlag.cookie());
    }

    template <typename FlagList>
    inline typename basic_lifetime<FlagList>::flag_list_type& basic_lifetime<FlagList>::flags() const
    {
        return iFlagList;
    }
}
