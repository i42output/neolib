// shared_thread_local.cpp
/*
 *  Copyright (c) 2023 Leigh Johnston.
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
#include <vector>
#include <unordered_map>
#include <neolib/app/shared_thread_local.hpp>

namespace neolib
{
    template<> i_shared_thread_local& services::start_service<i_shared_thread_local>()
    {
        static shared_thread_local sService;
        return sService;
    }

    shared_thread_local::result_type shared_thread_local::allocate_or_get(char const* aFullyQualifiedVariableName, std::size_t aVariableSize, void(*aDeleter)(void*))
    {
        struct data
        {
            std::size_t size;
            std::unique_ptr<char[]> memory;
            void(*deleter)(void*);

            // note: we cannot use the unique_ptr's deleter.
            data(std::size_t aSize, void(*aDeleter)(void*)) :
                size{ aSize },
                memory{ std::make_unique<char[]>(aSize) },
                deleter{ aDeleter }
            {
            }
            ~data()
            {
                deleter(memory.get());
            }
        };

        result_type result = {};

        thread_local std::vector<std::unique_ptr<data>> tLocalStack;
        thread_local std::unordered_map<std::string, data const*> tLocals;
        auto existing = tLocals.find(aFullyQualifiedVariableName);
        if (existing == tLocals.end())
        {
            tLocalStack.push_back(std::make_unique<data>(aVariableSize, aDeleter));
            existing = tLocals.insert(std::make_pair(aFullyQualifiedVariableName, tLocalStack.back().get())).first;
            result.initializationRequired = true;
        }
        if (existing->second->size != aVariableSize)
            throw std::logic_error("neolib::shared_thread_local::allocate_or_get: bad size");
        result.memory = existing->second->memory.get();
        return result;
    }
}
