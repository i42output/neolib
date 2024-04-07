// numerical.hpp
/*
 *  Copyright (c) 2015, 2020, 2024 Leigh Johnston.
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
#include <stdexcept>
#include <vector>
#include <utility>
#include <array>
#include <ranges>
#include <algorithm>
#include <ostream>
#include <istream>
#include <boost/math/constants/constants.hpp>
#include <neolib/core/vecarray.hpp>
#include <neolib/core/swizzle.hpp>
#include <neolib/core/simd.hpp>
#include <neolib/core/optional.hpp>
#include <neolib/core/string_utils.hpp>

namespace neolib
{ 
    namespace math
    {
#define USE_AVX
#define USE_EMM

        using namespace boost::math::constants;

        using scalar    = double;
        using angle     = double;

        namespace constants
        {
            template <typename T>
            constexpr T zero = static_cast<T>(0.0);
            template <typename T>
            constexpr T one = static_cast<T>(1.0);
            template <typename T>
            constexpr T two = static_cast<T>(2.0);
            template <typename T>
            constexpr T three = static_cast<T>(3.0);
            template <typename T>
            constexpr T four = static_cast<T>(4.0);
        }

        template <typename T, typename SFINAE = std::enable_if_t<std::is_scalar_v<T>, sfinae>>
        inline T lerp(T aX1, T aX2, double aAmount)
        {
            double x1 = aX1;
            double x2 = aX2;
            return static_cast<T>((x2 - x1) * aAmount + x1);
        }

        inline angle to_rad(angle aDegrees)
        {
            return aDegrees / 180.0 * pi<angle>();
        }

        inline angle to_deg(angle aRadians)
        {
            return aRadians * 180.0 / pi<angle>();
        }

        struct column_vector {};
        struct row_vector {};

        template <typename T, std::uint32_t _Size, typename Type = column_vector>
        class basic_vector;
    }

    namespace math
    {
        template <typename T, std::uint32_t _Size, typename Type>
        class basic_vector
        {
        public:
            using abstract_type     = basic_vector; // todo: abstract base; std::array?
        public:
            using type              = Type;
        public:
            using value_type        = T;
            using vector_type       = basic_vector<value_type, _Size, Type>;
            using size_type         = std::uint32_t;
            using array_type        = std::array<value_type, _Size>;
            using const_iterator    = typename array_type::const_iterator;
            using iterator          = typename array_type::iterator;
        public:
            template <std::uint32_t Size2> struct rebind { using type = basic_vector<T, Size2, Type>; };
        public:
            static constexpr std::uint32_t Size = _Size;
        public:
            basic_vector() : v{} {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, typename std::enable_if_t<Size == 1, SFINAE> = 0) : v{ {x} } {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, value_type y, typename std::enable_if_t<Size == 2, SFINAE> = 0) : v{ {x, y} } {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, value_type y, value_type z, typename std::enable_if_t<Size == 3, SFINAE> = 0) : v{ {x, y, z} } {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, value_type y, value_type z, value_type w, typename std::enable_if_t<Size == 4, SFINAE> = 0) : v{ { x, y, z, w } } {}
            template <typename... Arguments>
            explicit basic_vector(const value_type& value, Arguments&&... aArguments) : v{ {value, std::forward<Arguments>(aArguments)...} } {}
            template <typename... Arguments>
            explicit basic_vector(value_type&& value, Arguments&&... aArguments) : v{ {std::move(value), std::forward<Arguments>(aArguments)...} } {}
            explicit basic_vector(const array_type& v) : v{ v } {}
            basic_vector(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neolib::basic_vector: initializer list too big"); std::uninitialized_copy(values.begin(), values.end(), v.begin()); std::uninitialized_fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); }
            template <typename V, typename A, std::uint32_t S, std::uint32_t... Indexes>
            basic_vector(const swizzle<V, A, S, Indexes...>& aSwizzle) : basic_vector{ ~aSwizzle } {}
            basic_vector(const basic_vector& other) : v{ other.v } {}
            basic_vector(basic_vector&& other) : v{ std::move(other.v) } {}
            template <typename T2>
            basic_vector(const basic_vector<T2, Size, Type>& other) { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            template <typename T2, std::uint32_t Size2, typename SFINAE = int>
            basic_vector(const basic_vector<T2, Size2, Type>& other, typename std::enable_if_t<Size2 < Size, SFINAE> = 0) : v{} { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            basic_vector& operator=(const basic_vector& other) { v = other.v; return *this; }
            basic_vector& operator=(basic_vector&& other) { v = std::move(other.v); return *this; }
            basic_vector& operator=(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neolib::basic_vector: initializer list too big"); std::copy(values.begin(), values.end(), v.begin()); std::fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); return *this; }
        public:
            static std::uint32_t size() { return Size; }
            value_type operator[](std::uint32_t aIndex) const { return v[aIndex]; }
            value_type& operator[](std::uint32_t aIndex) { return v[aIndex]; }
            const_iterator begin() const { return v.begin(); }
            const_iterator end() const { return v.end(); }
            iterator begin() { return v.begin(); }
            iterator end() { return v.end(); }
            operator const array_type&() const { return v; }
        public:
            template <typename T2>
            basic_vector<T2, Size, Type> as() const
            {
                return basic_vector<T2, Size, Type>{ *this };
            }
        public:
            bool operator==(const basic_vector& right) const { return v == right.v; }
            bool operator!=(const basic_vector& right) const { return v != right.v; }
            basic_vector& operator+=(value_type value) { for (auto& e : v) e += value; return *this; }
            basic_vector& operator-=(value_type value) { for (auto& e : v) e -= value; return *this; }
            basic_vector& operator*=(value_type value) { for (auto& e : v) e *= value; return *this; }
            basic_vector& operator/=(value_type value) { for (auto& e : v) e /= value; return *this; }
            basic_vector& operator+=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::plus{}); return *this; }
            basic_vector& operator-=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::minus{}); return *this; }
            basic_vector& operator*=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::multiplies{}); return *this; }
            basic_vector& operator/=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::divides{}); return *this; }
            basic_vector operator-() const { basic_vector result; std::ranges::transform(v, result.v.begin(), std::negate{});; return result; }
            basic_vector scale(const basic_vector& right) const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = v[index] * right[index]; return result; }
            value_type magnitude() const { value_type ss = constants::zero<value_type>; for (std::uint32_t index = 0; index < Size; ++index) ss += (v[index] * v[index]); return std::sqrt(ss); }
            basic_vector normalized() const { basic_vector result; value_type im = constants::one<value_type> / magnitude(); for (std::uint32_t index = 0; index < Size; ++index) result.v[index] = v[index] * im; return result; }
            basic_vector min(const basic_vector& right) const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::min(v[index], right.v[index]); return result; }
            basic_vector max(const basic_vector& right) const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::max(v[index], right.v[index]); return result; }
            value_type min() const { value_type result = v[0]; for (std::uint32_t index = 1; index < Size; ++index) result = std::min(v[index], result); return result; }
            basic_vector ceil() const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::ceil(v[index]); return result; }
            basic_vector floor() const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::floor(v[index]); return result; }
            basic_vector round() const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::round(v[index]); return result; }
            value_type distance(const basic_vector& right) const { value_type total = 0; for (std::uint32_t index = 0; index < Size; ++index) total += ((v[index] - right.v[index]) * (v[index] - right.v[index])); return std::sqrt(total); }
            value_type dot(const basic_vector& right) const
            {
                value_type result = constants::zero<value_type>;
                for (std::uint32_t index = 0; index < Size; ++index)
                    result += (v[index] * right[index]);
                return result;
            }
            template <typename SFINAE = basic_vector>
            std::enable_if_t<Size == 3, SFINAE> cross(const basic_vector& right) const
            {
                return basic_vector{ 
                    y * right.z - z * right.y, 
                    z * right.x - x * right.z, 
                    x * right.y - y * right.x };
            }
            basic_vector hadamard_product(const basic_vector& right) const
            {
                basic_vector result = *this;
                result *= right;
                return result;
            }
        public:
            friend void swap(basic_vector& a, basic_vector& b)
            {
                using std::swap;
                swap(a.v, b.v);
            }
        public:
            union
            {
                array_type v;
                struct // todo: alignment, padding?
                {
                    value_type x;
                    value_type y;
                    value_type z;
                };
                swizzle<vector_type, array_type, 2, 0, 0> xx;
                swizzle<vector_type, array_type, 2, 0, 1> xy;
                swizzle<vector_type, array_type, 2, 0, 2> xz;
                swizzle<vector_type, array_type, 2, 1, 0> yx;
                swizzle<vector_type, array_type, 2, 1, 1> yy;
                swizzle<vector_type, array_type, 2, 1, 2> yz;
                swizzle<vector_type, array_type, 2, 2, 0> zx;
                swizzle<vector_type, array_type, 2, 2, 1> zy;
                swizzle<vector_type, array_type, 2, 2, 2> zz;
                swizzle<vector_type, array_type, 3, 0, 0, 0> xxx;
                swizzle<vector_type, array_type, 3, 0, 0, 1> xxy;
                swizzle<vector_type, array_type, 3, 0, 0, 2> xxz;
                swizzle<vector_type, array_type, 3, 0, 1, 0> xyx;
                swizzle<vector_type, array_type, 3, 0, 1, 1> xyy;
                swizzle<vector_type, array_type, 3, 0, 1, 2> xyz;
                swizzle<vector_type, array_type, 3, 1, 0, 0> yxx;
                swizzle<vector_type, array_type, 3, 1, 0, 1> yxy;
                swizzle<vector_type, array_type, 3, 1, 0, 2> yxz;
                swizzle<vector_type, array_type, 3, 1, 1, 0> yyx;
                swizzle<vector_type, array_type, 3, 1, 1, 1> yyy;
                swizzle<vector_type, array_type, 3, 1, 1, 2> yyz;
                swizzle<vector_type, array_type, 3, 1, 2, 0> yzx;
                swizzle<vector_type, array_type, 3, 1, 2, 1> yzy;
                swizzle<vector_type, array_type, 3, 1, 2, 2> yzz;
                swizzle<vector_type, array_type, 3, 2, 0, 0> zxx;
                swizzle<vector_type, array_type, 3, 2, 0, 1> zxy;
                swizzle<vector_type, array_type, 3, 2, 0, 2> zxz;
                swizzle<vector_type, array_type, 3, 2, 1, 0> zyx;
                swizzle<vector_type, array_type, 3, 2, 1, 1> zyy;
                swizzle<vector_type, array_type, 3, 2, 1, 2> zyz;
                swizzle<vector_type, array_type, 3, 2, 2, 0> zzx;
                swizzle<vector_type, array_type, 3, 2, 2, 1> zzy;
                swizzle<vector_type, array_type, 3, 2, 2, 2> zzz;
            };
        };

        template <typename T, typename Type>
        class basic_vector<T, 2, Type>
        {
        public:
            using abstract_type     = basic_vector; // todo: abstract base; std::array?
        public:
            using type              = Type;
        public:
            using value_type        = T;
            using vector_type       = basic_vector<value_type, 2, Type>;
            using size_type         = std::uint32_t;
            using array_type        = std::array<value_type, 2>;
            using const_iterator    = typename array_type::const_iterator;
            using iterator          = typename array_type::iterator;
        public:
            template <std::uint32_t Size2> struct rebind { using type = basic_vector<T, Size2, Type>; };
        public:
            static constexpr std::uint32_t Size = 2;
        public:
            basic_vector() : v{} {}
            explicit basic_vector(value_type x, value_type y) : v{ {x, y} } {}
            template <typename... Arguments>
            explicit basic_vector(const value_type& value, Arguments&&... aArguments) : v{ {value, std::forward<Arguments>(aArguments)...} } {}
            template <typename... Arguments>
            explicit basic_vector(value_type&& value, Arguments&&... aArguments) : v{ {std::move(value), std::forward<Arguments>(aArguments)...} } {}
            explicit basic_vector(const array_type& v) : v{ v } {}
            basic_vector(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neolib::basic_vector: initializer list too big"); std::uninitialized_copy(values.begin(), values.end(), v.begin()); std::uninitialized_fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); }
            template <typename V, typename A, std::uint32_t S, std::uint32_t... Indexes>
            basic_vector(const swizzle<V, A, S, Indexes...>& aSwizzle) : basic_vector{ ~aSwizzle } {}
            basic_vector(const basic_vector& other) : v{ other.v } {}
            basic_vector(basic_vector&& other) : v{ std::move(other.v) } {}
            template <typename T2>
            basic_vector(const basic_vector<T2, Size, Type>& other) { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            template <typename T2, std::uint32_t Size2, typename SFINAE = int>
            basic_vector(const basic_vector<T2, Size2, Type>& other, typename std::enable_if_t < Size2 < Size, SFINAE> = 0) : v{} { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            basic_vector& operator=(const basic_vector& other) { v = other.v; return *this; }
            basic_vector& operator=(basic_vector&& other) { v = std::move(other.v); return *this; }
            basic_vector& operator=(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neolib::basic_vector: initializer list too big"); std::copy(values.begin(), values.end(), v.begin()); std::fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); return *this; }
        public:
            static std::uint32_t size() { return Size; }
            value_type operator[](std::uint32_t aIndex) const { return v[aIndex]; }
            value_type& operator[](std::uint32_t aIndex) { return v[aIndex]; }
            const_iterator begin() const { return v.begin(); }
            const_iterator end() const { return v.end(); }
            iterator begin() { return v.begin(); }
            iterator end() { return v.end(); }
            operator const array_type& () const { return v; }
        public:
            template <typename T2>
            basic_vector<T2, Size, Type> as() const
            {
                return basic_vector<T2, Size, Type>{ *this };
            }
        public:
            bool operator==(const basic_vector& right) const { return v == right.v; }
            bool operator!=(const basic_vector& right) const { return v != right.v; }
            basic_vector& operator+=(value_type value) { for (auto& e : v) e += value; return *this; }
            basic_vector& operator-=(value_type value) { for (auto& e : v) e -= value; return *this; }
            basic_vector& operator*=(value_type value) { for (auto& e : v) e *= value; return *this; }
            basic_vector& operator/=(value_type value) { for (auto& e : v) e /= value; return *this; }
            basic_vector& operator+=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::plus{}); return *this; }
            basic_vector& operator-=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::minus{}); return *this; }
            basic_vector& operator*=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::multiplies{}); return *this; }
            basic_vector& operator/=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::divides{}); return *this; }
            basic_vector operator-() const { basic_vector result; std::ranges::transform(v, result.v.begin(), std::negate{});; return result; }
            basic_vector scale(const basic_vector& right) const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = v[index] * right[index]; return result; }
            value_type magnitude() const { value_type ss = constants::zero<value_type>; for (std::uint32_t index = 0; index < Size; ++index) ss += (v[index] * v[index]); return std::sqrt(ss); }
            basic_vector normalized() const { basic_vector result; value_type im = constants::one<value_type> / magnitude(); for (std::uint32_t index = 0; index < Size; ++index) result.v[index] = v[index] * im; return result; }
            basic_vector min(const basic_vector& right) const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::min(v[index], right.v[index]); return result; }
            basic_vector max(const basic_vector& right) const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::max(v[index], right.v[index]); return result; }
            value_type min() const { value_type result = v[0]; for (std::uint32_t index = 1; index < Size; ++index) result = std::min(v[index], result); return result; }
            basic_vector ceil() const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::ceil(v[index]); return result; }
            basic_vector floor() const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::floor(v[index]); return result; }
            basic_vector round() const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::round(v[index]); return result; }
            value_type distance(const basic_vector& right) const { value_type total = 0; for (std::uint32_t index = 0; index < Size; ++index) total += ((v[index] - right.v[index]) * (v[index] - right.v[index])); return std::sqrt(total); }
            value_type dot(const basic_vector& right) const
            {
                value_type result = constants::zero<value_type>;
                for (std::uint32_t index = 0; index < Size; ++index)
                    result += (v[index] * right[index]);
                return result;
            }
            basic_vector hadamard_product(const basic_vector& right) const
            {
                basic_vector result = *this;
                result *= right;
                return result;
            }
        public:
            friend void swap(basic_vector& a, basic_vector& b)
            {
                using std::swap;
                swap(a.v, b.v);
            }
        public:
            union
            {
                array_type v;
                struct // todo: alignment, padding?
                {
                    value_type x;
                    value_type y;
                };
                swizzle<vector_type, array_type, 2, 0, 0> xx;
                swizzle<vector_type, array_type, 2, 0, 1> xy;
                swizzle<vector_type, array_type, 2, 1, 0> yx;
                swizzle<vector_type, array_type, 2, 1, 1> yy;
                swizzle<vector_type, array_type, 3, 0, 0, 0> xxx;
                swizzle<vector_type, array_type, 3, 0, 0, 1> xxy;
                swizzle<vector_type, array_type, 3, 0, 1, 0> xyx;
                swizzle<vector_type, array_type, 3, 0, 1, 1> xyy;
                swizzle<vector_type, array_type, 3, 1, 0, 0> yxx;
                swizzle<vector_type, array_type, 3, 1, 0, 1> yxy;
                swizzle<vector_type, array_type, 3, 1, 1, 0> yyx;
                swizzle<vector_type, array_type, 3, 1, 1, 1> yyy;
            };
        };

        template <typename T, typename Type>
        class basic_vector<T, 1, Type>
        {
        public:
            using abstract_type     = basic_vector; // todo: abstract base; std::array?
        public:
            using type              = Type;
        public:
            using value_type        = T;
            using vector_type       = basic_vector<value_type, 1, Type>;
            using size_type         = std::uint32_t;
            using array_type        = std::array<value_type, 1>;
            using const_iterator    = typename array_type::const_iterator;
            using iterator          = typename array_type::iterator;
        public:
            template <std::uint32_t Size2> struct rebind { using type = basic_vector<T, Size2, Type>; };
        public:
            static constexpr std::uint32_t Size = 1;
        public:
            basic_vector() : v{} {}
            explicit basic_vector(value_type x) : v{ {x} } {}
            template <typename... Arguments>
            explicit basic_vector(const value_type& value, Arguments&&... aArguments) : v{ {value, std::forward<Arguments>(aArguments)...} } {}
            template <typename... Arguments>
            explicit basic_vector(value_type&& value, Arguments&&... aArguments) : v{ {std::move(value), std::forward<Arguments>(aArguments)...} } {}
            explicit basic_vector(const array_type& v) : v{ v } {}
            basic_vector(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neolib::basic_vector: initializer list too big"); std::uninitialized_copy(values.begin(), values.end(), v.begin()); std::uninitialized_fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); }
            template <typename V, typename A, std::uint32_t S, std::uint32_t... Indexes>
            basic_vector(const swizzle<V, A, S, Indexes...>& aSwizzle) : basic_vector{ ~aSwizzle } {}
            basic_vector(const basic_vector& other) : v{ other.v } {}
            basic_vector(basic_vector&& other) : v{ std::move(other.v) } {}
            template <typename T2>
            basic_vector(const basic_vector<T2, Size, Type>& other) { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            template <typename T2, std::uint32_t Size2, typename SFINAE = int>
            basic_vector(const basic_vector<T2, Size2, Type>& other, typename std::enable_if_t < Size2 < Size, SFINAE> = 0) : v{} { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            basic_vector& operator=(const basic_vector& other) { v = other.v; return *this; }
            basic_vector& operator=(basic_vector&& other) { v = std::move(other.v); return *this; }
            basic_vector& operator=(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neolib::basic_vector: initializer list too big"); std::copy(values.begin(), values.end(), v.begin()); std::fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); return *this; }
        public:
            static std::uint32_t size() { return Size; }
            value_type operator[](std::uint32_t aIndex) const { return v[aIndex]; }
            value_type& operator[](std::uint32_t aIndex) { return v[aIndex]; }
            const_iterator begin() const { return v.begin(); }
            const_iterator end() const { return v.end(); }
            iterator begin() { return v.begin(); }
            iterator end() { return v.end(); }
            operator const array_type& () const { return v; }
        public:
            template <typename T2>
            basic_vector<T2, Size, Type> as() const
            {
                return basic_vector<T2, Size, Type>{ *this };
            }
        public:
            bool operator==(const basic_vector& right) const { return v == right.v; }
            bool operator!=(const basic_vector& right) const { return v != right.v; }
            basic_vector& operator+=(value_type value) { for (auto& e : v) e += value; return *this; }
            basic_vector& operator-=(value_type value) { for (auto& e : v) e -= value; return *this; }
            basic_vector& operator*=(value_type value) { for (auto& e : v) e *= value; return *this; }
            basic_vector& operator/=(value_type value) { for (auto& e : v) e /= value; return *this; }
            basic_vector& operator+=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::plus{}); return *this; }
            basic_vector& operator-=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::minus{}); return *this; }
            basic_vector& operator*=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::multiplies{}); return *this; }
            basic_vector& operator/=(const basic_vector& right) { std::ranges::transform(v, right.v, v.begin(), std::divides{}); return *this; }
            basic_vector operator-() const { basic_vector result; std::ranges::transform(v, result.v.begin(), std::negate{});; return result; }
            basic_vector scale(const basic_vector& right) const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = v[index] * right[index]; return result; }
            value_type magnitude() const { value_type ss = constants::zero<value_type>; for (std::uint32_t index = 0; index < Size; ++index) ss += (v[index] * v[index]); return std::sqrt(ss); }
            basic_vector normalized() const { basic_vector result; value_type im = constants::one<value_type> / magnitude(); for (std::uint32_t index = 0; index < Size; ++index) result.v[index] = v[index] * im; return result; }
            basic_vector min(const basic_vector& right) const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::min(v[index], right.v[index]); return result; }
            basic_vector max(const basic_vector& right) const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::max(v[index], right.v[index]); return result; }
            value_type min() const { value_type result = v[0]; for (std::uint32_t index = 1; index < Size; ++index) result = std::min(v[index], result); return result; }
            basic_vector ceil() const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::ceil(v[index]); return result; }
            basic_vector floor() const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::floor(v[index]); return result; }
            basic_vector round() const { basic_vector result; for (std::uint32_t index = 0; index < Size; ++index) result[index] = std::round(v[index]); return result; }
            value_type distance(const basic_vector& right) const { value_type total = 0; for (std::uint32_t index = 0; index < Size; ++index) total += ((v[index] - right.v[index]) * (v[index] - right.v[index])); return std::sqrt(total); }
            value_type dot(const basic_vector& right) const
            {
                value_type result = constants::zero<value_type>;
                for (std::uint32_t index = 0; index < Size; ++index)
                    result += (v[index] * right[index]);
                return result;
            }
            basic_vector hadamard_product(const basic_vector& right) const
            {
                basic_vector result = *this;
                result *= right;
                return result;
            }
        public:
            friend void swap(basic_vector& a, basic_vector& b)
            {
                using std::swap;
                swap(a.v, b.v);
            }
        public:
            union
            {
                array_type v;
                struct // todo: alignment, padding?
                {
                    value_type x;
                };
                swizzle<vector_type, array_type, 2, 0, 0> xx;
                swizzle<vector_type, array_type, 3, 0, 0, 0> xxx;
            };
        };

        template <typename T, std::uint32_t Size, typename Type>
        inline bool operator==(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v == aRhs.v;
        }

        template <typename T, std::uint32_t Size, typename Type>
        inline bool operator<(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v < aRhs.v;
        }

        template <typename T, std::uint32_t Size, typename Type>
        inline std::partial_ordering operator<=>(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v <=> aRhs.v;
        }

        using vector1 = basic_vector<double, 1>;
        using vector2 = basic_vector<double, 2>;
        using vector3 = basic_vector<double, 3>;
        using vector4 = basic_vector<double, 4>;

        using vec1 = vector1;
        using vec2 = vector2;
        using vec3 = vector3;
        using vec4 = vector4;

        using col_vec1 = vec1;
        using col_vec2 = vec2;
        using col_vec3 = vec3;
        using col_vec4 = vec4;

        using row_vec1 = basic_vector<double, 1, row_vector>;
        using row_vec2 = basic_vector<double, 2, row_vector>;
        using row_vec3 = basic_vector<double, 3, row_vector>;
        using row_vec4 = basic_vector<double, 4, row_vector>;

        using optional_vector1 = optional<vector1>;
        using optional_vector2 = optional<vector2>;
        using optional_vector3 = optional<vector3>;
        using optional_vector4 = optional<vector4>;

        using optional_vec1 = optional<vec1>;
        using optional_vec2 = optional<vec2>;
        using optional_vec3 = optional<vec3>;
        using optional_vec4 = optional<vec4>;

        using optional_col_vec1 = optional<col_vec1>;
        using optional_col_vec2 = optional<col_vec2>;
        using optional_col_vec3 = optional<col_vec3>;
        using optional_col_vec4 = optional<col_vec4>;

        using optional_row_vec1 = optional<row_vec1>;
        using optional_row_vec2 = optional<row_vec2>;
        using optional_row_vec3 = optional<row_vec3>;
        using optional_row_vec4 = optional<row_vec4>;

        using vec2_list = std::vector<vec2>;
        using vec3_list = std::vector<vec3>;

        using optional_vec2_list = optional<vec2_list>;
        using optional_vec3_list = optional<vec3_list>;

        using vertices_2d = vec2_list;
        using vertices    = vec3_list;

        using optional_vertices_2d_t = optional_vec2_list;
        using optional_vertices_t    = optional_vec3_list;

        using vector1f = basic_vector<float, 1>;
        using vector2f = basic_vector<float, 2>;
        using vector3f = basic_vector<float, 3>;
        using vector4f = basic_vector<float, 4>;

        using vec1f = vector1f;
        using vec2f = vector2f;
        using vec3f = vector3f;
        using vec4f = vector4f;

        using i32 = int32_t;
        using i64 = int64_t;

        using vector1i32 = basic_vector<i32, 1>;
        using vector2i32 = basic_vector<i32, 2>;
        using vector3i32 = basic_vector<i32, 3>;
        using vector4i32 = basic_vector<i32, 4>;

        using vec1i32 = vector1i32;
        using vec2i32 = vector2i32;
        using vec3i32 = vector3i32;
        using vec4i32 = vector4i32;

        using u32 = std::uint32_t;
        using u64 = std::uint32_t;

        using vector1u32 = basic_vector<u32, 1>;
        using vector2u32 = basic_vector<u32, 2>;
        using vector3u32 = basic_vector<u32, 3>;
        using vector4u32 = basic_vector<u32, 4>;

        using vec1u32 = vector1u32;
        using vec2u32 = vector2u32;
        using vec3u32 = vector3u32;
        using vec4u32 = vector4u32;

        template <std::size_t VertexCount>
        using vec3_array = neolib::vecarray<vec3, VertexCount, VertexCount>;

        template <std::size_t VertexCount>
        using vec2_array = neolib::vecarray<vec2, VertexCount, VertexCount>;

        using avec1i8 = std::array<int8_t, 1>;
        using avec2i8 = std::array<int8_t, 2>;
        using avec3i8 = std::array<int8_t, 3>;
        using avec4i8 = std::array<int8_t, 4>;

        using avec1i16 = std::array<int16_t, 1>;
        using avec2i16 = std::array<int16_t, 2>;
        using avec3i16 = std::array<int16_t, 3>;
        using avec4i16 = std::array<int16_t, 4>;

        using avec1i32 = std::array<int32_t, 1>;
        using avec2i32 = std::array<int32_t, 2>;
        using avec3i32 = std::array<int32_t, 3>;
        using avec4i32 = std::array<int32_t, 4>;

        using avec1u8 = std::array<uint8_t, 1>;
        using avec2u8 = std::array<uint8_t, 2>;
        using avec3u8 = std::array<uint8_t, 3>;
        using avec4u8 = std::array<uint8_t, 4>;

        using avec1u16 = std::array<uint16_t, 1>;
        using avec2u16 = std::array<uint16_t, 2>;
        using avec3u16 = std::array<uint16_t, 3>;
        using avec4u16 = std::array<uint16_t, 4>;

        using avec1u32 = std::array<std::uint32_t, 1>;
        using avec2u32 = std::array<std::uint32_t, 2>;
        using avec3u32 = std::array<std::uint32_t, 3>;
        using avec4u32 = std::array<std::uint32_t, 4>;

        using avec1f = std::array<float, 1>;
        using avec2f = std::array<float, 2>;
        using avec3f = std::array<float, 3>;
        using avec4f = std::array<float, 4>;

        using avec1 = std::array<double, 1>;
        using avec2 = std::array<double, 2>;
        using avec3 = std::array<double, 3>;
        using avec4 = std::array<double, 4>;

        using triangle  = std::array<vec3, 3>;
        using quad      = std::array<vec3, 4>;

        using triangle_2d   = std::array<vec2, 3>;
        using quad_2d       = std::array<vec2, 4>;

        using trianglef = std::array<vec3f, 3>;
        using quadf     = std::array<vec3f, 4>;

        using trianglef_2d  = std::array<vec2f, 3>;
        using quadf_2d      = std::array<vec2f, 4>;

        template <typename T, std::uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator+(const basic_vector<T, D, Type>& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result = left;
            result += right;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator-(const basic_vector<T, D, Type>& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result = left;
            result -= right;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount> operator+(const basic_vector<T, D, Type>& left, const std::array<basic_vector<T, D, Type>, VertexCount>& right)
        {
            std::array<basic_vector<T, D, Type>, VertexCount> result = right;
            for (auto& v : result)
                v += left;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount> operator+(const std::array<basic_vector<T, D, Type>, VertexCount>& left, const basic_vector<T, D, Type>& right)
        {
            std::array<basic_vector<T, D, Type>, VertexCount> result = left;
            for (auto& v : result)
                v += right;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount>& operator+=(std::array<basic_vector<T, D, Type>, VertexCount>& left, const basic_vector<T, D, Type>& right)
        {
            for (auto& v : left)
                v += right;
            return left;
        }

        template <typename T, std::uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount> operator-(const basic_vector<T, D, Type>& left, const std::array<basic_vector<T, D, Type>, VertexCount>& right)
        {
            std::array<basic_vector<T, D, Type>, VertexCount> result = right;
            for (auto& v : result)
                v = left - v;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount> operator-(const std::array<basic_vector<T, D, Type>, VertexCount>& left, const basic_vector<T, D, Type>& right)
        {
            std::array<basic_vector<T, D, Type>, VertexCount> result = left;
            for (auto& v : result)
                v -= right;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount>& operator-=(std::array<basic_vector<T, D, Type>, VertexCount>& left, const basic_vector<T, D, Type>& right)
        {
            for (auto& v : left)
                v -= right;
            return left;
        }

        template <typename T, std::uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator+(const basic_vector<T, D, Type>& left, const T& right)
        {
            basic_vector<T, D, Type> result = left;
            for (std::uint32_t i = 0; i < D; ++i)
                result[i] += right;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator+(const T& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result = right;
            for (std::uint32_t i = 0; i < D; ++i)
                result[i] += left;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator-(const basic_vector<T, D, Type>& left, const T& right)
        {
            basic_vector<T, D, Type> result = left;
            for (std::uint32_t i = 0; i < D; ++i)
                result[i] -= right;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator-(const T& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result;
            for (std::uint32_t i = 0; i < D; ++i)
                result[i] = left - right[i];
            return result;
        }

        template <typename T, std::uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator*(const basic_vector<T, D, Type>& left, const T& right)
        {
            basic_vector<T, D, Type> result = left;
            for (std::uint32_t i = 0; i < D; ++i)
                result[i] *= right;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator*(const T& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result = right;
            for (std::uint32_t i = 0; i < D; ++i)
                result[i] *= left;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator/(const basic_vector<T, D, Type>& left, const T& right)
        {
            basic_vector<T, D, Type> result = left;
            for (std::uint32_t i = 0; i < D; ++i)
                result[i] /= right;
            return result;
        }

        template <typename T, std::uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator/(const T& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result;
            for (std::uint32_t i = 0; i < D; ++i)
                result[i] = left / right[i];
            return result;
        }

        template <typename T, std::uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator%(const basic_vector<T, D, Type>& left, const T& right)
        {
            basic_vector<T, D, Type> result;
            for (std::uint32_t i = 0; i < D; ++i)
                result[i] = std::fmod(left[i], right);
            return result;
        }

        template <typename T, std::uint32_t D>
        inline T operator*(const basic_vector<T, D, row_vector>& left, const basic_vector<T, D, column_vector>& right)
        {
            T result = {};
            for (std::uint32_t index = 0; index < D; ++index)
                result += (left[index] * right[index]);
            return result;
        }

        template <typename T, typename Type>
        inline basic_vector<T, 3, Type> operator+(const basic_vector<T, 3, Type>& left, const basic_vector<T, 3, Type>& right)
        {
            return basic_vector<T, 3, Type>{ left[0] + right[0], left[1] + right[1], left[2] + right[2] };
        }

        template <typename T, typename Type>
        inline basic_vector<T, 3, Type> operator-(const basic_vector<T, 3, Type>& left, const basic_vector<T, 3, Type>& right)
        {
            return basic_vector<T, 3, Type>{ left[0] - right[0], left[1] - right[1], left[2] - right[2] };
        }

        template <typename T, typename Type>
        inline basic_vector<T, 3, Type> operator+(const basic_vector<T, 3, Type>& left, const T& right)
        {
            return basic_vector<T, 3, Type>{ left[0] + right, left[1] + right, left[2] + right };
        }

        template <typename T, typename Type>
        inline basic_vector<T, 3, Type> operator+(const T& left, const basic_vector<T, 3, Type>& right)
        {
            return basic_vector<T, 3, Type>{ left + right[0], left + right[1], left + right[2] };
        }

        template <typename T, typename Type>
        inline basic_vector<T, 3, Type> operator-(const basic_vector<T, 3, Type>& left, const T& right)
        {
            return basic_vector<T, 3, Type>{ left[0] - right, left[1] - right, left[2] - right };
        }

        template <typename T, typename Type>
        inline basic_vector<T, 3, Type> operator-(const T& left, const basic_vector<T, 3, Type>& right)
        {
            return basic_vector<T, 3, Type>{ left - right[0], left - right[1], left - right[2] };
        }

        template <typename T, typename Type>
        inline basic_vector<T, 3, Type> operator*(const basic_vector<T, 3, Type>& left, const T& right)
        {
            return basic_vector<T, 3, Type>{ left[0] * right, left[1] * right, left[2] * right };
        }

        template <typename T, typename Type>
        inline basic_vector<T, 3, Type> operator*(const T& left, const basic_vector<T, 3, Type>& right)
        {
            return basic_vector<T, 3, Type>{ left * right[0], left * right[1], left * right[2] };
        }

        template <typename T, typename Type>
        inline basic_vector<T, 3, Type> operator/(const basic_vector<T, 3, Type>& left, const T& right)
        {
            return basic_vector<T, 3, Type>{ left[0] / right, left[1] / right, left[2] / right };
        }

        template <typename T, typename Type>
        inline basic_vector<T, 3, Type> operator%(const basic_vector<T, 3, Type>& left, const T& right)
        {
            return basic_vector<T, 3, Type>{ std::fmod(left[0], right), std::fmod(left[1], right), std::fmod(left[2], right) };
        }

        template <typename T>
        inline T operator*(const basic_vector<T, 3, row_vector>& left, const basic_vector<T, 3, column_vector>& right)
        {
            return left[0] * right[0] + left[1] * right[1] + left[2] * right[2];
        }

        template <typename T, typename Type>
        inline basic_vector<T, 3, Type> midpoint(const basic_vector<T, 3, Type>& left, const basic_vector<T, 3, Type>& right)
        {
            return (left + right) / constants::two<T>;
        }

        template <typename T, std::uint32_t Size, typename Type>
        inline basic_vector<T, Size, Type> lerp(const basic_vector<T, Size, Type>& aV1, const basic_vector<T, Size, Type>& aV2, double aAmount)
        {
            basic_vector<T, Size, Type> result;
            for (std::uint32_t i = 0; i < Size; ++i)
            {
                double x1 = aV1[i];
                double x2 = aV2[i];
                result[i] = static_cast<T>((x2 - x1) * aAmount + x1);
            }
            return result;
        }

        /* todo: specializations that use SIMD intrinsics. */
        template <typename T, std::uint32_t Rows, std::uint32_t Columns>
        class basic_matrix
        {
        public:
            using abstract_type = basic_matrix; // todo: abstract base
        public:
            using value_type    = T;
            using row_type      = basic_vector<T, Columns, row_vector>;
            using column_type   = basic_vector<T, Rows, column_vector>;
            using array_type    = std::array<column_type, Columns>;
        public:
            template <typename T2>
            struct rebind { using type = basic_matrix<T2, Rows, Columns>; };
        public:
            basic_matrix() : m{ {} } {}
            basic_matrix(std::initializer_list<std::initializer_list<value_type>> aColumns) { std::copy(aColumns.begin(), aColumns.end(), m.begin()); }
            basic_matrix(const basic_matrix& other) : m{ other.m }, isIdentity{ other.isIdentity } {}
            basic_matrix(basic_matrix&& other) : m{ std::move(other.m) }, isIdentity{ other.isIdentity } {}
            template <typename T2>
            basic_matrix(const basic_matrix<T2, Rows, Columns>& other)
            {
                for (std::uint32_t column = 0; column < Columns; ++column)
                    for (std::uint32_t row = 0; row < Rows; ++row)
                        (*this)[column][row] = static_cast<value_type>(other[column][row]);
                isIdentity = other.maybe_identity();
            }
            basic_matrix& operator=(const basic_matrix& other) { m = other.m; isIdentity = other.isIdentity; return *this; }
            basic_matrix& operator=(basic_matrix&& other) { m = std::move(other.m); isIdentity = other.isIdentity; return *this; }
        public:
            template <typename T2>
            basic_matrix<T2, Rows, Columns> as() const
            {
                return basic_matrix<T2, Rows, Columns>{ *this };
            }
        public:
            std::pair<std::uint32_t, std::uint32_t> size() const { return std::make_pair(Rows, Columns); }
            const column_type& operator[](std::uint32_t aColumn) const { return m[aColumn]; }
            column_type& operator[](std::uint32_t aColumn) { isIdentity = std::nullopt; return m[aColumn]; }
            const value_type* data() const { return &m[0].v[0]; }
        public:
            bool operator==(const basic_matrix& right) const { return m == right.m; }
            bool operator!=(const basic_matrix& right) const { return m != right.m; }
            basic_matrix& operator+=(const basic_matrix& right) { for (std::uint32_t column = 0; column < Columns; ++column) (*this)[column] += right[column]; return *this; }
            basic_matrix& operator-=(const basic_matrix& right) { for (std::uint32_t column = 0; column < Columns; ++column) (*this)[column] -= right[column]; return *this; }
            basic_matrix& operator*=(const basic_matrix& right)
            {
                basic_matrix result;
                for (std::uint32_t column = 0; column < Columns; ++column)
                    for (std::uint32_t row = 0; row < Rows; ++row)
                        for (std::uint32_t index = 0; index < Columns; ++index)
                            result[column][row] += ((*this)[index][row] * right[column][index]);
                *this = result;
                return *this;
            }
            basic_matrix operator-() const
            {
                basic_matrix result = *this;
                for (std::uint32_t column = 0; column < Columns; ++column)
                    for (std::uint32_t row = 0; row < Rows; ++row)
                        result[column][row] = -result[column][row];
                return result;
            }
            basic_matrix round_to(value_type aEpsilon) const
            {
                basic_matrix result;
                for (std::uint32_t column = 0; column < Columns; ++column)
                    for (std::uint32_t row = 0; row < Rows; ++row)
                    {
                         std::modf((*this)[column][row] / aEpsilon + 0.5, &result[column][row]);
                         result[column][row] *= aEpsilon;
                    }
                return result;
            }
            basic_matrix<T, Columns, Rows> transposed() const
            {
                basic_matrix<T, Columns, Rows> result;
                for (std::uint32_t column = 0; column < Columns; ++column)
                    for (std::uint32_t row = 0; row < Rows; ++row)
                        result[row][column] = (*this)[column][row];
                return result;
            }
            template <typename SFINAE = basic_matrix>
            static const std::enable_if_t<Rows == Columns, SFINAE>& identity()
            {
                auto make_identity = []()
                {
                    basic_matrix result;
                    for (std::uint32_t diag = 0; diag < Rows; ++diag)
                        result[diag][diag] = static_cast<value_type>(1.0);
                    result.isIdentity = true;
                    return result;
                };
                static basic_matrix const sIdentity = make_identity();
                return sIdentity;
            }
            bool is_identity() const
            {
                if (isIdentity)
                    return *isIdentity;
                else
                    return *(isIdentity = ((*this) == identity()));
            }
            const std::optional<bool>& maybe_identity() const
            {
                return isIdentity;
            }
        public:
            friend void swap(basic_matrix& a, basic_matrix& b)
            {
                using std::swap;
                swap(a.m, b.m);
                swap(a.isIdentity, b.isIdentity);
            }
        private:
            array_type m;
            mutable std::optional<bool> isIdentity;
        };

        using matrix11 = basic_matrix<double, 1, 1>;
        using matrix22 = basic_matrix<double, 2, 2>;
        using matrix21 = basic_matrix<double, 2, 1>;
        using matrix12 = basic_matrix<double, 1, 2>;
        using matrix33 = basic_matrix<double, 3, 3>;
        using matrix31 = basic_matrix<double, 3, 1>;
        using matrix32 = basic_matrix<double, 3, 2>;
        using matrix13 = basic_matrix<double, 1, 3>;
        using matrix23 = basic_matrix<double, 2, 3>;
        using matrix44 = basic_matrix<double, 4, 4>;
        using matrix41 = basic_matrix<double, 4, 1>;
        using matrix42 = basic_matrix<double, 4, 2>;
        using matrix43 = basic_matrix<double, 4, 3>;
        using matrix14 = basic_matrix<double, 1, 4>;
        using matrix24 = basic_matrix<double, 2, 4>;
        using matrix34 = basic_matrix<double, 3, 4>;

        using matrix1 = matrix11;
        using matrix2 = matrix22;
        using matrix3 = matrix33;
        using matrix4 = matrix44;

        using mat11 = matrix11;
        using mat22 = matrix22;
        using mat21 = matrix21;
        using mat12 = matrix12;
        using mat33 = matrix33;
        using mat31 = matrix31;
        using mat32 = matrix32;
        using mat13 = matrix13;
        using mat23 = matrix23;
        using mat44 = matrix44;
        using mat41 = matrix41;
        using mat42 = matrix42;
        using mat43 = matrix43;
        using mat14 = matrix14;
        using mat24 = matrix24;
        using mat34 = matrix34;

        using mat1 = mat11;
        using mat2 = mat22;
        using mat3 = mat33;
        using mat4 = mat44;

        using optional_matrix11 = optional<matrix11>;
        using optional_matrix22 = optional<matrix22>;
        using optional_matrix21 = optional<matrix21>;
        using optional_matrix12 = optional<matrix12>;
        using optional_matrix33 = optional<matrix33>;
        using optional_matrix31 = optional<matrix31>;
        using optional_matrix32 = optional<matrix32>;
        using optional_matrix13 = optional<matrix13>;
        using optional_matrix23 = optional<matrix23>;
        using optional_matrix44 = optional<matrix44>;
        using optional_matrix41 = optional<matrix41>;
        using optional_matrix42 = optional<matrix42>;
        using optional_matrix43 = optional<matrix43>;
        using optional_matrix14 = optional<matrix14>;
        using optional_matrix24 = optional<matrix24>;
        using optional_matrix34 = optional<matrix34>;

        using optional_matrix1 = optional<matrix11>;
        using optional_matrix2 = optional<matrix22>;
        using optional_matrix3 = optional<matrix33>;
        using optional_matrix4 = optional<matrix44>;

        using optional_mat11 = optional<mat11>;
        using optional_mat22 = optional<mat22>;
        using optional_mat21 = optional<mat21>;
        using optional_mat12 = optional<mat12>;
        using optional_mat33 = optional<mat33>;
        using optional_mat31 = optional<mat31>;
        using optional_mat32 = optional<mat32>;
        using optional_mat13 = optional<mat13>;
        using optional_mat23 = optional<mat23>;
        using optional_mat44 = optional<mat44>;
        using optional_mat41 = optional<mat41>;
        using optional_mat42 = optional<mat42>;
        using optional_mat43 = optional<mat43>;
        using optional_mat14 = optional<mat14>;
        using optional_mat24 = optional<mat24>;
        using optional_mat34 = optional<mat34>;

        using optional_mat1 = optional<mat11>;
        using optional_mat2 = optional<mat22>;
        using optional_mat3 = optional<mat33>;
        using optional_mat4 = optional<mat44>;

        using matrix11f = basic_matrix<float, 1, 1>;
        using matrix22f = basic_matrix<float, 2, 2>;
        using matrix21f = basic_matrix<float, 2, 1>;
        using matrix12f = basic_matrix<float, 1, 2>;
        using matrix33f = basic_matrix<float, 3, 3>;
        using matrix31f = basic_matrix<float, 3, 1>;
        using matrix32f = basic_matrix<float, 3, 2>;
        using matrix13f = basic_matrix<float, 1, 3>;
        using matrix23f = basic_matrix<float, 2, 3>;
        using matrix44f = basic_matrix<float, 4, 4>;
        using matrix41f = basic_matrix<float, 4, 1>;
        using matrix42f = basic_matrix<float, 4, 2>;
        using matrix43f = basic_matrix<float, 4, 3>;
        using matrix14f = basic_matrix<float, 1, 4>;
        using matrix24f = basic_matrix<float, 2, 4>;
        using matrix34f = basic_matrix<float, 3, 4>;

        using mat11f = matrix11f;
        using mat22f = matrix22f;
        using mat21f = matrix21f;
        using mat12f = matrix12f;
        using mat33f = matrix33f;
        using mat31f = matrix31f;
        using mat32f = matrix32f;
        using mat13f = matrix13f;
        using mat23f = matrix23f;
        using mat44f = matrix44f;
        using mat41f = matrix41f;
        using mat42f = matrix42f;
        using mat43f = matrix43f;
        using mat14f = matrix14f;
        using mat24f = matrix24f;
        using mat34f = matrix34f;

        using mat1f = matrix11f;
        using mat2f = matrix22f;
        using mat3f = matrix33f;
        using mat4f = matrix44f;

        using optional_matrix11f = optional<matrix11f>;
        using optional_matrix22f = optional<matrix22f>;
        using optional_matrix21f = optional<matrix21f>;
        using optional_matrix12f = optional<matrix12f>;
        using optional_matrix33f = optional<matrix33f>;
        using optional_matrix31f = optional<matrix31f>;
        using optional_matrix32f = optional<matrix32f>;
        using optional_matrix13f = optional<matrix13f>;
        using optional_matrix23f = optional<matrix23f>;
        using optional_matrix44f = optional<matrix44f>;
        using optional_matrix41f = optional<matrix41f>;
        using optional_matrix42f = optional<matrix42f>;
        using optional_matrix43f = optional<matrix43f>;
        using optional_matrix14f = optional<matrix14f>;
        using optional_matrix24f = optional<matrix24f>;
        using optional_matrix34f = optional<matrix34f>;

        using optional_matrix1f = optional<matrix11f>;
        using optional_matrix2f = optional<matrix22f>;
        using optional_matrix3f = optional<matrix33f>;
        using optional_matrix4f = optional<matrix44f>;

        using optional_mat11f = optional<mat11f>;
        using optional_mat22f = optional<mat22f>;
        using optional_mat21f = optional<mat21f>;
        using optional_mat12f = optional<mat12f>;
        using optional_mat33f = optional<mat33f>;
        using optional_mat31f = optional<mat31f>;
        using optional_mat32f = optional<mat32f>;
        using optional_mat13f = optional<mat13f>;
        using optional_mat23f = optional<mat23f>;
        using optional_mat44f = optional<mat44f>;
        using optional_mat41f = optional<mat41f>;
        using optional_mat42f = optional<mat42f>;
        using optional_mat43f = optional<mat43f>;
        using optional_mat14f = optional<mat14f>;
        using optional_mat24f = optional<mat24f>;
        using optional_mat34f = optional<mat34f>;

        using optional_mat1f = optional<mat11f>;
        using optional_mat2f = optional<mat22f>;
        using optional_mat3f = optional<mat33f>;
        using optional_mat4f = optional<mat44f>;

        template <typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator+(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result += right;
            return result;
        }

        template <typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator-(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result -= right;
            return result;
        }

        template <typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator*(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result *= right;
            return result;
        }

        template <typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator/(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result /= right;
            return result;
        }

        template <typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator+(scalar left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = right;
            result += left;
            return result;
        }

        template <typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator-(scalar left, const basic_matrix<T, Rows, Columns>& right)
        {
            return -right + left;
        }

        template <typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator*(scalar left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = right;
            result *= left;
            return result;
        }

        template <typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator+(const basic_matrix<T, Rows, Columns>& left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result += right;
            return result;
        }

        template <typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator-(const basic_matrix<T, Rows, Columns>& left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result -= right;
            return result;
        }

        template <typename T, std::uint32_t D1, std::uint32_t D2>
        inline basic_matrix<T, D1, D1> operator*(const basic_matrix<T, D1, D2>& left, const basic_matrix<T, D2, D1>& right)
        {
            if (left.is_identity())
                return right;
            if (right.is_identity())
                return left;
            basic_matrix<T, D1, D1> result;
            for (std::uint32_t column = 0u; column < D1; ++column)
                for (std::uint32_t row = 0u; row < D1; ++row)
                    for (std::uint32_t index = 0; index < D2; ++index)
                        result[column][row] += (left[index][row] * right[column][index]);
            return result;
        }

        template <typename T>
        inline basic_matrix<T, 4u, 4u> operator*(const basic_matrix<T, 4u, 4u>& left, const basic_matrix<T, 4u, 4u>& right)
        {
            if (left.is_identity())
                return right;
            if (right.is_identity())
                return left;
            basic_matrix<T, 4u, 4u> result;
            for (std::uint32_t column = 0u; column < 4u; ++column)
                for (std::uint32_t row = 0u; row < 4u; ++row)
                    result[column][row] = simd_fma_4d(left[0u][row], right[column][0u], left[1u][row], right[column][1u], left[2u][row], right[column][2u], left[3u][row], right[column][3u]);
            return result;
        }

        template <typename T, std::uint32_t D>
        inline basic_vector<T, D, column_vector> operator*(const basic_matrix<T, D, D>& left, const basic_vector<T, D, column_vector>& right)
        {
            if (left.is_identity())
                return right;
            basic_vector<T, D, column_vector> result;
            for (std::uint32_t row = 0; row < D; ++row)
                for (std::uint32_t index = 0; index < D; ++index)
                    result[row] += (left[index][row] * right[index]);
            return result;
        }

        template <typename T, std::uint32_t D, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, column_vector>, VertexCount> operator*(const basic_matrix<T, D, D>& left, const std::array<basic_vector<T, D, column_vector>, VertexCount>& right)
        {
            if (left.is_identity())
                return right;
            std::array<basic_vector<T, D, column_vector>, VertexCount> result;
            for (std::size_t vector = 0; vector < VertexCount; ++vector)
                result[vector] = left * right[vector];
            return result;
        }

        template <typename T>
        inline basic_vector<T, 4u, column_vector> operator*(const basic_matrix<T, 4u, 4u>& left, const basic_vector<T, 4u, column_vector>& right)
        {
            if (left.is_identity())
                return right;
            basic_vector<T, 4u, column_vector> result;
            for (std::uint32_t row = 0u; row < 4u; ++row)
                result[row] = simd_fma_4d(left[0][row], right[0], left[1][row], right[1], left[2][row], right[2], left[3][row], right[3]);
            return result;
        }

        template <typename T, std::size_t VertexCount>
        inline std::array<basic_vector<T, 4u, column_vector>, VertexCount> operator*(const basic_matrix<T, 4u, 4u>& left, const std::array<basic_vector<T, 4u, column_vector>, VertexCount>& right)
        {
            if (left.is_identity())
                return right;
            std::array<basic_vector<T, 4u, column_vector>, VertexCount> result;
            for (std::size_t vector = 0; vector < VertexCount; ++vector)
                result[vector] = left * right[vector];
            return result;
        }

        template <typename T, std::uint32_t D>
        inline basic_vector<T, D, row_vector> operator*(const basic_vector<T, D, row_vector>& left, const basic_matrix<T, D, D>& right)
        {
            if (right.is_identity())
                return left;
            basic_vector<T, D, row_vector> result;
            for (std::uint32_t column = 0; column < D; ++column)
                for (std::uint32_t index = 0; index < D; ++index)
                    result[column] += (left[index] * right[column][index]);
            return result;
        }

        template <typename T, std::uint32_t D, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, row_vector>, VertexCount> operator*(const std::array<basic_vector<T, D, row_vector>, VertexCount>& left, const basic_matrix<T, D, D>& right)
        {
            if (right.is_identity())
                return left;
            std::array<basic_vector<T, D, row_vector>, VertexCount> result;
            for (std::size_t vector = 0; vector < VertexCount; ++vector)
                result[vector] = left[vector] * right;
            return result;
        }

        template <typename T>
        inline basic_vector<T, 4u, row_vector> operator*(const basic_vector<T, 4u, row_vector>& left, const basic_matrix<T, 4u, 4u>& right)
        {
            if (right.is_identity())
                return left;
            basic_vector<T, 4u, row_vector> result;
            for (std::uint32_t column = 0u; column < 4u; ++column)
                result[column] = simd_fma_4d(left[0], right[column][0], left[1], right[column][1], left[2], right[column][2], left[3], right[column][3]);
            return result;
        }

        template <typename T, std::size_t VertexCount>
        inline basic_vector<T, 4u, row_vector> operator*(const std::array<basic_vector<T, 4u, row_vector>, VertexCount>& left, const basic_matrix<T, 4u, 4u>& right)
        {
            if (right.is_identity())
                return left;
            std::array<basic_vector<T, 4u, row_vector>, VertexCount> result;
            for (std::size_t vector = 0; vector < VertexCount; ++vector)
                result[vector] = left[vector] * right;
            return result;
        }

        template <typename T, std::uint32_t D>
        inline basic_matrix<T, D, D> operator*(const basic_vector<T, D, column_vector>& left, const basic_vector<T, D, row_vector>& right)
        {
            basic_matrix<T, D, D> result;
            for (std::uint32_t column = 0; column < D; ++column)
                for (std::uint32_t row = 0; row < D; ++row)
                    result[column][row] = (left[row] * right[column]);
            return result;
        }

        template <typename T>
        inline basic_matrix<T, 4u, 4u> operator*(const basic_vector<T, 4u, column_vector>& left, const basic_vector<T, 4u, row_vector>& right)
        {
            basic_matrix<T, 4u, 4u> result;
            for (std::uint32_t column = 0; column < 4u; ++column)
                simd_mul_4d(left[0u], right[column], left[1u], right[column], left[2u], right[column], left[3u], right[column], result[column][0u], result[column][1u], result[column][2u], result[column][3u]);
            return result;
        }

        template <typename T, std::uint32_t D>
        inline basic_matrix<T, D, D> without_translation(const basic_matrix<T, D, D>& matrix)
        {
            auto result = matrix;
            for (std::uint32_t row = 0; row < D - 1; ++row)
                result[D - 1][row] = 0.0;
            return result;
        }

        template <typename Elem, typename Traits, typename T, std::uint32_t Size, typename Type>
        inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_vector<T, Size, Type>& aVector)
        {
            aStream << "[";
            for (std::uint32_t i = 0; i < Size; ++i)
            {
                if (i != 0)
                    aStream << ", ";
                aStream << aVector[i];
            }
            aStream << "]";
            return aStream;
        }

        template <typename Elem, typename Traits, typename T, std::uint32_t Size, typename Type>
        inline std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& aStream, basic_vector<T, Size, Type>& aVector)
        {
            auto previousImbued = aStream.getloc();
            if (typeid(std::use_facet<std::ctype<char>>(previousImbued)) != typeid(neolib::comma_and_brackets_as_whitespace))
                aStream.imbue(std::locale{ previousImbued, new neolib::comma_and_brackets_as_whitespace{} });
            for (std::uint32_t i = 0; i < Size; ++i)
                aStream >> aVector[i];
            aStream.imbue(previousImbued);
            return aStream;
        }

        template <typename Elem, typename Traits, typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_matrix<T, Rows, Columns>& aMatrix)
        {
            aStream << "[";
            for (std::uint32_t row = 0; row < Rows; ++row)
            {
                if (row != 0)
                    aStream << ", ";
                aStream << "[";
                for (std::uint32_t column = 0; column < Columns; ++column)
                {
                    if (column != 0)
                        aStream << ", ";
                    aStream << aMatrix[column][row];
                }
                aStream << "]";
            }
            aStream << "]";
            return aStream;
        }

        template <typename Elem, typename Traits, typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const optional<basic_matrix<T, Rows, Columns>>& aMatrix)
        {
            if (aMatrix != std::nullopt)
                aStream << *aMatrix;
            else
                aStream << "[null]";
            return aStream;
        }

        template <typename Elem, typename Traits, typename T, std::uint32_t Rows, std::uint32_t Columns>
        inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const std::optional<basic_matrix<T, Rows, Columns>>& aMatrix)
        {
            if (aMatrix != std::nullopt)
                aStream << *aMatrix;
            else
                aStream << "[null]";
            return aStream;
        }

        // 3D helpers

        template <typename T>
        inline basic_vector<T, 3u, column_vector> operator*(const basic_matrix<T, 4u, 4u>& left, const basic_vector<T, 3u, column_vector>& right)
        {
            if (left.is_identity())
                return right;
            basic_vector<T, 3u, column_vector> result;
            for (std::uint32_t row = 0u; row < 3u; ++row)
                result[row] = static_cast<T>(simd_fma_4d(left[0][row], right[0], left[1][row], right[1], left[2][row], right[2], left[3][row], 1.0));
            return result;
        }

        template <typename T>
        inline std::vector<basic_vector<T, 3u, column_vector>> operator*(const basic_matrix<T, 4u, 4u>& left, const std::vector<basic_vector<T, 3u, column_vector>>& right)
        {
            if (left.is_identity())
                return right;
            std::vector<basic_vector<T, 3u, column_vector>> result;
            result.reserve(right.size());
            for (auto const& v : right)
                result.push_back(left * v);
            return result;
        }

        inline mat33 rotation_matrix(const vec3& axis, scalar angle, scalar epsilon = 0.00001)
        {
            if (std::abs(angle) <= epsilon)
                return mat33::identity();
            else if (std::abs(angle - boost::math::constants::pi<scalar>()) <= epsilon)
                return -mat33::identity();
            scalar const s = std::sin(angle);
            scalar const c = std::cos(angle);
            scalar const a = 1.0 - c;
            scalar const ax = a * axis.x;
            scalar const ay = a * axis.y;
            scalar const az = a * axis.z;
            return mat33{
                { ax * axis.x + c, ax * axis.y + axis.z * s, ax * axis.z - axis.y * s },
                { ay * axis.x - axis.z * s, ay * axis.y + c, ay * axis.z + axis.x * s },
                { az * axis.x + axis.y * s, az * axis.y - axis.x * s, az * axis.z + c } }.round_to(epsilon);
        }

        inline mat33 rotation_matrix(const vec3& vectorA, const vec3& vectorB, scalar epsilon = 0.00001)
        {
            auto const nva = vectorA.normalized();
            auto const nvb = vectorB.normalized();
            return rotation_matrix(nva.cross(nvb).normalized(), std::acos(nva.dot(nvb)), epsilon);
        }

        inline mat33 rotation_matrix(const vec3& angles)
        {
            scalar ax = angles.x;
            scalar ay = angles.y;
            scalar az = angles.z;
            if (ax != 0.0 || ay != 0.0)
            {
                mat33 rx = { { 1.0, 0.0, 0.0 },{ 0.0, std::cos(ax), std::sin(ax) },{ 0.0, -std::sin(ax), std::cos(ax) } };
                mat33 ry = { { std::cos(ay), 0.0, -std::sin(ay) },{ 0.0, 1.0, 0.0 },{ std::sin(ay), 0.0, std::cos(ay) } };
                mat33 rz = { { std::cos(az), std::sin(az), 0.0 },{ -std::sin(az), std::cos(az), 0.0 },{ 0.0, 0.0, 1.0 } };
                return rz * ry * rx;
            }
            else
            {
                return mat33{ { std::cos(az), std::sin(az), 0.0 },{ -std::sin(az), std::cos(az), 0.0 },{ 0.0, 0.0, 1.0 } };
            }
        }

        inline mat44 affine_rotation_matrix(const vec3& angles)
        {
            scalar ax = angles.x;
            scalar ay = angles.y;
            scalar az = angles.z;
            if (ax != 0.0 || ay != 0.0)
            {
                mat44 rx = { { 1.0, 0.0, 0.0, 0.0 },{ 0.0, std::cos(ax), std::sin(ax), 0.0 },{ 0.0, -std::sin(ax), std::cos(ax), 0.0 },{0.0, 0.0, 0.0, 1.0} };
                mat44 ry = { { std::cos(ay), 0.0, -std::sin(ay), 0.0 },{ 0.0, 1.0, 0.0, 0.0 },{ std::sin(ay), 0.0, std::cos(ay), 0.0 },{0.0, 0.0, 0.0, 1.0} };
                mat44 rz = { { std::cos(az), std::sin(az), 0.0, 0.0 },{ -std::sin(az), std::cos(az), 0.0, 0.0 },{ 0.0, 0.0, 1.0, 0.0 },{0.0, 0.0, 0.0, 1.0} };
                return rz * ry * rx;
            }
            else
            {
                return mat44{ { std::cos(az), std::sin(az), 0.0, 0.0 },{ -std::sin(az), std::cos(az), 0.0, 0.0 },{ 0.0, 0.0, 1.0, 0.0 },{0.0, 0.0, 0.0, 1.0} };
            }
        }

        inline mat44& apply_translation(mat44& aMatrix, const vec3& aTranslation)
        {
            // todo: SIMD
            aMatrix[3][0] += aTranslation.x;
            aMatrix[3][1] += aTranslation.y;
            aMatrix[3][2] += aTranslation.z;
            return aMatrix;
        }

        inline mat44& apply_scaling(mat44& aMatrix, const vec3& aScaling)
        {
            // todo: SIMD
            aMatrix[0][0] *= aScaling.x;
            aMatrix[1][1] *= aScaling.y;
            aMatrix[2][2] *= aScaling.z;
            return aMatrix;
        }

        // Function

        template <typename T>
        inline bool nearly_equal(T lhs, T rhs, scalar epsilon = 0.00001, std::enable_if_t<std::is_floating_point_v<T>, sfinae> = {})
        {
            return static_cast<double>(std::abs(lhs - rhs)) < epsilon;
        }
        template <typename T>
        inline bool nearly_equal(T lhs, T rhs, scalar epsilon = 0.00001, std::enable_if_t<std::is_integral_v<T>, sfinae> = {})
        {
            return lhs == rhs;
        }
        template <typename T, std::uint32_t Size, typename Type = column_vector>
        inline bool nearly_equal(basic_vector<T, Size, Type> const& lhs, basic_vector<T, Size, Type> const& rhs, scalar epsilon = 0.00001)
        {
            for (std::uint32_t index = 0; index < Size; ++index)
                if (!nearly_equal(lhs[index], rhs[index], epsilon))
                    return false;
            return true;
        }
        template <typename T>
        inline bool nearly_equal(optional<T> const& lhs, optional<T> const& rhs, scalar epsilon = 0.00001)
        {
            if (!!lhs != !!rhs)
                return false;
            if (!lhs)
                return true;
            return nearly_equal(*lhs, *rhs, epsilon);
        }
        template <typename T>
        inline bool nearly_equal(std::optional<T> const& lhs, std::optional<T> const& rhs, scalar epsilon = 0.00001)
        {
            if (!!lhs != !!rhs)
                return false;
            if (!lhs)
                return true;
            return nearly_equal(*lhs, *rhs, epsilon);
        }
        template <typename T1, typename T2>
        inline bool nearly_equal(std::pair<T1, T2> const& lhs, std::pair<T1, T2> const& rhs, scalar epsilon = 0.00001)
        {
            return nearly_equal(lhs.first, rhs.first, epsilon) && nearly_equal(lhs.second, rhs.second, epsilon);
        }
        template <typename T>
        inline bool nearly_equal(std::vector<T> const& lhs, std::vector<T> const& rhs, scalar epsilon = 0.00001)
        {
            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                [epsilon](auto const& lhs, auto const& rhs) { return nearly_equal(lhs, rhs, epsilon); });
        }

        template <typename T, std::size_t D>
        inline basic_vector<T, D> quad_extents(std::array<basic_vector<T, D>, 4> const& aQuad)
        {
            return basic_vector<T, D>{ 
                (aQuad[1].distance(aQuad[0]) + aQuad[3].distance(aQuad[2])) / static_cast<T>(2.0),
                (aQuad[0].distance(aQuad[3]) + aQuad[1].distance(aQuad[2])) / static_cast<T>(2.0)
            };
        }

        // AABB

        struct aabb
        {
            using abstract_type = aabb;

            vec3 min;
            vec3 max;
            aabb() : min{}, max{} {}
            aabb(const vec3& aMin, const vec3& aMax) : min{ aMin }, max{ aMax } {}
        };

        inline vec3 aabb_origin(const aabb& aAabb)
        {
            return aAabb.min + (aAabb.max - aAabb.min) / 2.0;
        }

        inline vec3 aabb_extents(const aabb& aAabb)
        {
            return aAabb.max - aAabb.min;
        }

        template <typename... Transforms>
        inline aabb aabb_transform(const aabb& aAabb, const Transforms&... aTransforms)
        {
            std::array<vec3, 8> boxVertices =
            {
                (aTransforms * ... * vec3{ aAabb.min.x, aAabb.min.y, aAabb.min.z }),
                (aTransforms * ... * vec3{ aAabb.max.x, aAabb.min.y, aAabb.min.z }),
                (aTransforms * ... * vec3{ aAabb.min.x, aAabb.max.y, aAabb.min.z }),
                (aTransforms * ... * vec3{ aAabb.max.x, aAabb.max.y, aAabb.min.z }),
                (aTransforms * ... * vec3{ aAabb.min.x, aAabb.min.y, aAabb.max.z }),
                (aTransforms * ... * vec3{ aAabb.max.x, aAabb.min.y, aAabb.max.z }),
                (aTransforms * ... * vec3{ aAabb.min.x, aAabb.max.y, aAabb.max.z }),
                (aTransforms * ... * vec3{ aAabb.max.x, aAabb.max.y, aAabb.max.z })
            };
            aabb result{ boxVertices[0], boxVertices[0] };
            for (auto const& v : boxVertices)
            {
                result.min = result.min.min(v);
                result.max = result.max.max(v);
            }
            return result;
        }

        inline aabb to_aabb(const vec3& aOrigin, scalar aSize)
        {
            return aabb{ aOrigin - aSize / 2.0, aOrigin + aSize / 2.0 };
        }

        inline aabb to_aabb(const vec3& aOrigin, const vec3& aSize)
        {
            return aabb{ aOrigin - aSize / 2.0, aOrigin + aSize / 2.0 };
        }

        template <typename VertexIter>
        inline aabb to_aabb(VertexIter aBegin, VertexIter aEnd, const mat44& aTransformation = mat44::identity())
        {
            if (aBegin == aEnd)
                return aabb_transform(aabb{}, aTransformation);
            aabb result{ aBegin->xyz, aBegin->xyz };
            for (auto i = std::next(aBegin); i != aEnd; ++i)
            {
                result.min = result.min.min(i->xyz);
                result.max = result.max.max(i->xyz);
            }
            return aabb_transform(result, aTransformation);
        }

        inline aabb to_aabb(const vertices& aVertices, const mat44& aTransformation = mat44::identity())
        {
            return to_aabb(aVertices.begin(), aVertices.end(), aTransformation);
        }

        inline bool operator==(const aabb& left, const aabb& right)
        {
            return left.min == right.min && left.max == right.max;
        }

        inline bool operator<(const aabb& left, const aabb& right)
        {
            return std::forward_as_tuple(left.min.z, left.min.y, left.min.x, left.max.z, left.max.y, left.max.x) <
                std::forward_as_tuple(right.min.z, right.min.y, right.min.x, right.max.z, right.max.y, right.max.x);
        }

        inline std::partial_ordering operator<=>(const aabb& left, const aabb& right)
        {
            return std::forward_as_tuple(left.min.z, left.min.y, left.min.x, left.max.z, left.max.y, left.max.x) <=>
                std::forward_as_tuple(right.min.z, right.min.y, right.min.x, right.max.z, right.max.y, right.max.x);
        }

        using optional_aabb = optional<aabb>;

        inline aabb aabb_union(const aabb& left, const aabb& right)
        {
            return aabb{ left.min.min(right.min), left.max.max(right.max) };
        }

        inline scalar aabb_volume(const aabb& a)
        {
            auto extents = a.max - a.min;
            return extents.x * extents.y * (extents.z != 0.0 ? extents.z : 1.0);
        }

        inline bool aabb_contains(const aabb& outer, const aabb& inner)
        {
            return inner.min >= outer.min && inner.max <= outer.max;
        }

        inline bool aabb_contains(const aabb& outer, const vec3& point)
        {
            return point >= outer.min && point <= outer.max;
        }

        inline bool aabb_intersects(const aabb& first, const aabb& second)
        {
            if (first.max.x < second.min.x)
                return false;
            if (first.min.x > second.max.x)
                return false;
            if (first.max.y < second.min.y)
                return false;
            if (first.min.y > second.max.y)
                return false;
            if (first.max.z < second.min.z)
                return false;
            if (first.min.z > second.max.z)
                return false;
            return true;
        }

        inline bool aabb_intersects(const optional<aabb>& first, const std::optional<aabb>& second)
        {
            if (first == std::nullopt || second == std::nullopt)
                return false;
            return aabb_intersects(*first, *second);
        }
            
        inline bool aabb_intersects(const optional<aabb>& first, const aabb& second)
        {
            if (first == std::nullopt)
                return false;
            return aabb_intersects(*first, second);
        }

        inline bool aabb_intersects(const aabb& first, const std::optional<aabb>& second)
        {
            if (second == std::nullopt)
                return false;
            return aabb_intersects(first, *second);
        }

        struct aabb_2d
        {
            using abstract_type = aabb_2d;

            vec2 min;
            vec2 max;
            aabb_2d() : min{}, max{} {}
            aabb_2d(const vec2& aMin, const vec2& aMax) : min{ aMin }, max{ aMax } {}
            aabb_2d(const aabb& aAabb) : min{ aAabb.min.xy }, max{ aAabb.max.xy } {}
        };

        inline vec2 aabb_origin(const aabb_2d& aAabb)
        {
            return aAabb.min + (aAabb.max - aAabb.min) / 2.0;
        }

        inline vec2 aabb_extents(const aabb_2d& aAabb)
        {
            return aAabb.max - aAabb.min;
        }

        template <typename... Transforms>
        inline aabb_2d aabb_transform(const aabb_2d& aAabb, const Transforms&... aTransforms)
        {
            std::array<vec3, 4> boxVertices =
            {
                (aTransforms * ... * vec3{ aAabb.min.x, aAabb.min.y, 0.0 }),
                (aTransforms * ... * vec3{ aAabb.max.x, aAabb.min.y, 0.0 }),
                (aTransforms * ... * vec3{ aAabb.min.x, aAabb.max.y, 0.0 }),
                (aTransforms * ... * vec3{ aAabb.max.x, aAabb.max.y, 0.0 })
            };
            aabb_2d result{ boxVertices[0].xy, boxVertices[0].xy };
            for (auto const& v : boxVertices)
            {
                result.min = result.min.min(v.xy);
                result.max = result.max.max(v.xy);
            }
            return result;
        }

        inline aabb_2d to_aabb_2d(const vec3& aOrigin, scalar aSize)
        {
            return aabb_2d{ (aOrigin - aSize / 2.0).xy, (aOrigin + aSize / 2.0).xy };
        }

        inline aabb_2d to_aabb_2d(const vec3& aOrigin, const vec3& aSize)
        {
            return aabb_2d{ (aOrigin - aSize / 2.0).xy, (aOrigin + aSize / 2.0).xy };
        }

        template <typename VertexIter>
        inline aabb_2d to_aabb_2d(VertexIter aBegin, VertexIter aEnd, const mat44& aTransformation = mat44::identity())
        {
            if (aBegin == aEnd)
                return aabb_transform(aabb_2d{}, aTransformation);
            aabb_2d result{ aBegin->xy, aBegin->xy };
            for (auto i = std::next(aBegin); i != aEnd; ++i)
            {
                result.min = result.min.min(i->xy);
                result.max = result.max.max(i->xy);
            }
            return aabb_transform(result, aTransformation);
        }

        inline aabb_2d to_aabb_2d(const vertices& aVertices, const mat44& aTransformation = mat44::identity())
        {
            return to_aabb_2d(aVertices.begin(), aVertices.end(), aTransformation);
        }

        inline bool operator==(const aabb_2d& left, const aabb_2d& right)
        {
            return left.min == right.min && left.max == right.max;
        }

        inline bool operator<(const aabb_2d& left, const aabb_2d& right)
        {
            return std::forward_as_tuple(left.min.y, left.min.x, left.max.y, left.max.x) <
                std::forward_as_tuple(right.min.y, right.min.x, right.max.y, right.max.x);
        }

        inline std::partial_ordering operator<=>(const aabb_2d& left, const aabb_2d& right)
        {
            return std::forward_as_tuple(left.min.y, left.min.x, left.max.y, left.max.x) <=>
                std::forward_as_tuple(right.min.y, right.min.x, right.max.y, right.max.x);
        }

        using optional_aabb_2d = optional<aabb_2d>;

        inline aabb_2d aabb_union(const aabb_2d& left, const aabb_2d& right)
        {
            return aabb_2d{ left.min.min(right.min), left.max.max(right.max) };
        }

        inline scalar aabb_volume(const aabb_2d& a)
        {
            auto extents = a.max - a.min;
            return extents.x * extents.y;
        }

        inline bool aabb_contains(const aabb_2d& outer, const aabb_2d& inner)
        {
            return inner.min >= outer.min && inner.max <= outer.max;
        }

        inline bool aabb_contains(const aabb_2d& outer, const vec2& point)
        {
            return point >= outer.min && point <= outer.max;
        }

        inline bool aabb_intersects(const aabb_2d& first, const aabb_2d& second)
        {
            if (first.max.x < second.min.x)
                return false;
            if (first.min.x > second.max.x)
                return false;
            if (first.max.y < second.min.y)
                return false;
            if (first.min.y > second.max.y)
                return false;
            return true;
        }

        inline bool aabb_intersects(const optional<aabb_2d>& first, const std::optional<aabb_2d>& second)
        {
            if (first == std::nullopt || second == std::nullopt)
                return false;
            return aabb_intersects(*first, *second);
        }

        inline bool aabb_intersects(const optional<aabb_2d>& first, const aabb_2d& second)
        {
            if (first == std::nullopt)
                return false;
            return aabb_intersects(*first, second);
        }

        inline bool aabb_intersects(const aabb_2d& first, const std::optional<aabb_2d>& second)
        {
            if (second == std::nullopt)
                return false;
            return aabb_intersects(first, *second);
        }

        inline vec2 bezier_cubic(vec2 const& p0, vec2 const& p1, vec2 const& p2, vec2 const& p3, scalar t)
        {
            return std::pow(1.0 - t, 3.0) * p0 + 3.0 * std::pow(1.0 - t, 2.0) * t * p1 + 3 * (1.0 - t) * std::pow(t, 2.0) * p2 + std::pow(t, 3.0) * p3;
        }

        inline vec2 bezier_cubic_x(vec2 const& p0, vec2 const& p1, vec2 const& p2, vec2 const& p3, scalar x)
        {
            return bezier_cubic(p0, p1, p2, p3, (x - p0.x) / (p3.x - p0.x));
        }

        inline vec2 bezier_cubic_y(vec2 const& p0, vec2 const& p1, vec2 const& p2, vec2 const& p3, scalar y)
        {
            return bezier_cubic(p0, p1, p2, p3, (y - p0.y) / (p3.y - p0.y));
        }

        template <typename T>
        inline basic_vector<T, 2> bezier_cubic(basic_vector<T, 2> const& p0, basic_vector<T, 2> const& p1, basic_vector<T, 2> const& p2, basic_vector<T, 2> const& p3, T t)
        {
            return bezier_cubic(p0.template as<scalar>(), p1.template as<scalar>(), p2.template as<scalar>(), p3.template as<scalar>(), static_cast<scalar>(t)).template as<T>();
        }

        template <typename T>
        inline basic_vector<T, 2> bezier_cubic_x(basic_vector<T, 2> const& p0, basic_vector<T, 2> const& p1, basic_vector<T, 2> const& p2, basic_vector<T, 2> const& p3, T x)
        {
            return bezier_cubic_x(p0.template as<scalar>(), p1.template as<scalar>(), p2.template as<scalar>(), p3.template as<scalar>(), static_cast<scalar>(x)).template as<T>();
        }

        template <typename T>
        inline basic_vector<T, 2> bezier_cubic_y(basic_vector<T, 2> const& p0, basic_vector<T, 2> const& p1, basic_vector<T, 2> const& p2, basic_vector<T, 2> const& p3, T y)
        {
            return bezier_cubic_y(p0.template as<scalar>(), p1.template as<scalar>(), p2.template as<scalar>(), p3.template as<scalar>(), static_cast<scalar>(y)).template as<T>();
        }
    }

    using namespace math;
}
