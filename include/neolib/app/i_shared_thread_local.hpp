// i_shared_thread_local.hpp
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

#pragma once

#include <neolib/neolib.hpp>
#include <neolib/app/services.hpp>

namespace neolib
{
    /**
     * @brief Shared thread local service.
     * 
     * This service is a way to share thread local variables between an application and multiple DLLs
     * that all statically link against the same library that contains the thread local variable.
     * 
     * To use this service should be exposed to the DLLs from the application. TODO: HOWTO documentation.
     */
    class i_shared_thread_local : public i_service
    {
    public:
        struct result_type
        {
            void* memory;
            bool initializationRequired;
        };
    public:
        virtual result_type allocate_or_get(char const* aFullyQualifiedVariableName, std::size_t aVariableSize, void(*aDeleter)(void*)) = 0;
    public:
        template <typename T>
        result_type allocate_or_get(char const* aFullyQualifiedVariableName, void(*aDeleter)(void*))
        {
            return allocate_or_get(aFullyQualifiedVariableName, sizeof(T), aDeleter);
        }
    public:
        static uuid const& iid() { static uuid const sIid{ 0x975e11be, 0xd285, 0x4704, 0x9eef, { 0x28, 0xfb, 0x6b, 0x5e, 0xe0, 0x76 } }; return sIid; }
    };

    #define shared_thread_local_impl(VariableType, VariableScope, VariableName, InitialValue) \
    thread_local auto const& neolib_PartialResult_##VariableName = \
        neolib::service<neolib::i_shared_thread_local>().allocate_or_get<VariableType>( \
            STRING(VariableScope) "::" STRING(VariableName), \
            [](void* aMemory) { using T = VariableType; static_cast<VariableType*>(aMemory)->~T(); }); \
    auto const& neolib_CapturablePartialResult_##VariableName = neolib_PartialResult_##VariableName; \
    thread_local auto& VariableName = [&neolib_CapturablePartialResult_##VariableName]() -> VariableType& \
        { \
            if (neolib_CapturablePartialResult_##VariableName.initializationRequired) \
            { \
                new (static_cast<VariableType*>(neolib_CapturablePartialResult_##VariableName.memory)) VariableType{ InitialValue }; \
            } \
            return *static_cast<VariableType*>(neolib_CapturablePartialResult_##VariableName.memory); \
        }();

    #define shared_thread_local(VariableType, VariableScope, VariableName, ...) \
    shared_thread_local_impl(VariableType, VariableScope, VariableName, __VA_ARGS__)
}
