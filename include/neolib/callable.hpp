// callable.hpp
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
#include <neolib/i_callable.hpp>

namespace neolib
{
    template <typename Signature>
    struct callable;
    template <typename R, typename C, typename A1>
    struct callable<R(C::*)(A1) const> : i_callable<R(C::*)(A1) const>
    {
        typedef i_callable<R(C::*)(A1) const> base_type;
        using typename base_type::abstract_type;
        using typename base_type::signature_type;
        using typename base_type::return_type;
        using typename base_type::class_type;
        A1 argument;
        callable(const A1& argument) :
            argument{ argument }
        {
        }
        return_type call(const class_type& aObject, signature_type aCallable) const override
        {
            aCallable(argument);
        }
    };
    template <typename R, typename C, typename A1>
    struct callable<R(C::*)(A1)> : i_callable<R(C::*)(A1)>
    {
        typedef i_callable<R(C::*)(A1)> base_type;
        using typename base_type::abstract_type;
        using typename base_type::signature_type;
        using typename base_type::return_type;
        using typename base_type::class_type;
        A1 argument;
        callable(const A1& argument) :
            argument{ argument }
        {
        }
        return_type call(class_type& aObject, signature_type aCallable) const override
        {
            aCallable(argument);
        }
    };
}
