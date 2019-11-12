// i_custom_type.hpp - v1.0
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
#include "i_reference_counted.hpp"
#include "i_string.hpp"

namespace neolib
{
    class i_custom_type : public i_reference_counted
    {
        // exceptions
    public:
        struct no_instance : std::logic_error { no_instance() : std::logic_error("neolib::i_custom_type::no_instance") {} };
        // construction/assignment
    public:
        virtual i_custom_type* clone() const = 0;
        virtual i_custom_type& assign(const i_custom_type& aRhs) = 0;
        i_custom_type& operator=(const i_custom_type& aRhs)
        {
            assign(aRhs);
            return *this;
        }
        // comparison
    public:
        virtual bool operator==(const i_custom_type&) const = 0;
        virtual bool operator<(const i_custom_type&) const = 0;
        // state
    public:
        bool has_instance() const { return instance_ptr() != nullptr; }
        template <typename T>
        const T& instance_as() const { if (!has_instance()) throw no_instance();  return *static_cast<const T*>(instance_ptr()); }
        template <typename T>
        T& instance_as() { if (!has_instance()) throw no_instance(); return *static_cast<T*>(instance_ptr()); }
        virtual const void* instance_ptr() const = 0;
        virtual void* instance_ptr() = 0;
        // meta
    public:
        virtual void name(i_string& aName) const = 0;
        virtual void to_string(i_string& aString) const = 0;
        std::string name() const { string n; name(n); return n.to_std_string(); }
        std::string to_string() const { string s; to_string(s); return s.to_std_string(); }
    };

    inline bool operator!=(const i_custom_type& lhs, const i_custom_type& rhs)
    {
        return !(lhs == rhs);
    }
}
