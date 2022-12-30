// numerical.hpp
/*
 *  Copyright (c) 2015, 2020 Leigh Johnston.
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

        typedef double scalar;
        typedef double angle;

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

        template <typename T, uint32_t _Size, typename Type = column_vector>
        class basic_vector;
    }

    template <typename T, uint32_t _Size, typename Type>
    bool constexpr vecarray_trivial_v<math::basic_vector<T, _Size, Type>> = true;

    namespace math
    {
        template <typename T, uint32_t _Size, typename Type>
        class basic_vector
        {
            typedef basic_vector<T, _Size, Type> self_type;
        public:
            typedef self_type abstract_type; // todo: abstract base; std::array?
        public:
            typedef Type type;
        public:
            typedef T value_type;
            typedef basic_vector<value_type, _Size, Type> vector_type;
            typedef uint32_t size_type;
            typedef std::array<value_type, _Size> array_type;
            typedef typename array_type::const_iterator const_iterator;
            typedef typename array_type::iterator iterator;
        public:
            template <uint32_t Size2> struct rebind { typedef basic_vector<T, Size2, Type> type; };
        public:
            static constexpr uint32_t Size = _Size;
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
            template <typename V, typename A, uint32_t S, uint32_t... Indexes>
            basic_vector(const swizzle<V, A, S, Indexes...>& aSwizzle) : self_type{ ~aSwizzle } {}
            basic_vector(const self_type& other) : v{ other.v } {}
            basic_vector(self_type&& other) : v{ std::move(other.v) } {}
            template <typename T2>
            basic_vector(const basic_vector<T2, Size, Type>& other) { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            template <typename T2, uint32_t Size2, typename SFINAE = int>
            basic_vector(const basic_vector<T2, Size2, Type>& other, typename std::enable_if_t<Size2 < Size, SFINAE> = 0) : v{} { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            self_type& operator=(const self_type& other) { v = other.v; return *this; }
            self_type& operator=(self_type&& other) { v = std::move(other.v); return *this; }
            self_type& operator=(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neolib::basic_vector: initializer list too big"); std::copy(values.begin(), values.end(), v.begin()); std::fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); return *this; }
        public:
            static uint32_t size() { return Size; }
            value_type operator[](uint32_t aIndex) const { return v[aIndex]; }
            value_type& operator[](uint32_t aIndex) { return v[aIndex]; }
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
            bool operator==(const self_type& right) const { return v == right.v; }
            bool operator!=(const self_type& right) const { return v != right.v; }
            self_type& operator+=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] += value; return *this; }
            self_type& operator-=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] -= value; return *this; }
            self_type& operator*=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] *= value; return *this; }
            self_type& operator/=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] /= value; return *this; }
            self_type& operator+=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] += right.v[index]; return *this; }
            self_type& operator-=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] -= right.v[index]; return *this; }
            self_type& operator*=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] *= right.v[index]; return *this; }
            self_type& operator/=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] /= right.v[index]; return *this; }
            self_type operator-() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result.v[index] = -v[index]; return result; }
            self_type scale(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = v[index] * right[index]; return result; }
            value_type magnitude() const { value_type ss = constants::zero<value_type>; for (uint32_t index = 0; index < Size; ++index) ss += (v[index] * v[index]); return std::sqrt(ss); }
            self_type normalized() const { self_type result; value_type im = constants::one<value_type> / magnitude(); for (uint32_t index = 0; index < Size; ++index) result.v[index] = v[index] * im; return result; }
            self_type min(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::min(v[index], right.v[index]); return result; }
            self_type max(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::max(v[index], right.v[index]); return result; }
            value_type min() const { value_type result = v[0]; for (uint32_t index = 1; index < Size; ++index) result = std::min(v[index], result); return result; }
            self_type ceil() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::ceil(v[index]); return result; }
            self_type floor() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::floor(v[index]); return result; }
            self_type round() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::round(v[index]); return result; }
            value_type distance(const self_type& right) const { value_type total = 0; for (uint32_t index = 0; index < Size; ++index) total += ((v[index] - right.v[index]) * (v[index] - right.v[index])); return std::sqrt(total); }
            value_type dot(const self_type& right) const
            {
                value_type result = constants::zero<value_type>;
                for (uint32_t index = 0; index < Size; ++index)
                    result += (v[index] * right[index]);
                return result;
            }
            template <typename SFINAE = self_type>
            std::enable_if_t<Size == 3, SFINAE> cross(const self_type& right) const
            {
                return self_type{ 
                    y * right.z - z * right.y, 
                    z * right.x - x * right.z, 
                    x * right.y - y * right.x };
            }
            self_type hadamard_product(const self_type& right) const
            {
                self_type result = *this;
                result *= right;
                return result;
            }
        public:
            friend void swap(self_type& a, self_type& b)
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
            typedef basic_vector<T, 2, Type> self_type;
        public:
            typedef self_type abstract_type; // todo: abstract base; std::array?
        public:
            typedef Type type;
        public:
            typedef T value_type;
            typedef basic_vector<value_type, 2, Type> vector_type;
            typedef uint32_t size_type;
            typedef std::array<value_type, 2> array_type;
            typedef typename array_type::const_iterator const_iterator;
            typedef typename array_type::iterator iterator;
        public:
            template <uint32_t Size2> struct rebind { typedef basic_vector<T, Size2, Type> type; };
        public:
            static constexpr uint32_t Size = 2;
        public:
            basic_vector() : v{} {}
            explicit basic_vector(value_type x, value_type y) : v{ {x, y} } {}
            template <typename... Arguments>
            explicit basic_vector(const value_type& value, Arguments&&... aArguments) : v{ {value, std::forward<Arguments>(aArguments)...} } {}
            template <typename... Arguments>
            explicit basic_vector(value_type&& value, Arguments&&... aArguments) : v{ {std::move(value), std::forward<Arguments>(aArguments)...} } {}
            explicit basic_vector(const array_type& v) : v{ v } {}
            basic_vector(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neolib::basic_vector: initializer list too big"); std::uninitialized_copy(values.begin(), values.end(), v.begin()); std::uninitialized_fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); }
            template <typename V, typename A, uint32_t S, uint32_t... Indexes>
            basic_vector(const swizzle<V, A, S, Indexes...>& aSwizzle) : self_type{ ~aSwizzle } {}
            basic_vector(const self_type& other) : v{ other.v } {}
            basic_vector(self_type&& other) : v{ std::move(other.v) } {}
            template <typename T2>
            basic_vector(const basic_vector<T2, Size, Type>& other) { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            template <typename T2, uint32_t Size2, typename SFINAE = int>
            basic_vector(const basic_vector<T2, Size2, Type>& other, typename std::enable_if_t < Size2 < Size, SFINAE> = 0) : v{} { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            self_type& operator=(const self_type& other) { v = other.v; return *this; }
            self_type& operator=(self_type&& other) { v = std::move(other.v); return *this; }
            self_type& operator=(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neolib::basic_vector: initializer list too big"); std::copy(values.begin(), values.end(), v.begin()); std::fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); return *this; }
        public:
            static uint32_t size() { return Size; }
            value_type operator[](uint32_t aIndex) const { return v[aIndex]; }
            value_type& operator[](uint32_t aIndex) { return v[aIndex]; }
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
            bool operator==(const self_type& right) const { return v == right.v; }
            bool operator!=(const self_type& right) const { return v != right.v; }
            self_type& operator+=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] += value; return *this; }
            self_type& operator-=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] -= value; return *this; }
            self_type& operator*=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] *= value; return *this; }
            self_type& operator/=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] /= value; return *this; }
            self_type& operator+=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] += right.v[index]; return *this; }
            self_type& operator-=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] -= right.v[index]; return *this; }
            self_type& operator*=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] *= right.v[index]; return *this; }
            self_type& operator/=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] /= right.v[index]; return *this; }
            self_type operator-() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result.v[index] = -v[index]; return result; }
            self_type scale(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = v[index] * right[index]; return result; }
            value_type magnitude() const { value_type ss = constants::zero<value_type>; for (uint32_t index = 0; index < Size; ++index) ss += (v[index] * v[index]); return std::sqrt(ss); }
            self_type normalized() const { self_type result; value_type im = constants::one<value_type> / magnitude(); for (uint32_t index = 0; index < Size; ++index) result.v[index] = v[index] * im; return result; }
            self_type min(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::min(v[index], right.v[index]); return result; }
            self_type max(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::max(v[index], right.v[index]); return result; }
            value_type min() const { value_type result = v[0]; for (uint32_t index = 1; index < Size; ++index) result = std::min(v[index], result); return result; }
            self_type ceil() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::ceil(v[index]); return result; }
            self_type floor() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::floor(v[index]); return result; }
            self_type round() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::round(v[index]); return result; }
            value_type distance(const self_type& right) const { value_type total = 0; for (uint32_t index = 0; index < Size; ++index) total += ((v[index] - right.v[index]) * (v[index] - right.v[index])); return std::sqrt(total); }
            value_type dot(const self_type& right) const
            {
                value_type result = constants::zero<value_type>;
                for (uint32_t index = 0; index < Size; ++index)
                    result += (v[index] * right[index]);
                return result;
            }
            self_type hadamard_product(const self_type& right) const
            {
                self_type result = *this;
                result *= right;
                return result;
            }
        public:
            friend void swap(self_type& a, self_type& b)
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
            typedef basic_vector<T, 1, Type> self_type;
        public:
            typedef self_type abstract_type; // todo: abstract base; std::array?
        public:
            typedef Type type;
        public:
            typedef T value_type;
            typedef basic_vector<value_type, 1, Type> vector_type;
            typedef uint32_t size_type;
            typedef std::array<value_type, 1> array_type;
            typedef typename array_type::const_iterator const_iterator;
            typedef typename array_type::iterator iterator;
        public:
            template <uint32_t Size2> struct rebind { typedef basic_vector<T, Size2, Type> type; };
        public:
            static constexpr uint32_t Size = 1;
        public:
            basic_vector() : v{} {}
            explicit basic_vector(value_type x) : v{ {x} } {}
            template <typename... Arguments>
            explicit basic_vector(const value_type& value, Arguments&&... aArguments) : v{ {value, std::forward<Arguments>(aArguments)...} } {}
            template <typename... Arguments>
            explicit basic_vector(value_type&& value, Arguments&&... aArguments) : v{ {std::move(value), std::forward<Arguments>(aArguments)...} } {}
            explicit basic_vector(const array_type& v) : v{ v } {}
            basic_vector(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neolib::basic_vector: initializer list too big"); std::uninitialized_copy(values.begin(), values.end(), v.begin()); std::uninitialized_fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); }
            template <typename V, typename A, uint32_t S, uint32_t... Indexes>
            basic_vector(const swizzle<V, A, S, Indexes...>& aSwizzle) : self_type{ ~aSwizzle } {}
            basic_vector(const self_type& other) : v{ other.v } {}
            basic_vector(self_type&& other) : v{ std::move(other.v) } {}
            template <typename T2>
            basic_vector(const basic_vector<T2, Size, Type>& other) { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            template <typename T2, uint32_t Size2, typename SFINAE = int>
            basic_vector(const basic_vector<T2, Size2, Type>& other, typename std::enable_if_t < Size2 < Size, SFINAE> = 0) : v{} { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            self_type& operator=(const self_type& other) { v = other.v; return *this; }
            self_type& operator=(self_type&& other) { v = std::move(other.v); return *this; }
            self_type& operator=(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neolib::basic_vector: initializer list too big"); std::copy(values.begin(), values.end(), v.begin()); std::fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); return *this; }
        public:
            static uint32_t size() { return Size; }
            value_type operator[](uint32_t aIndex) const { return v[aIndex]; }
            value_type& operator[](uint32_t aIndex) { return v[aIndex]; }
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
            bool operator==(const self_type& right) const { return v == right.v; }
            bool operator!=(const self_type& right) const { return v != right.v; }
            self_type& operator+=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] += value; return *this; }
            self_type& operator-=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] -= value; return *this; }
            self_type& operator*=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] *= value; return *this; }
            self_type& operator/=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] /= value; return *this; }
            self_type& operator+=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] += right.v[index]; return *this; }
            self_type& operator-=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] -= right.v[index]; return *this; }
            self_type& operator*=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] *= right.v[index]; return *this; }
            self_type& operator/=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] /= right.v[index]; return *this; }
            self_type operator-() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result.v[index] = -v[index]; return result; }
            self_type scale(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = v[index] * right[index]; return result; }
            value_type magnitude() const { value_type ss = constants::zero<value_type>; for (uint32_t index = 0; index < Size; ++index) ss += (v[index] * v[index]); return std::sqrt(ss); }
            self_type normalized() const { self_type result; value_type im = constants::one<value_type> / magnitude(); for (uint32_t index = 0; index < Size; ++index) result.v[index] = v[index] * im; return result; }
            self_type min(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::min(v[index], right.v[index]); return result; }
            self_type max(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::max(v[index], right.v[index]); return result; }
            value_type min() const { value_type result = v[0]; for (uint32_t index = 1; index < Size; ++index) result = std::min(v[index], result); return result; }
            self_type ceil() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::ceil(v[index]); return result; }
            self_type floor() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::floor(v[index]); return result; }
            self_type round() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::round(v[index]); return result; }
            value_type distance(const self_type& right) const { value_type total = 0; for (uint32_t index = 0; index < Size; ++index) total += ((v[index] - right.v[index]) * (v[index] - right.v[index])); return std::sqrt(total); }
            value_type dot(const self_type& right) const
            {
                value_type result = constants::zero<value_type>;
                for (uint32_t index = 0; index < Size; ++index)
                    result += (v[index] * right[index]);
                return result;
            }
            self_type hadamard_product(const self_type& right) const
            {
                self_type result = *this;
                result *= right;
                return result;
            }
        public:
            friend void swap(self_type& a, self_type& b)
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

        template <typename T, uint32_t Size, typename Type>
        inline bool operator==(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v == aRhs.v;
        }

        template <typename T, uint32_t Size, typename Type>
        inline bool operator<(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v < aRhs.v;
        }

        template <typename T, uint32_t Size, typename Type>
        inline std::partial_ordering operator<=>(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v <=> aRhs.v;
        }

        typedef basic_vector<double, 1> vector1;
        typedef basic_vector<double, 2> vector2;
        typedef basic_vector<double, 3> vector3;
        typedef basic_vector<double, 4> vector4;

        typedef vector1 vec1;
        typedef vector2 vec2;
        typedef vector3 vec3;
        typedef vector4 vec4;

        typedef vec1 col_vec1;
        typedef vec2 col_vec2;
        typedef vec3 col_vec3;
        typedef vec4 col_vec4;

        typedef basic_vector<double, 1, row_vector> row_vec1;
        typedef basic_vector<double, 2, row_vector> row_vec2;
        typedef basic_vector<double, 3, row_vector> row_vec3;
        typedef basic_vector<double, 4, row_vector> row_vec4;

        typedef optional<vector1> optional_vector1;
        typedef optional<vector2> optional_vector2;
        typedef optional<vector3> optional_vector3;
        typedef optional<vector4> optional_vector4;

        typedef optional<vec1> optional_vec1;
        typedef optional<vec2> optional_vec2;
        typedef optional<vec3> optional_vec3;
        typedef optional<vec4> optional_vec4;

        typedef optional<col_vec1> optional_col_vec1;
        typedef optional<col_vec2> optional_col_vec2;
        typedef optional<col_vec3> optional_col_vec3;
        typedef optional<col_vec4> optional_col_vec4;

        typedef optional<row_vec1> optional_row_vec1;
        typedef optional<row_vec2> optional_row_vec2;
        typedef optional<row_vec3> optional_row_vec3;
        typedef optional<row_vec4> optional_row_vec4;

        typedef std::vector<vec2> vec2_list;
        typedef std::vector<vec3> vec3_list;

        typedef optional<vec2_list> optional_vec2_list;
        typedef optional<vec3_list> optional_vec3_list;

        typedef vec2_list vertices_2d;
        typedef vec3_list vertices;

        typedef optional_vec2_list optional_vertices_2d_t;
        typedef optional_vec3_list optional_vertices_t;

        typedef basic_vector<float, 1> vector1f;
        typedef basic_vector<float, 2> vector2f;
        typedef basic_vector<float, 3> vector3f;
        typedef basic_vector<float, 4> vector4f;

        typedef vector1f vec1f;
        typedef vector2f vec2f;
        typedef vector3f vec3f;
        typedef vector4f vec4f;

        typedef int32_t i32;
        typedef int64_t i64;

        typedef basic_vector<i32, 1> vector1i32;
        typedef basic_vector<i32, 2> vector2i32;
        typedef basic_vector<i32, 3> vector3i32;
        typedef basic_vector<i32, 4> vector4i32;

        typedef vector1i32 vec1i32;
        typedef vector2i32 vec2i32;
        typedef vector3i32 vec3i32;
        typedef vector4i32 vec4i32;

        typedef uint32_t u32;
        typedef uint32_t u64;

        typedef basic_vector<u32, 1> vector1u32;
        typedef basic_vector<u32, 2> vector2u32;
        typedef basic_vector<u32, 3> vector3u32;
        typedef basic_vector<u32, 4> vector4u32;

        typedef vector1u32 vec1u32;
        typedef vector2u32 vec2u32;
        typedef vector3u32 vec3u32;
        typedef vector4u32 vec4u32;

        template <std::size_t VertexCount>
        using vec3_array = neolib::vecarray<vec3, VertexCount, VertexCount, neolib::check<neolib::vecarray_overflow>, std::allocator<vec3>>;

        template <std::size_t VertexCount>
        using vec2_array = neolib::vecarray<vec2, VertexCount, VertexCount, neolib::check<neolib::vecarray_overflow>, std::allocator<vec2>>;

        typedef std::array<int8_t, 1> avec1i8;
        typedef std::array<int8_t, 2> avec2i8;
        typedef std::array<int8_t, 3> avec3i8;
        typedef std::array<int8_t, 4> avec4i8;

        typedef std::array<int16_t, 1> avec1i16;
        typedef std::array<int16_t, 2> avec2i16;
        typedef std::array<int16_t, 3> avec3i16;
        typedef std::array<int16_t, 4> avec4i16;

        typedef std::array<int32_t, 1> avec1i32;
        typedef std::array<int32_t, 2> avec2i32;
        typedef std::array<int32_t, 3> avec3i32;
        typedef std::array<int32_t, 4> avec4i32;

        typedef std::array<uint8_t, 1> avec1u8;
        typedef std::array<uint8_t, 2> avec2u8;
        typedef std::array<uint8_t, 3> avec3u8;
        typedef std::array<uint8_t, 4> avec4u8;

        typedef std::array<uint16_t, 1> avec1u16;
        typedef std::array<uint16_t, 2> avec2u16;
        typedef std::array<uint16_t, 3> avec3u16;
        typedef std::array<uint16_t, 4> avec4u16;

        typedef std::array<uint32_t, 1> avec1u32;
        typedef std::array<uint32_t, 2> avec2u32;
        typedef std::array<uint32_t, 3> avec3u32;
        typedef std::array<uint32_t, 4> avec4u32;

        typedef std::array<float, 1> avec1f;
        typedef std::array<float, 2> avec2f;
        typedef std::array<float, 3> avec3f;
        typedef std::array<float, 4> avec4f;

        typedef std::array<double, 1> avec1;
        typedef std::array<double, 2> avec2;
        typedef std::array<double, 3> avec3;
        typedef std::array<double, 4> avec4;

        typedef std::array<vec3, 3> triangle;
        typedef std::array<vec3, 4> quad;

        typedef std::array<vec2, 3> triangle_2d;
        typedef std::array<vec2, 4> quad_2d;

        typedef std::array<vec3f, 3> trianglef;
        typedef std::array<vec3f, 4> quadf;

        typedef std::array<vec2f, 3> trianglef_2d;
        typedef std::array<vec2f, 4> quadf_2d;

        template <typename T, uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator+(const basic_vector<T, D, Type>& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result = left;
            result += right;
            return result;
        }

        template <typename T, uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator-(const basic_vector<T, D, Type>& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result = left;
            result -= right;
            return result;
        }

        template <typename T, uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount> operator+(const basic_vector<T, D, Type>& left, const std::array<basic_vector<T, D, Type>, VertexCount>& right)
        {
            std::array<basic_vector<T, D, Type>, VertexCount> result = right;
            for (auto& v : result)
                v += left;
            return result;
        }

        template <typename T, uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount> operator+(const std::array<basic_vector<T, D, Type>, VertexCount>& left, const basic_vector<T, D, Type>& right)
        {
            std::array<basic_vector<T, D, Type>, VertexCount> result = left;
            for (auto& v : result)
                v += right;
            return result;
        }

        template <typename T, uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount>& operator+=(std::array<basic_vector<T, D, Type>, VertexCount>& left, const basic_vector<T, D, Type>& right)
        {
            for (auto& v : left)
                v += right;
            return left;
        }

        template <typename T, uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount> operator-(const basic_vector<T, D, Type>& left, const std::array<basic_vector<T, D, Type>, VertexCount>& right)
        {
            std::array<basic_vector<T, D, Type>, VertexCount> result = right;
            for (auto& v : result)
                v = left - v;
            return result;
        }

        template <typename T, uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount> operator-(const std::array<basic_vector<T, D, Type>, VertexCount>& left, const basic_vector<T, D, Type>& right)
        {
            std::array<basic_vector<T, D, Type>, VertexCount> result = left;
            for (auto& v : result)
                v -= right;
            return result;
        }

        template <typename T, uint32_t D, typename Type, std::size_t VertexCount>
        inline std::array<basic_vector<T, D, Type>, VertexCount>& operator-=(std::array<basic_vector<T, D, Type>, VertexCount>& left, const basic_vector<T, D, Type>& right)
        {
            for (auto& v : left)
                v -= right;
            return left;
        }

        template <typename T, uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator+(const basic_vector<T, D, Type>& left, const T& right)
        {
            basic_vector<T, D, Type> result = left;
            for (uint32_t i = 0; i < D; ++i)
                result[i] += right;
            return result;
        }

        template <typename T, uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator+(const T& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result = right;
            for (uint32_t i = 0; i < D; ++i)
                result[i] += left;
            return result;
        }

        template <typename T, uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator-(const basic_vector<T, D, Type>& left, const T& right)
        {
            basic_vector<T, D, Type> result = left;
            for (uint32_t i = 0; i < D; ++i)
                result[i] -= right;
            return result;
        }

        template <typename T, uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator-(const T& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result;
            for (uint32_t i = 0; i < D; ++i)
                result[i] = left - right[i];
            return result;
        }

        template <typename T, uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator*(const basic_vector<T, D, Type>& left, const T& right)
        {
            basic_vector<T, D, Type> result = left;
            for (uint32_t i = 0; i < D; ++i)
                result[i] *= right;
            return result;
        }

        template <typename T, uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator*(const T& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result = right;
            for (uint32_t i = 0; i < D; ++i)
                result[i] *= left;
            return result;
        }

        template <typename T, uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator/(const basic_vector<T, D, Type>& left, const T& right)
        {
            basic_vector<T, D, Type> result = left;
            for (uint32_t i = 0; i < D; ++i)
                result[i] /= right;
            return result;
        }

        template <typename T, uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator/(const T& left, const basic_vector<T, D, Type>& right)
        {
            basic_vector<T, D, Type> result;
            for (uint32_t i = 0; i < D; ++i)
                result[i] = left / right[i];
            return result;
        }

        template <typename T, uint32_t D, typename Type>
        inline basic_vector<T, D, Type> operator%(const basic_vector<T, D, Type>& left, const T& right)
        {
            basic_vector<T, D, Type> result;
            for (uint32_t i = 0; i < D; ++i)
                result[i] = std::fmod(left[i], right);
            return result;
        }

        template <typename T, uint32_t D>
        inline T operator*(const basic_vector<T, D, row_vector>& left, const basic_vector<T, D, column_vector>& right)
        {
            T result = {};
            for (uint32_t index = 0; index < D; ++index)
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

        template <typename T, uint32_t Size, typename Type>
        inline basic_vector<T, Size, Type> lerp(const basic_vector<T, Size, Type>& aV1, const basic_vector<T, Size, Type>& aV2, double aAmount)
        {
            basic_vector<T, Size, Type> result;
            for (uint32_t i = 0; i < Size; ++i)
            {
                double x1 = aV1[i];
                double x2 = aV2[i];
                result[i] = static_cast<T>((x2 - x1) * aAmount + x1);
            }
            return result;
        }

        /* todo: specializations that use SIMD intrinsics. */
        template <typename T, uint32_t Rows, uint32_t Columns>
        class basic_matrix
        {
            typedef basic_matrix<T, Rows, Columns> self_type;
        public:
            typedef self_type abstract_type; // todo: abstract base
        public:
            typedef T value_type;
            typedef basic_vector<T, Columns, row_vector> row_type;
            typedef basic_vector<T, Rows, column_vector> column_type;
            typedef std::array<column_type, Columns> array_type;
        public:
            template <typename T2>
            struct rebind { typedef basic_matrix<T2, Rows, Columns> type; };
        public:
            basic_matrix() : m{ {} } {}
            basic_matrix(std::initializer_list<std::initializer_list<value_type>> aColumns) { std::copy(aColumns.begin(), aColumns.end(), m.begin()); }
            basic_matrix(const self_type& other) : m{ other.m }, isIdentity{ other.isIdentity } {}
            basic_matrix(self_type&& other) : m{ std::move(other.m) }, isIdentity{ other.isIdentity } {}
            template <typename T2>
            basic_matrix(const basic_matrix<T2, Rows, Columns>& other)
            {
                for (uint32_t column = 0; column < Columns; ++column)
                    for (uint32_t row = 0; row < Rows; ++row)
                        (*this)[column][row] = static_cast<value_type>(other[column][row]);
                isIdentity = other.maybe_identity();
            }
            self_type& operator=(const self_type& other) { m = other.m; isIdentity = other.isIdentity; return *this; }
            self_type& operator=(self_type&& other) { m = std::move(other.m); isIdentity = other.isIdentity; return *this; }
        public:
            template <typename T2>
            basic_matrix<T2, Rows, Columns> as() const
            {
                return basic_matrix<T2, Rows, Columns>{ *this };
            }
        public:
            std::pair<uint32_t, uint32_t> size() const { return std::make_pair(Rows, Columns); }
            const column_type& operator[](uint32_t aColumn) const { return m[aColumn]; }
            column_type& operator[](uint32_t aColumn) { isIdentity = std::nullopt; return m[aColumn]; }
            const value_type* data() const { return &m[0].v[0]; }
        public:
            bool operator==(const self_type& right) const { return m == right.m; }
            bool operator!=(const self_type& right) const { return m != right.m; }
            self_type& operator+=(const self_type& right) { for (uint32_t column = 0; column < Columns; ++column) (*this)[column] += right[column]; return *this; }
            self_type& operator-=(const self_type& right) { for (uint32_t column = 0; column < Columns; ++column) (*this)[column] -= right[column]; return *this; }
            self_type& operator*=(const self_type& right)
            {
                self_type result;
                for (uint32_t column = 0; column < Columns; ++column)
                    for (uint32_t row = 0; row < Rows; ++row)
                        for (uint32_t index = 0; index < Columns; ++index)
                            result[column][row] += ((*this)[index][row] * right[column][index]);
                *this = result;
                return *this;
            }
            self_type operator-() const
            {
                self_type result = *this;
                for (uint32_t column = 0; column < Columns; ++column)
                    for (uint32_t row = 0; row < Rows; ++row)
                        result[column][row] = -result[column][row];
                return result;
            }
            self_type round_to(value_type aEpsilon) const
            {
                self_type result;
                for (uint32_t column = 0; column < Columns; ++column)
                    for (uint32_t row = 0; row < Rows; ++row)
                    {
                         std::modf((*this)[column][row] / aEpsilon + 0.5, &result[column][row]);
                         result[column][row] *= aEpsilon;
                    }
                return result;
            }
            basic_matrix<T, Columns, Rows> transposed() const
            {
                basic_matrix<T, Columns, Rows> result;
                for (uint32_t column = 0; column < Columns; ++column)
                    for (uint32_t row = 0; row < Rows; ++row)
                        result[row][column] = (*this)[column][row];
                return result;
            }
            template <typename SFINAE = self_type>
            static const std::enable_if_t<Rows == Columns, SFINAE>& identity()
            {
                auto make_identity = []()
                {
                    self_type result;
                    for (uint32_t diag = 0; diag < Rows; ++diag)
                        result[diag][diag] = static_cast<value_type>(1.0);
                    result.isIdentity = true;
                    return result;
                };
                static self_type const sIdentity = make_identity();
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
            friend void swap(self_type& a, self_type& b)
            {
                using std::swap;
                swap(a.m, b.m);
                swap(a.isIdentity, b.isIdentity);
            }
        private:
            array_type m;
            mutable std::optional<bool> isIdentity;
        };

        typedef basic_matrix<double, 1, 1> matrix11;
        typedef basic_matrix<double, 2, 2> matrix22;
        typedef basic_matrix<double, 2, 1> matrix21;
        typedef basic_matrix<double, 1, 2> matrix12;
        typedef basic_matrix<double, 3, 3> matrix33;
        typedef basic_matrix<double, 3, 1> matrix31;
        typedef basic_matrix<double, 3, 2> matrix32;
        typedef basic_matrix<double, 1, 3> matrix13;
        typedef basic_matrix<double, 2, 3> matrix23;
        typedef basic_matrix<double, 4, 4> matrix44;
        typedef basic_matrix<double, 4, 1> matrix41;
        typedef basic_matrix<double, 4, 2> matrix42;
        typedef basic_matrix<double, 4, 3> matrix43;
        typedef basic_matrix<double, 1, 4> matrix14;
        typedef basic_matrix<double, 2, 4> matrix24;
        typedef basic_matrix<double, 3, 4> matrix34;

        typedef matrix11 matrix1;
        typedef matrix22 matrix2;
        typedef matrix33 matrix3;
        typedef matrix44 matrix4;

        typedef matrix11 mat11;
        typedef matrix22 mat22;
        typedef matrix21 mat21;
        typedef matrix12 mat12;
        typedef matrix33 mat33;
        typedef matrix31 mat31;
        typedef matrix32 mat32;
        typedef matrix13 mat13;
        typedef matrix23 mat23;
        typedef matrix44 mat44;
        typedef matrix41 mat41;
        typedef matrix42 mat42;
        typedef matrix43 mat43;
        typedef matrix14 mat14;
        typedef matrix24 mat24;
        typedef matrix34 mat34;

        typedef mat11 mat1;
        typedef mat22 mat2;
        typedef mat33 mat3;
        typedef mat44 mat4;

        typedef optional<matrix11> optional_matrix11;
        typedef optional<matrix22> optional_matrix22;
        typedef optional<matrix21> optional_matrix21;
        typedef optional<matrix12> optional_matrix12;
        typedef optional<matrix33> optional_matrix33;
        typedef optional<matrix31> optional_matrix31;
        typedef optional<matrix32> optional_matrix32;
        typedef optional<matrix13> optional_matrix13;
        typedef optional<matrix23> optional_matrix23;
        typedef optional<matrix44> optional_matrix44;
        typedef optional<matrix41> optional_matrix41;
        typedef optional<matrix42> optional_matrix42;
        typedef optional<matrix43> optional_matrix43;
        typedef optional<matrix14> optional_matrix14;
        typedef optional<matrix24> optional_matrix24;
        typedef optional<matrix34> optional_matrix34;

        typedef optional<matrix11> optional_matrix1;
        typedef optional<matrix22> optional_matrix2;
        typedef optional<matrix33> optional_matrix3;
        typedef optional<matrix44> optional_matrix4;

        typedef optional<mat11> optional_mat11;
        typedef optional<mat22> optional_mat22;
        typedef optional<mat21> optional_mat21;
        typedef optional<mat12> optional_mat12;
        typedef optional<mat33> optional_mat33;
        typedef optional<mat31> optional_mat31;
        typedef optional<mat32> optional_mat32;
        typedef optional<mat13> optional_mat13;
        typedef optional<mat23> optional_mat23;
        typedef optional<mat44> optional_mat44;
        typedef optional<mat41> optional_mat41;
        typedef optional<mat42> optional_mat42;
        typedef optional<mat43> optional_mat43;
        typedef optional<mat14> optional_mat14;
        typedef optional<mat24> optional_mat24;
        typedef optional<mat34> optional_mat34;

        typedef optional<mat11> optional_mat1;
        typedef optional<mat22> optional_mat2;
        typedef optional<mat33> optional_mat3;
        typedef optional<mat44> optional_mat4;

        typedef basic_matrix<float, 1, 1> matrix11f;
        typedef basic_matrix<float, 2, 2> matrix22f;
        typedef basic_matrix<float, 2, 1> matrix21f;
        typedef basic_matrix<float, 1, 2> matrix12f;
        typedef basic_matrix<float, 3, 3> matrix33f;
        typedef basic_matrix<float, 3, 1> matrix31f;
        typedef basic_matrix<float, 3, 2> matrix32f;
        typedef basic_matrix<float, 1, 3> matrix13f;
        typedef basic_matrix<float, 2, 3> matrix23f;
        typedef basic_matrix<float, 4, 4> matrix44f;
        typedef basic_matrix<float, 4, 1> matrix41f;
        typedef basic_matrix<float, 4, 2> matrix42f;
        typedef basic_matrix<float, 4, 3> matrix43f;
        typedef basic_matrix<float, 1, 4> matrix14f;
        typedef basic_matrix<float, 2, 4> matrix24f;
        typedef basic_matrix<float, 3, 4> matrix34f;

        typedef matrix11f mat11f;
        typedef matrix22f mat22f;
        typedef matrix21f mat21f;
        typedef matrix12f mat12f;
        typedef matrix33f mat33f;
        typedef matrix31f mat31f;
        typedef matrix32f mat32f;
        typedef matrix13f mat13f;
        typedef matrix23f mat23f;
        typedef matrix44f mat44f;
        typedef matrix41f mat41f;
        typedef matrix42f mat42f;
        typedef matrix43f mat43f;
        typedef matrix14f mat14f;
        typedef matrix24f mat24f;
        typedef matrix34f mat34f;

        typedef matrix11f mat1f;
        typedef matrix22f mat2f;
        typedef matrix33f mat3f;
        typedef matrix44f mat4f;

        typedef optional<matrix11f> optional_matrix11f;
        typedef optional<matrix22f> optional_matrix22f;
        typedef optional<matrix21f> optional_matrix21f;
        typedef optional<matrix12f> optional_matrix12f;
        typedef optional<matrix33f> optional_matrix33f;
        typedef optional<matrix31f> optional_matrix31f;
        typedef optional<matrix32f> optional_matrix32f;
        typedef optional<matrix13f> optional_matrix13f;
        typedef optional<matrix23f> optional_matrix23f;
        typedef optional<matrix44f> optional_matrix44f;
        typedef optional<matrix41f> optional_matrix41f;
        typedef optional<matrix42f> optional_matrix42f;
        typedef optional<matrix43f> optional_matrix43f;
        typedef optional<matrix14f> optional_matrix14f;
        typedef optional<matrix24f> optional_matrix24f;
        typedef optional<matrix34f> optional_matrix34f;

        typedef optional<matrix11f> optional_matrix1f;
        typedef optional<matrix22f> optional_matrix2f;
        typedef optional<matrix33f> optional_matrix3f;
        typedef optional<matrix44f> optional_matrix4f;

        typedef optional<mat11f> optional_mat11f;
        typedef optional<mat22f> optional_mat22f;
        typedef optional<mat21f> optional_mat21f;
        typedef optional<mat12f> optional_mat12f;
        typedef optional<mat33f> optional_mat33f;
        typedef optional<mat31f> optional_mat31f;
        typedef optional<mat32f> optional_mat32f;
        typedef optional<mat13f> optional_mat13f;
        typedef optional<mat23f> optional_mat23f;
        typedef optional<mat44f> optional_mat44f;
        typedef optional<mat41f> optional_mat41f;
        typedef optional<mat42f> optional_mat42f;
        typedef optional<mat43f> optional_mat43f;
        typedef optional<mat14f> optional_mat14f;
        typedef optional<mat24f> optional_mat24f;
        typedef optional<mat34f> optional_mat34f;

        typedef optional<mat11f> optional_mat1f;
        typedef optional<mat22f> optional_mat2f;
        typedef optional<mat33f> optional_mat3f;
        typedef optional<mat44f> optional_mat4f;

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator+(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result += right;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator-(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result -= right;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator*(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result *= right;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator/(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result /= right;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator+(scalar left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = right;
            result += left;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator-(scalar left, const basic_matrix<T, Rows, Columns>& right)
        {
            return -right + left;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator*(scalar left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = right;
            result *= left;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator+(const basic_matrix<T, Rows, Columns>& left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result += right;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator-(const basic_matrix<T, Rows, Columns>& left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result -= right;
            return result;
        }

        template <typename T, uint32_t D1, uint32_t D2>
        inline basic_matrix<T, D1, D1> operator*(const basic_matrix<T, D1, D2>& left, const basic_matrix<T, D2, D1>& right)
        {
            if (left.is_identity())
                return right;
            if (right.is_identity())
                return left;
            basic_matrix<T, D1, D1> result;
            for (uint32_t column = 0u; column < D1; ++column)
                for (uint32_t row = 0u; row < D1; ++row)
                    for (uint32_t index = 0; index < D2; ++index)
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
            for (uint32_t column = 0u; column < 4u; ++column)
                for (uint32_t row = 0u; row < 4u; ++row)
                    result[column][row] = simd_fma_4d(left[0u][row], right[column][0u], left[1u][row], right[column][1u], left[2u][row], right[column][2u], left[3u][row], right[column][3u]);
            return result;
        }

        template <typename T, uint32_t D>
        inline basic_vector<T, D, column_vector> operator*(const basic_matrix<T, D, D>& left, const basic_vector<T, D, column_vector>& right)
        {
            if (left.is_identity())
                return right;
            basic_vector<T, D, column_vector> result;
            for (uint32_t row = 0; row < D; ++row)
                for (uint32_t index = 0; index < D; ++index)
                    result[row] += (left[index][row] * right[index]);
            return result;
        }

        template <typename T, uint32_t D, std::size_t VertexCount>
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
            for (uint32_t row = 0u; row < 4u; ++row)
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

        template <typename T, uint32_t D>
        inline basic_vector<T, D, row_vector> operator*(const basic_vector<T, D, row_vector>& left, const basic_matrix<T, D, D>& right)
        {
            if (right.is_identity())
                return left;
            basic_vector<T, D, row_vector> result;
            for (uint32_t column = 0; column < D; ++column)
                for (uint32_t index = 0; index < D; ++index)
                    result[column] += (left[index] * right[column][index]);
            return result;
        }

        template <typename T, uint32_t D, std::size_t VertexCount>
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
            for (uint32_t column = 0u; column < 4u; ++column)
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

        template <typename T, uint32_t D>
        inline basic_matrix<T, D, D> operator*(const basic_vector<T, D, column_vector>& left, const basic_vector<T, D, row_vector>& right)
        {
            basic_matrix<T, D, D> result;
            for (uint32_t column = 0; column < D; ++column)
                for (uint32_t row = 0; row < D; ++row)
                    result[column][row] = (left[row] * right[column]);
            return result;
        }

        template <typename T>
        inline basic_matrix<T, 4u, 4u> operator*(const basic_vector<T, 4u, column_vector>& left, const basic_vector<T, 4u, row_vector>& right)
        {
            basic_matrix<T, 4u, 4u> result;
            for (uint32_t column = 0; column < 4u; ++column)
                simd_mul_4d(left[0u], right[column], left[1u], right[column], left[2u], right[column], left[3u], right[column], result[column][0u], result[column][1u], result[column][2u], result[column][3u]);
            return result;
        }

        template <typename T, uint32_t D>
        inline basic_matrix<T, D, D> without_translation(const basic_matrix<T, D, D>& matrix)
        {
            auto result = matrix;
            for (uint32_t row = 0; row < D - 1; ++row)
                result[D - 1][row] = 0.0;
            return result;
        }

        template <typename Elem, typename Traits, typename T, uint32_t Size, typename Type>
        inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_vector<T, Size, Type>& aVector)
        {
            aStream << "[";
            for (uint32_t i = 0; i < Size; ++i)
            {
                if (i != 0)
                    aStream << ", ";
                aStream << aVector[i];
            }
            aStream << "]";
            return aStream;
        }

        template <typename Elem, typename Traits, typename T, uint32_t Size, typename Type>
        inline std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& aStream, basic_vector<T, Size, Type>& aVector)
        {
            auto previousImbued = aStream.getloc();
            if (typeid(std::use_facet<std::ctype<char>>(previousImbued)) != typeid(neolib::comma_and_brackets_as_whitespace))
                aStream.imbue(std::locale{ previousImbued, new neolib::comma_and_brackets_as_whitespace{} });
            for (uint32_t i = 0; i < Size; ++i)
                aStream >> aVector[i];
            aStream.imbue(previousImbued);
            return aStream;
        }

        template <typename Elem, typename Traits, typename T, uint32_t Rows, uint32_t Columns>
        inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_matrix<T, Rows, Columns>& aMatrix)
        {
            aStream << "[";
            for (uint32_t row = 0; row < Rows; ++row)
            {
                if (row != 0)
                    aStream << ", ";
                aStream << "[";
                for (uint32_t column = 0; column < Columns; ++column)
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

        template <typename Elem, typename Traits, typename T, uint32_t Rows, uint32_t Columns>
        inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const optional<basic_matrix<T, Rows, Columns>>& aMatrix)
        {
            if (aMatrix != std::nullopt)
                aStream << *aMatrix;
            else
                aStream << "[null]";
            return aStream;
        }

        template <typename Elem, typename Traits, typename T, uint32_t Rows, uint32_t Columns>
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
            for (uint32_t row = 0u; row < 3u; ++row)
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
        template <typename T, uint32_t Size, typename Type = column_vector>
        inline bool nearly_equal(basic_vector<T, Size, Type> const& lhs, basic_vector<T, Size, Type> const& rhs, scalar epsilon = 0.00001)
        {
            for (uint32_t index = 0; index < Size; ++index)
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
        basic_vector<T, D> quad_extents(std::array<basic_vector<T, D>, 4> const& aQuad)
        {
            return basic_vector<T, D>{ 
                (aQuad[1].distance(aQuad[0]) + aQuad[3].distance(aQuad[2])) / static_cast<T>(2.0),
                (aQuad[0].distance(aQuad[3]) + aQuad[1].distance(aQuad[2])) / static_cast<T>(2.0)
            };
        }

        // AABB

        struct aabb
        {
            typedef aabb abstract_type;

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

        typedef optional<aabb> optional_aabb;

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
            typedef aabb_2d abstract_type;

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

        typedef optional<aabb_2d> optional_aabb_2d;

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
