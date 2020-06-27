// setting_value.hpp
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
#include <optional>
#include <neolib/app/i_setting_value.hpp>

namespace neolib
{
    template <typename T>
    class setting_value : public i_setting_value
    {
        typedef setting_value<T> self_type;
    public:
        typedef i_setting_value abstract_type;
        typedef std::optional<T> container_type;
    public:
        setting_value() :
            iValue{}
        {
        }
        setting_value(T const& aDefaultValue) :
            iValue{ aDefaultValue }
        {
        }
        setting_value(self_type const& aOther) :
            iValue{ aOther.iValue }
        {
        }
        setting_value(i_setting_value const& aOther) :
            iValue{ aOther.is_set() ? aOther.get<T> : container_type{} }
        {
        }
    public:
        setting_type type() const override
        {
            return setting_type_v<T>;
        }
        bool is_set() const override
        {
            return !!iValue;
        }
        void clear() override
        {
            iValue = std::nullopt;
        }
    public:
        bool operator==(const i_setting_value& aRhs) const
        {
            if (type() != aRhs.type())
                return false;
            return get<T>() == aRhs.get<T>();
        }
        bool operator<(const i_setting_value& aRhs) const
        {
            if (type() != aRhs.type())
                return type() < aRhs.type();
            return get<T>() < aRhs.get<T>();
        }
    private:
        void const* data() const override
        {
            if (is_set())
                return &*iValue;
            throw not_set();
        }
        void* data() override
        {
            if (!is_set())
                iValue.emplace();
            return &*iValue;
        }
    private:
        container_type iValue;
    };
}
