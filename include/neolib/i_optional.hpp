// optional.hpp - v1.2.2
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
#include <optional>
#include <stdexcept>
#include <neolib/i_reference_counted.hpp>

namespace neolib
{
    template<typename T>
    class i_optional : public i_reference_counted
    {
        typedef i_optional<T> self_type;
        // types
    public:
        typedef self_type abstract_type;
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        struct not_valid : std::logic_error { not_valid() : std::logic_error("neolib::i_optional::not_valid") {} };
        // state
    public:
        virtual bool valid() const = 0;
        virtual bool invalid() const = 0;
        virtual operator bool() const = 0;
        // element access
    public:
        virtual reference get() = 0;
        virtual const_reference get() const = 0;
        virtual reference operator*() = 0;
        virtual const_reference operator*() const = 0;
        virtual pointer operator->() = 0;
        virtual const_pointer operator->() const = 0;
        // modifiers
    public:
        virtual void reset() = 0;
        virtual i_optional<T>& operator=(const i_optional<T>& rhs) = 0;
        virtual i_optional<T>& operator=(const T& value) = 0;
    };

    template <typename T>
    inline bool operator==(const i_optional<T>& lhs, std::nullopt_t)
    {
        return !lhs.valid();
    }
        
    template <typename T>
    inline bool operator!=(const i_optional<T>& lhs, std::nullopt_t)
    {
        return lhs.valid();
    }

    template <typename T>
    inline bool operator==(const i_optional<T>& lhs, const i_optional<T>& rhs)
    {
        if (lhs.valid() != rhs.valid())
            return false;
        if (!lhs.valid())
            return true;
        return lhs.get() == rhs.get();
    }

    template <typename T>
    inline bool operator!=(const i_optional<T>& lhs, const i_optional<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename T>
    inline bool operator<(const i_optional<T>& lhs, const i_optional<T>& rhs)
    {
        if (lhs.valid() != rhs.valid())
            return lhs.valid() < rhs.valid();
        if (!lhs.valid())
            return false;
        return lhs.get() < rhs.get();
    }
}
