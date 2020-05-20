// swizzle_array.hpp
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
#include <neolib/core/swizzle.hpp>

namespace neolib
{
    namespace math
    {
        template <typename V, typename T, uint32_t Size>
        struct swizzle_array
        {
            typedef T value_type;
            typedef std::array<value_type, Size> array_type;
            typedef V vector_type;

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

        template <typename V, typename T>
        struct swizzle_array<V, T, 1>
        {
            typedef T value_type;
            typedef std::array<value_type, 1> array_type;
            typedef V vector_type;

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

        template <typename V, typename T>
        struct swizzle_array<V, T, 2>
        {
            typedef T value_type;
            typedef std::array<value_type, 2> array_type;
            typedef V vector_type;

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

        template <typename V, typename T>
        struct swizzle_array<V, T, 3>
        {
            typedef T value_type;
            typedef std::array<value_type, 3> array_type;
            typedef V vector_type;

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
    }
}
