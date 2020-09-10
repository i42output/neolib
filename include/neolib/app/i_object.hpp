// i_object.hpp
/*
 *  Copyright (c) 2019, 2020 Leigh Johnston.
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
#include <type_traits>
#include <neolib/core/i_lifetime.hpp>
#include <neolib/plugin/i_plugin_event.hpp>

namespace neolib
{
    class i_object : public i_lifetime
    {
    public:
        declare_event(destroying);
        declare_event(destroyed);
    public:
        virtual ~i_object() = default;
    };

    template <typename Object>
    inline bool is_alive(Object& aObject)
    {
        if constexpr (std::is_base_of_v<i_lifetime, Object>)
            return static_cast<i_lifetime&>(aObject).is_alive();
        else
            return dynamic_cast<i_lifetime&>(aObject).is_alive();
    }

    template <typename Object, typename Handler>
    inline auto destroying(Object& aObject, const Handler aHandler)
    {
        if constexpr (std::is_base_of_v<i_object, Object>)
            return static_cast<i_object&>(aObject).destroying(aHandler);
        else
            return dynamic_cast<i_object&>(aObject).destroying(aHandler);
    }

    template <typename Object, typename Handler>
    inline auto destroyed(Object& aObject, const Handler aHandler)
    {
        if constexpr (std::is_base_of_v<i_object, Object>)
            return static_cast<i_object&>(aObject).destroyed(aHandler);
        else
            return dynamic_cast<i_object&>(aObject).destroyed(aHandler);
    }
}
