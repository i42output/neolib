// i_any.hpp
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

#include <typeinfo>
#include <any>

namespace neolib
{
    class i_any
    {
    public:
        typedef i_any abstract_type;
    private:
        template<class T>
        friend T any_cast(const i_any& operand);
        template<class T>
        friend T any_cast(i_any& operand);
        template<class T>
        friend T any_cast(i_any&& operand);
        template<class T>
        friend const T* any_cast(const i_any* operand) noexcept;
        template<class T>
        friend T* any_cast(i_any* operand) noexcept;
        template<class T>
        friend T unsafe_any_cast(const i_any& operand) noexcept;
        template<class T>
        friend T unsafe_any_cast(i_any& operand) noexcept;
    public:
        virtual ~i_any() = default;
    public:
        virtual void reset() = 0;
    public:
        virtual bool has_value() const = 0;
        virtual std::type_info const& type() const = 0;
    public:
        virtual bool operator==(const i_any& aOther) const = 0;
        virtual bool operator!=(const i_any& aOther) const = 0;
        virtual bool operator<(const i_any& aOther) const = 0;
    private:
        virtual const std::any& as_std_any() const = 0;
        virtual std::any& as_std_any() = 0;
        virtual const void* unsafe_ptr() const = 0;
        virtual void* unsafe_ptr() = 0;
    };

    template<class T>
    inline T any_cast(const i_any& operand)
    {
        return std::any_cast<T>(operand.as_std_any());
    }
    template<class T>
    inline T any_cast(i_any& operand)
    {
        return std::any_cast<T>(operand.as_std_any());
    }
    template<class T>
    inline T any_cast(i_any&& operand)
    {
        return std::any_cast<T>(std::move(operand.as_std_any()));
    }
    template<class T>
    inline const T* any_cast(const i_any* operand) noexcept
    {
        return std::any_cast<const T*>(&operand->as_std_any());
    }
    template<class T>
    inline T* any_cast(i_any* operand) noexcept
    {
        return std::any_cast<T*>(&operand->as_std_any());
    }
    template<class T>
    inline T unsafe_any_cast(const i_any& operand) noexcept
    {
        return *static_cast<const std::decay_t<T>*>(operand.unsafe_ptr());
    }
    template<class T>
    inline T unsafe_any_cast(i_any& operand) noexcept
    {
        return *static_cast<std::decay_t<T>*>(operand.unsafe_ptr());
    }
}
