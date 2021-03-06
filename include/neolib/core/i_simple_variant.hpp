// i_simple_variant.hpp
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
#include <string>
#include <boost/lexical_cast.hpp>
#include <neolib/core/i_reference_counted.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/i_enum.hpp>
#include <neolib/core/i_custom_type.hpp>
#include <neolib/plugin/i_plugin_variant.hpp>

namespace neolib
{
    enum class simple_variant_type : uint32_t
    {
        Boolean,
        Integer,
        Real,
        String,
        Enum,
        CustomType
    };
}

begin_declare_enum(neolib::simple_variant_type)
declare_enum_string(neolib::simple_variant_type, Boolean)
declare_enum_string(neolib::simple_variant_type, Integer)
declare_enum_string(neolib::simple_variant_type, Real)
declare_enum_string(neolib::simple_variant_type, String)
declare_enum_string(neolib::simple_variant_type, Enum)
declare_enum_string(neolib::simple_variant_type, CustomType)
end_declare_enum(neolib::simple_variant_type)

namespace neolib
{
    typedef i_plugin_variant<simple_variant_type, bool, int64_t, double, i_string, i_ref_ptr<i_enum>, i_ref_ptr<i_custom_type>> i_simple_variant;

    inline std::string to_string(i_simple_variant const& aVariant)
    {
        switch (aVariant.which())
        {
        case simple_variant_type::Boolean:
            if (aVariant.get<bool>())
                return "true";
            else
                return "false";
        case simple_variant_type::Integer:
            return std::to_string(aVariant.get<int64_t>());
        case simple_variant_type::Real:
            return std::to_string(aVariant.get<double>());
        case simple_variant_type::String:
            return aVariant.get<i_string>().to_std_string();
        case simple_variant_type::Enum:
            return aVariant.get<i_ref_ptr<i_enum>>()->to_string();
        case simple_variant_type::CustomType:
            return aVariant.get<i_ref_ptr<i_custom_type>>()->to_string();
        default:
            throw std::logic_error("neolib: cannot convert simple variant to string");
        }
    }
}
