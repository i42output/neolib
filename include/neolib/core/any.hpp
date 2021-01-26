// any.hpp
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
#include <any>
#include <stdexcept>
#include <typeinfo>

namespace neolib
{
    class any : private std::any
    {
    public:
        typedef any abstract_type; // todo
    private:
        template<class T>
        friend T any_cast(const any& operand);
        template<class T>
        friend T any_cast(any& operand);
        template<class T>
        friend T any_cast(any&& operand);
        template<class T>
        friend const T* any_cast(const any* operand) noexcept;
        template<class T>
        friend T* any_cast(any* operand) noexcept;
        template<class T>
        friend T unsafe_any_cast(const any& operand) noexcept;
        template<class T>
        friend T unsafe_any_cast(any& operand) noexcept;
    public:
        any() : 
            cptr{ nullptr }, 
            ptr{ nullptr }
        {
        }
        any(const any& aOther) : 
            std::any{ aOther.as_std_any() }, 
            cptr{ aOther.cptr }, 
            ptr{ aOther.ptr }
        {
        }
        any(any&& aOther) :
            std::any{ std::move(aOther.as_std_any()) }, 
            cptr{ aOther.cptr },
            ptr{ aOther.ptr }
        {
            aOther.cptr = nullptr;
            aOther.ptr = nullptr;
        }
        template <typename ValueType, typename SFINAE = void>
        any(ValueType&& aValue, typename std::enable_if<!std::is_same<std::decay_t<ValueType>, any>::value, SFINAE>::type* = nullptr) :
            std::any{ std::decay_t<ValueType>{aValue} },
            cptr{ &any::do_cptr<std::decay_t<ValueType>> },
            ptr{ &any::do_ptr<std::decay_t<ValueType>> }
        {
        }
    public:
        any& operator=(const any& aRhs)
        {
            std::any::operator=(aRhs.as_std_any());
            cptr = aRhs.cptr;
            ptr = aRhs.ptr;
            return *this;
        }
        any& operator=(any&& aRhs)
        {
            std::any::operator=(std::move(aRhs.as_std_any()));
            cptr = aRhs.cptr;
            ptr = aRhs.ptr;
            aRhs.cptr = nullptr;
            aRhs.ptr = nullptr;
            return *this;
        }
        template<typename ValueType>
        any& operator=(ValueType&& aRhs)
        {
            std::any::operator=(std::decay_t<ValueType>{aRhs});
            cptr = &any::do_cptr<std::decay_t<ValueType>>;
            ptr = &any::do_ptr<std::decay_t<ValueType>>;
            return *this;
        }
    public:
        template<class ValueType, class... Args >
        std::decay_t<ValueType>& emplace(Args&&... args)
        {
            auto& result = std::any::emplace<ValueType>(std::forward<Args...>(args...));
            cptr = &any::do_cptr<std::decay_t<ValueType>>;
            ptr = &any::do_ptr<std::decay_t<ValueType>>;
            return result;
        }
        template<class ValueType, class U, class... Args >
        std::decay_t<ValueType>& emplace(std::initializer_list<U> il, Args&&... args)
        {
            auto& result = std::any::emplace<ValueType>(il, std::forward<Args...>(args...));
            cptr = &any::do_cptr<std::decay_t<ValueType>>;
            ptr = &any::do_ptr<std::decay_t<ValueType>>;
            return result;
        }
        void reset()
        {
            std::any::reset();
            cptr = nullptr;
            ptr = nullptr;
        }
        void swap(any& aOther)
        {
            std::any::swap(aOther.as_std_any());
            std::swap(cptr, aOther.cptr);
            std::swap(ptr, aOther.ptr);
        }
    public:
        using std::any::has_value;
        using std::any::type;
    public:
        bool operator==(const any& aOther) const
        {
            if (cptr != nullptr && aOther.cptr != nullptr)
                return cptr(*this) == aOther.cptr(aOther);
            else
                return cptr == aOther.cptr;
        }
        bool operator!=(const any& aOther) const
        {
            if (cptr != nullptr && aOther.cptr != nullptr)
                return cptr(*this) != aOther.cptr(aOther);
            else
                return cptr != aOther.cptr;
        }
        bool operator<(const any& aOther) const
        {
            if (cptr != nullptr && aOther.cptr != nullptr)
                return cptr(*this) < aOther.cptr(aOther);
            else
                return cptr < aOther.cptr;
        }
    private:
        const std::any& as_std_any() const
        {
            return *this;
        }
        std::any& as_std_any()
        {
            return *this;
        }
        const void* unsafe_ptr() const
        {
            if (cptr != nullptr)
                return cptr(*this);
            else
                return nullptr;
        }
        void* unsafe_ptr()
        {
            if (cptr != nullptr)
                return ptr(*this);
            else
                return nullptr;
        }
        template <typename T>
        static const void* do_cptr(const any& aArg)
        {
            return &std::any_cast<const T&>(aArg.as_std_any());
        }
        template <typename T>
        static void* do_ptr(any& aArg)
        {
            return &std::any_cast<T&>(aArg.as_std_any());
        }
    private:
        const void*(*cptr)(const any&);
        void*(*ptr)(any&);
    };

    inline void swap(any& aLhs, any& aRhs)
    {
        aLhs.swap(aRhs);
    }

    template<class T>
    inline T any_cast(const any& operand)
    {
        return std::any_cast<T>(operand.as_std_any());
    }
    template<class T>
    inline T any_cast(any& operand)
    {
        return std::any_cast<T>(operand.as_std_any());
    }
    template<class T>
    inline T any_cast(any&& operand)
    {
        return std::any_cast<T>(std::move(operand.as_std_any()));
    }
    template<class T>
    inline const T* any_cast(const any* operand) noexcept
    {
        return std::any_cast<const T*>(&operand->as_std_any());
    }
    template<class T>
    inline T* any_cast(any* operand) noexcept
    {
        return std::any_cast<T*>(&operand->as_std_any());
    }
    template<class T>
    inline T unsafe_any_cast(const any& operand) noexcept
    {
        return *static_cast<const std::decay_t<T>*>(operand.unsafe_ptr());
    }
    template<class T>
    inline T unsafe_any_cast(any& operand) noexcept
    {
        return *static_cast<std::decay_t<T>*>(operand.unsafe_ptr());
    }
}
