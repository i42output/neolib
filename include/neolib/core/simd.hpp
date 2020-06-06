// simd.hpp
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
#include <array>
#include <thread>
#include <cstdlib>
#if defined(USE_AVX) || defined(USE_AVX_DYNAMIC)
#include <immintrin.h>
#endif
#if defined(USE_EMM) || defined(USE_EMM_DYNAMIC) 
#include <emmintrin.h>
#endif

namespace neolib
{
#if defined(USE_AVX_DYNAMIC) || defined(USE_EMM_DYNAMIC) 
    inline std::atomic<bool>& use_simd()
    {
        static std::atomic<bool> sUseSimd = true;
        return sUseSimd;
    }
#endif

#if defined(USE_AVX) || defined(USE_AVX_DYNAMIC)
    inline double to_scalar(__m256d const& avxRegister, std::size_t index)
    {
#ifdef _WIN32
        return avxRegister.m256d_f64[index];
#else
        return avxRegister[index];
#endif
    }
#endif

#if defined(USE_EMM) || defined(USE_EMM_DYNAMIC) 
    inline uint32_t to_scalar(__m128i const& emmRegister, std::size_t index)
    {
#ifdef _WIN32
        return emmRegister.m128i_u32[index];
#else
        return emmRegister[index];
#endif
    }
#endif

#if defined(USE_AVX) || defined(USE_AVX_DYNAMIC)
    inline double avx_simd_fma_4d(double x1, double x2, double y1, double y2, double z1, double z2, double w1, double w2)
    {
        alignas(32) __m256d lhs = _mm256_set_pd(x1, y1, z1, w1);
        alignas(32) __m256d rhs = _mm256_set_pd(x2, y2, z2, w2);
        alignas(32) __m256d ans = _mm256_mul_pd(lhs, rhs);
        return to_scalar(ans, 0) + to_scalar(ans, 1) + to_scalar(ans, 2) + to_scalar(ans, 3);
    }
#endif

    inline double fake_simd_fma_4d(double x1, double x2, double y1, double y2, double z1, double z2, double w1, double w2)
    {
        return x1 * x2 + y1 * y2 + z1 * z2 + w1 * w2;
    }

#if defined(USE_AVX)
    #define simd_fma_4d avx_simd_fma_4d
#elif defined(USE_AVX_DYNAMIC)
    inline double simd_fma_4d(double x1, double x2, double y1, double y2, double z1, double z2, double w1, double w2)
    {
        if (use_simd())
            return avx_simd_fma_4d(x1, x2, y1, y2, z1, z2, w1, w2);
        else
            return fake_simd_fma_4d(x1, x2, y1, y2, z1, z2, w1, w2);
    }
#else
    #define simd_fma_4d fake_simd_fma_4d
#endif

#if defined(USE_AVX) || defined(USE_AVX_DYNAMIC)
    inline void avx_simd_mul_4d(double x1, double x2, double y1, double y2, double z1, double z2, double w1, double w2, double& a, double& b, double& c, double& d)
    {
        alignas(32) __m256d lhs = _mm256_set_pd(x1, y1, z1, w1);
        alignas(32) __m256d rhs = _mm256_set_pd(x2, y2, z2, w2);
        alignas(32) __m256d ans = _mm256_mul_pd(lhs, rhs);
        a = to_scalar(ans, 0);
        b = to_scalar(ans, 1);
        c = to_scalar(ans, 2);
        d = to_scalar(ans, 3);
    }
#endif

    inline void fake_simd_mul_4d(double x1, double x2, double y1, double y2, double z1, double z2, double w1, double w2, double& a, double& b, double& c, double& d)
    {
        a = x1 * x2;
        b = y1 * y2;
        c = z1 * z2;
        d = w1 * w2;
    }

#if defined(USE_AVX)
    #define simd_mul_4d avx_simd_mul_4d
#elif defined(USE_AVX_DYNAMIC)
    inline void simd_mul_4d(double x1, double x2, double y1, double y2, double z1, double z2, double w1, double w2, double& a, double& b, double& c, double& d)
    {
        if (use_simd())
            avx_simd_mul_4d(x1, x2, y1, y2, z1, z2, w1, w2, a, b, c, d);
        else
            fake_simd_mul_4d(x1, x2, y1, y2, z1, z2, w1, w2, a, b, c, d);
    }
#else
    #define simd_mul_4d fake_simd_mul_4d
#endif

    /////////////////////////////////////////////////////////////////////////////
    // The Software is provided "AS IS" and possibly with faults. 
    // Intel disclaims any and all warranties and guarantees, express, implied or
    // otherwise, arising, with respect to the software delivered hereunder,
    // including but not limited to the warranty of merchantability, the warranty
    // of fitness for a particular purpose, and any warranty of non-infringement
    // of the intellectual property rights of any third party.
    // Intel neither assumes nor authorizes any person to assume for it any other
    // liability. Customer will use the software at its own risk. Intel will not
    // be liable to customer for any direct or indirect damages incurred in using
    // the software. In no event will Intel be liable for loss of profits, loss of
    // use, loss of data, business interruption, nor for punitive, incidental,
    // consequential, or special damages of any kind, even if advised of
    // the possibility of such damages.
    //
    // Copyright (c) 2003 Intel Corporation
    //
    // Third-party brands and names are the property of their respective owners
    //
    ///////////////////////////////////////////////////////////////////////////
    // Random Number Generation for SSE / SSE2
    // Source File
    // Version 0.1
    // Author Kipp Owens, Rajiv Parikh
    ////////////////////////////////////////////////////////////////////////

    namespace detail
    {
#if defined(USE_EMM) || defined(USE_EMM_DYNAMIC)
        inline __m128i& simd_rand_seed()
        {
            alignas(16) thread_local __m128i tSeed;
            return tSeed;
        }
#endif
    }

#if defined(USE_EMM) || defined(USE_EMM_DYNAMIC)
    inline void emm_simd_srand(uint32_t seed)
    {
        detail::simd_rand_seed() = _mm_set_epi32(seed, seed + 1, seed, seed + 1);
    }
#endif

    inline void fake_simd_srand(uint32_t seed)
    {
        std::srand(seed);
    }

#if defined(USE_EMM)
    #define simd_srand emm_simd_srand
#elif defined(USE_EMM_DYNAMIC)
    inline void simd_srand(uint32_t seed)
    {
        if (use_simd())
            emm_simd_srand(seed);
        else
            fake_simd_srand(seed);
    }
#else
    #define simd_srand fake_simd_srand
#endif
        
    inline void simd_srand(std::thread::id seed)
    {
        simd_srand(static_cast<uint32_t>(std::hash<std::thread::id>{}(seed)));
    }

#if defined(USE_EMM) || defined(USE_EMM_DYNAMIC)
    inline uint32_t emm_simd_rand()
    {
        thread_local std::array<uint32_t, 4> result = {};
        thread_local std::size_t resultCounter = 4;
        if (resultCounter < 4)
            return result[resultCounter++];
        alignas(16) __m128i cur_seed_split;
        alignas(16) __m128i multiplier;
        alignas(16) __m128i adder;
        alignas(16) __m128i mod_mask;
        alignas(16) __m128i sra_mask;
        alignas(16) __m128i ans;
        alignas(16) static const uint32_t mult[4] =
        { 214013, 17405, 214013, 69069 };
        alignas(16) static const uint32_t gadd[4] =
        { 2531011, 10395331, 13737667, 1 };
        alignas(16) static const uint32_t mask[4] =
        { 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 };
        alignas(16) static const uint32_t masklo[4] =
        { 0x00007FFF, 0x00007FFF, 0x00007FFF, 0x00007FFF };

        adder = _mm_load_si128((__m128i*) gadd);
        multiplier = _mm_load_si128((__m128i*) mult);
        mod_mask = _mm_load_si128((__m128i*) mask);
        sra_mask = _mm_load_si128((__m128i*) masklo);

        cur_seed_split = _mm_shuffle_epi32(detail::simd_rand_seed(), _MM_SHUFFLE(2, 3, 0, 1));

        detail::simd_rand_seed() = _mm_mul_epu32(detail::simd_rand_seed(), multiplier);

        multiplier = _mm_shuffle_epi32(multiplier, _MM_SHUFFLE(2, 3, 0, 1));
        cur_seed_split = _mm_mul_epu32(cur_seed_split, multiplier);

        detail::simd_rand_seed() = _mm_and_si128(detail::simd_rand_seed(), mod_mask);

        cur_seed_split = _mm_and_si128(cur_seed_split, mod_mask);
        cur_seed_split = _mm_shuffle_epi32(cur_seed_split, _MM_SHUFFLE(2, 3, 0, 1));

        detail::simd_rand_seed() = _mm_or_si128(detail::simd_rand_seed(), cur_seed_split);
        detail::simd_rand_seed() = _mm_add_epi32(detail::simd_rand_seed(), adder);

        _mm_storeu_si128(&ans, detail::simd_rand_seed());
        result = { to_scalar(ans, 0), to_scalar(ans, 1), to_scalar(ans, 2), to_scalar(ans, 3) };
        resultCounter = 0;
        return result[resultCounter];
    }
#endif

    inline uint32_t fake_simd_rand()
    {
        thread_local std::array<uint32_t, 4> result = {};
        thread_local std::size_t resultCounter = 4;
        if (resultCounter < 4)
            return result[resultCounter++];
        result = { static_cast<uint32_t>(std::rand()), static_cast<uint32_t>(std::rand()), static_cast<uint32_t>(std::rand()), static_cast<uint32_t>(std::rand()) };
        resultCounter = 0;
        return result[resultCounter];
    }

#if defined(USE_EMM)
    #define simd_rand emm_simd_rand
#elif defined(USE_EMM_DYNAMIC)
    inline uint32_t simd_rand()
    {
        if (use_simd())
            return emm_simd_rand();
        else
            return fake_simd_rand();
    }
#else
    #define simd_rand fake_simd_srand
#endif

    template <typename T>
    inline T simd_rand(T aUpper)
    {
        return static_cast<T>(simd_rand() % static_cast<uint32_t>(aUpper));
    }
}
