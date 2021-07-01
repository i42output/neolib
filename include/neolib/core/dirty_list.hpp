// dirty_list.hpp
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
#include <vector>
#
namespace neolib
{
    /**
     * @brief Dirty list used to represent a collection of flags to represent changes
     * to state in re-entrant functions (see timer_object::poll and timer_service::poll).
     */
    class dirty_list
    {
    public:
        dirty_list(std::recursive_mutex& aMutex) : 
            iMutex{ aMutex }
        {
        }
    public:
        void enter_scope()
        {
            std::unique_lock lock{ iMutex };
            iDirtyFlags.emplace_back();
        }
        void leave_scope()
        {
            std::unique_lock lock{ iMutex };
            iDirtyFlags.pop_back();
        }
        bool is_dirty() const
        {
            std::unique_lock lock{ iMutex };
            return !iDirtyFlags.empty() && iDirtyFlags.back();
        }
        void dirty()
        {
            std::unique_lock lock{ iMutex };
            std::fill(iDirtyFlags.begin(), iDirtyFlags.end(), true);
        }
        void clean()
        {
            std::unique_lock lock{ iMutex };
            if (!iDirtyFlags.empty())
                iDirtyFlags.back() = false;
        }
    private:
        std::recursive_mutex& iMutex;
        std::vector<bool> iDirtyFlags;
    };

    class scoped_dirty
    {
    public:
        scoped_dirty(dirty_list& aList) :
            iList{ aList }
        {
            iList.enter_scope();
        }
        ~scoped_dirty()
        {
            iList.leave_scope();
        }
    private:
        dirty_list& iList;
    };
}
