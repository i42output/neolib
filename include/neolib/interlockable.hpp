// interlockable.hpp v1.2
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

#include "neolib.hpp"
#include <map>
#include <memory>
#include "lockable.hpp"

namespace neolib
{
    class interlockable : protected lockable
    {
        // types
    public:
        struct deadlock_error : std::logic_error 
        { 
            deadlock_error() : std::logic_error("neolib::interlockable::deadlock_error") {} 
        };
    private:
        class interlock : private lockable
        {
            // construction
        public:
            interlock() : iAtom(false) {}
            // operations
        public:
            bool acquire() const
            { 
                neolib::lock lock(*this); 
                if (!iAtom)
                {
                    iAtom = true;
                    return true;
                }
                return false;
            }
            void release() const
            { 
                neolib::lock lock(*this); 
                iAtom = false; 
            }
            // attributes
        private:
            mutable bool iAtom;
        };
        typedef std::shared_ptr<interlock> interlock_ptr;
        typedef std::map<const interlockable*, interlock_ptr> interlocks;
        // operations
    public:
        void interlock_add(interlockable& aOther)
        {
            neolib::lock lock(*this); 
            aOther.iInterlocks[this] = iInterlocks[&aOther] = interlock_ptr(new interlock);
        }
        void interlock_remove(interlockable& aOther)
        {
            neolib::lock lock(*this); 
            iInterlocks.erase(&aOther);
            aOther.iInterlocks.erase(this);
        }
    protected:
        void interlock_acquire(const interlockable& aOther)
        {
            neolib::lock lock(*this); 
            interlocks::iterator theInterlock = iInterlocks.find(&aOther);
            if (theInterlock == iInterlocks.end())
                return;
            while(!theInterlock->second->acquire())
            {
                if (!purge(aOther))
                    throw deadlock_error();
            }
        }
        void interlock_release(const interlockable& aOther)
        {
            neolib::lock lock(*this); 
            interlocks::iterator theInterlock = iInterlocks.find(&aOther);
            if (theInterlock == iInterlocks.end())
                return;
            theInterlock->second->release();
        }
        // implementation
    private:
        virtual bool purge(const interlockable& aOther) = 0;
        // attributes
    private:
        interlocks iInterlocks;
    };
}
