// secure_memory.cpp
/*
 *  Copyright (c) 2026 Leigh Johnston.
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

#include <neolib/neolib.hpp>
#include <windows.h>
#include <algorithm>
#include <new>
#include <system_error>
#include <neolib/secure/secure_string.hpp>

namespace neolib
{
    namespace
    {
        std::size_t page_size()
        {
            static std::size_t const sPageSize = []()
            {
                SYSTEM_INFO systemInfo;
                ::GetSystemInfo(&systemInfo);
                return static_cast<std::size_t>(systemInfo.dwPageSize);
            }();
            return sPageSize;
        }

        std::size_t whole_pages(std::size_t aSize)
        {
            auto const pageSize = page_size();
            return (std::max<std::size_t>(aSize, 1u) + pageSize - 1u) / pageSize * pageSize;
        }

        bool lock(void* aMemory, std::size_t aSize)
        {
            if (::VirtualLock(aMemory, aSize))
                return true;
            if (::GetLastError() != ERROR_WORKING_SET_QUOTA)
                return false;
            // the number of pages a process can lock is limited by its minimum working set size;
            // grow the working set to accommodate the new pages and try again
            HANDLE const process = ::GetCurrentProcess();
            SIZE_T minimumWorkingSetSize = 0u;
            SIZE_T maximumWorkingSetSize = 0u;
            if (!::GetProcessWorkingSetSize(process, &minimumWorkingSetSize, &maximumWorkingSetSize))
                return false;
            SIZE_T const increase = aSize + 16u * page_size();
            if (!::SetProcessWorkingSetSize(process, minimumWorkingSetSize + increase, maximumWorkingSetSize + increase))
                return false;
            return ::VirtualLock(aMemory, aSize) != FALSE;
        }
    }

    void* secure_allocate(std::size_t aSize)
    {
        auto const size = whole_pages(aSize);
        void* const memory = ::VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (memory == nullptr)
            throw std::bad_alloc{};
        if (!lock(memory, size))
        {
            auto const error = ::GetLastError();
            ::VirtualFree(memory, 0u, MEM_RELEASE);
            throw std::system_error{ static_cast<int>(error), std::system_category(), "neolib::secure_allocate: VirtualLock failed" };
        }
        return memory;
    }

    void secure_deallocate(void* aMemory, std::size_t aSize) noexcept
    {
        if (aMemory == nullptr)
            return;
        auto const size = whole_pages(aSize);
        secure_erase(aMemory, size);
        ::VirtualUnlock(aMemory, size);
        ::VirtualFree(aMemory, 0u, MEM_RELEASE);
    }
}
