// thread_local.hpp
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
#include <memory>
#include <vector>

namespace neolib
{
    template <typename T>
    struct variable_stack
    {
        typedef T value_type;
        typedef std::unique_ptr<value_type> pointer_type;
        typedef std::vector<pointer_type> stack_type;

        std::size_t stackPointer = 0;
        stack_type stack;

        value_type& current()
        {
            auto& ptr = stack[stackPointer - 1];
            if (ptr == nullptr)
                ptr = std::make_unique<value_type>();
            return *ptr;
        }

        void push()
        {
            ++stackPointer;
            if (stack.size() < stackPointer)
                stack.resize(stackPointer);
        }

        void pop()
        {
            --stackPointer;
        }
    };

    template <typename T>
    class variable_stack_context
    {
    public:
        variable_stack_context(variable_stack<T>& aStack) :
            iStack{ aStack }
        {
            iStack.push();
        }
        ~variable_stack_context()
        {
            iStack.pop();
        }
    private:
        variable_stack<T>& iStack;
    };
}
