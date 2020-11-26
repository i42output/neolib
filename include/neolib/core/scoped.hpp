// scoped.hpp
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

#include <neolib/neolib.hpp>
#include <atomic>

namespace neolib
{
    class scoped_flag
    {
    public:
        scoped_flag(bool& aFlag, bool aValue = true) : iFlag{ aFlag }, iSaved{ aFlag }, iIgnore{ false } { iFlag = aValue; }
        ~scoped_flag() { if (!iIgnore) iFlag = iSaved; }
    public:
        bool saved() const { return iSaved; }
        void ignore() { iIgnore = true; }
    private:
        bool& iFlag;
        bool iSaved;
        bool iIgnore;
    };

    class scoped_atomic_flag
    {
    public:
        scoped_atomic_flag(std::atomic<bool>& aFlag, bool aValue = true) : iFlag{ aFlag }, iSaved{ aFlag }, iIgnore{ false } { iFlag = aValue; }
        ~scoped_atomic_flag() { if (!iIgnore) iFlag = iSaved; }
    public:
        bool saved() const { return iSaved; }
        void ignore() { iIgnore = true; }
    private:
        std::atomic<bool>& iFlag;
        bool iSaved;
        bool iIgnore;
    };

    template <typename T>
    class scoped_counter
    {
    public:
        scoped_counter(T& aCounter) : iCounter(aCounter), iIgnore{ false } { ++iCounter; }
        ~scoped_counter() { if (!iIgnore) --iCounter; }
    public:
        void ignore() { iIgnore = true; }
    private:
        T& iCounter;
        bool iIgnore;
    };

    template <typename T>
    class scoped_pointer
    {
    public:
        scoped_pointer(T*& aPointer, T* aValue) : iPointer{ aPointer }, iSaved{ aPointer }, iIgnore{ false } { iPointer = aValue; }
        ~scoped_pointer() { if (!iIgnore) iPointer = iSaved; }
    public:
        T const& saved() const { return iSaved; }
        void ignore() { iIgnore = true; }
    private:
        T*& iPointer;
        T* iSaved;
        bool iIgnore;
    };

    template <typename T>
    class scoped_object
    {
    public:
        scoped_object(T& aObject, T aValue = {}) : iObject{ aObject }, iSaved{ aObject }, iIgnore{ false } { iObject = aValue; }
        ~scoped_object() { if (!iIgnore) iObject = iSaved; }
    public:
        T const& saved() const { return iSaved; }
        void ignore() { iIgnore = true; }
    private:
        T& iObject;
        T iSaved;
        bool iIgnore;
    };
}
