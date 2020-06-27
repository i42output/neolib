// i_setting.hpp
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
#include <neolib/core/i_reference_counted.hpp>
#include <neolib/core/string.hpp>
#include <neolib/app/setting_value.hpp>
#include <neolib/app/i_setting_constraints.hpp>

namespace neolib
{
    class i_settings;

    class i_setting : public i_reference_counted
    {
        friend class settings;
    public:
        typedef i_setting abstract_type;
        typedef uint32_t id_type;
    public:
        virtual i_settings& manager() const = 0;
        virtual id_type id() const = 0;
        virtual i_string const& category() const = 0;
        virtual i_string const& name() const = 0;
        virtual i_setting_constraints const& constraints() const = 0;
        virtual bool dirty() const = 0;
        virtual bool hidden() const = 0;
        virtual i_setting_value const& value() const = 0;
        virtual i_setting_value const& new_value() const = 0;
        virtual void value_as_string(i_string& aValue) const = 0;
        virtual void set_value(i_setting_value const& aNewValue) = 0;
        virtual void set_value_from_string(i_string const& aNewValue) = 0;
        virtual void clear() = 0;
    private:
        virtual void set_id(id_type id) = 0;
        virtual bool apply_change() = 0;
        virtual bool discard_change() = 0;
    private:
        virtual void clone(i_ref_ptr<i_setting>& aResult) const = 0;
    public:
        template <typename T>
        T const& value() const
        {
            return value().get<T>();
        }
        template <typename T>
        T const& new_value() const
        {
            return new_value().get<T>();
        }
        std::string value_as_string() const
        {
            thread_local string result;
            value_as_string(result);
            return result.to_std_string();
        }
        template <typename T>
        void set_value(T const& aNewValue)
        {
            return set_value(static_cast<i_setting_value const&>(setting_value<T>{ aNewValue }));
        }
    public:
        i_setting& operator=(i_setting const& aRhs)
        {
            set_value(aRhs.value());
            return *this;
        }
        i_setting& operator=(i_setting_value const& aRhs)
        {
            set_value(aRhs);
            return *this;
        }
        template <typename T>
        i_setting& operator=(T const& aNewValue)
        {
            set_value(aNewValue);
            return *this;
        }
    };
}
