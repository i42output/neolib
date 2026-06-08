// easing.hpp
/*
 *  Copyright (c) 2018-2026 Leigh Johnston.
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
 /* TERMS OF USE - EASING EQUATIONS
  * Open source under the BSD License.
  * Copyright(c)2001 Robert Penner
  * All rights reserved.
  * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met :
  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the author nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  */

#include <neolib/neolib.hpp>
#include <algorithm>
#include <numeric>
#include <neolib/core/enum.hpp>
#include <neolib/core/numerical.hpp>
#include <neolib/core/string_utils.hpp>

namespace neolib
{
    namespace math
    {
        template <typename T>
        inline T ease_linear(T t)
        {
            return t;
        }

        template <typename T>
        inline T ease_in_sine(T t)
        {
            return 1.0 - std::cos(t * math::half_pi<T>());
        }

        template <typename T>
        inline T ease_out_sine(T t)
        {
            return 1.0 - ease_in_sine(1.0 - t);
        }

        template <typename T>
        inline T ease_in_out_sine(T t)
        {
            return (t < 0.5 ? ease_in_sine(t * 2.0) : 1.0 + ease_out_sine((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_out_in_sine(T t)
        {
            return (t < 0.5 ? ease_out_sine(t * 2.0) : 1.0 + ease_in_sine((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_in_quad(T t)
        {
            return t * t;
        }

        template <typename T>
        inline T ease_out_quad(T t)
        {
            return 1.0 - ease_in_quad(1.0 - t);
        }

        template <typename T>
        inline T ease_in_out_quad(T t)
        {
            return (t < 0.5 ? ease_in_quad(t * 2.0) : 1.0 + ease_out_quad((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_out_in_quad(T t)
        {
            return (t < 0.5 ? ease_out_quad(t * 2.0) : 1.0 + ease_in_quad((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_in_cubic(T t)
        {
            return t * t * t;
        }

        template <typename T>
        inline T ease_out_cubic(T t)
        {
            return 1.0 - ease_in_cubic(1.0 - t);
        }

        template <typename T>
        inline T ease_in_out_cubic(T t)
        {
            return (t < 0.5 ? ease_in_cubic(t * 2.0) : 1.0 + ease_out_cubic((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_out_in_cubic(T t)
        {
            return (t < 0.5 ? ease_out_cubic(t * 2.0) : 1.0 + ease_in_cubic((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_in_quart(T t)
        {
            return t * t * t * t;
        }

        template <typename T>
        inline T ease_out_quart(T t)
        {
            return 1.0 - ease_in_quart(1.0 - t);
        }

        template <typename T>
        inline T ease_in_out_quart(T t)
        {
            return (t < 0.5 ? ease_in_quart(t * 2.0) : 1.0 + ease_out_quart((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_out_in_quart(T t)
        {
            return (t < 0.5 ? ease_out_quart(t * 2.0) : 1.0 + ease_in_quart((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_in_quint(T t)
        {
            return t * t * t * t * t;
        }

        template <typename T>
        inline T ease_out_quint(T t)
        {
            return 1.0 - ease_in_quint(1.0 - t);
        }

        template <typename T>
        inline T ease_in_out_quint(T t)
        {
            return (t < 0.5 ? ease_in_quint(t * 2.0) : 1.0 + ease_out_quint((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_out_in_quint(T t)
        {
            return (t < 0.5 ? ease_out_quint(t * 2.0) : 1.0 + ease_in_quint((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_in_expo(T t)
        {
            return std::pow(2.0, 10 * (t - 1.0));
        }

        template <typename T>
        inline T ease_out_expo(T t)
        {
            return 1.0 - ease_in_expo(1.0 - t);
        }

        template <typename T>
        inline T ease_in_out_expo(T t)
        {
            return (t < 0.5 ? ease_in_expo(t * 2.0) : 1.0 + ease_out_expo((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_out_in_expo(T t)
        {
            return (t < 0.5 ? ease_out_expo(t * 2.0) : 1.0 + ease_in_expo((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_in_circ(T t)
        {
            return 1.0 - std::sqrt(1.0 - t * t);
        }

        template <typename T>
        inline T ease_out_circ(T t)
        {
            return 1.0 - ease_in_circ(1.0 - t);
        }

        template <typename T>
        inline T ease_in_out_circ(T t)
        {
            return (t < 0.5 ? ease_in_circ(t * 2.0) : 1.0 + ease_out_circ((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_out_in_circ(T t)
        {
            return (t < 0.5 ? ease_out_circ(t * 2.0) : 1.0 + ease_in_circ((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T, typename Arg = T>
        inline T ease_in_back(T t, Arg s = 1.70158)
        {
            return t * t * ((s + 1.0) * t - s);
        }

        template <typename T, typename Arg = T>
        inline T ease_out_back(T t, T s = 1.70158)
        {
            return 1.0 - ease_in_back(1.0 - t, s);
        }

        template <typename T, typename Arg = T>
        inline T ease_in_out_back(T t, Arg s = 1.70158)
        {
            return (t < 0.5 ? ease_in_back(t * 2.0, s) : 1.0 + ease_out_back((t - 0.5) * 2.0, s)) / 2.0;
        }

        template <typename T, typename Arg = T>
        inline T ease_out_in_back(T t, Arg s = 1.70158)
        {
            return (t < 0.5 ? ease_out_back(t * 2.0, s) : 1.0 + ease_in_back((t - 0.5) * 2.0, s)) / 2.0;
        }

        template <typename T, typename Arg = T>
        inline T ease_in_elastic(T t, Arg a = 0.5, Arg p = 0.25)
        {
            if (t == 0.0)
                return 0;
            else if (t == 1.0)
                return 1.0;
            T s;
            if (a < 1.0)
            {
                a = 1.0;
                s = p / 4.0;
            }
            else
                s = p / (2 * pi<T>()) * std::asin(1.0 / a);
            return -(a * std::pow(2.0, 10.0 * (t - 1.0)) * std::sin(((t - 1.0) - s) * (two_pi<T>()) / p));
        }

        template <typename T, typename Arg = T>
        inline T ease_out_elastic(T t, Arg a = 0.5, Arg p = 0.25)
        {
            return 1.0 - ease_in_elastic(1.0 - t, a, p);
        }

        template <typename T, typename Arg = T>
        inline T ease_in_out_elastic(T t, Arg a = 0.5, Arg p = 0.25)
        {
            return (t < 0.5 ? ease_in_elastic(t * 2.0, a, p) : 1.0 + ease_out_elastic((t - 0.5) * 2.0, a, p)) / 2.0;
        }

        template <typename T, typename Arg = T>
        inline T ease_out_in_elastic(T t, Arg a = 0.5, Arg p = 0.25)
        {
            return (t < 0.5 ? ease_out_elastic(t * 2.0, a, p) : 1.0 + ease_in_elastic((t - 0.5) * 2.0, a, p)) / 2.0;
        }

        template <typename T>
        inline T ease_out_bounce(T t)
        {
            if (t < (1.0 / 2.75))
            {
                return (7.5625 * t * t);
            }
            else if (t < (2.0 / 2.75))
            {
                t -= (1.5 / 2.75);
                return (7.5625 * t * t + 0.75);
            }
            else if (t < (2.5 / 2.75))
            {
                t -= (2.25 / 2.75);
                return (7.5625 * t * t + 0.9375);
            }
            else
            {
                t -= (2.625 / 2.75);
                return (7.5625 * t * t + 0.984375);
            }
        }

        template <typename T>
        inline T ease_in_bounce(T t)
        {
            return 1.0 - ease_out_bounce(1.0 - t);
        }

        template <typename T>
        inline T ease_in_out_bounce(T t)
        {
            return (t < 0.5 ? ease_in_bounce(t * 2.0) : 1.0 + ease_out_bounce((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_out_in_bounce(T t)
        {
            return (t < 0.5 ? ease_out_bounce(t * 2.0) : 1.0 + ease_in_bounce((t - 0.5) * 2.0)) / 2.0;
        }

        template <typename T>
        inline T ease_zero(T)
        {
            return 0.0;
        }

        template <typename T>
        inline T ease_one(T)
        {
            return 1.0;
        }

        template <typename T>
        inline T ease_in_step(T t)
        {
            return t < 0.5 ? 0.0 : 1.0;
        }

        template <typename T>
        inline T ease_out_step(T t)
        {
            return 1.0 - ease_in_step(t);
        }

        template <typename T>
        inline T ease_in_out_step(T t)
        {
            return t < 0.5 ? ease_in_step(t * 2.0) : ease_out_step((t - 0.5) * 2.0);
        }

        template <typename T>
        inline T ease_out_in_step(T t)
        {
            return t < 0.5 ? ease_out_step(t * 2.0) : ease_in_step((t - 0.5) * 2.0);
        }

        enum class easing_class : std::uint32_t
        {
            Linear = 0x0000,
            Quad = 0x0001,
            Cubic = 0x0002,
            Quart = 0x0003,
            Quint = 0x0004,
            Sine = 0x0005,
            Expo = 0x0006,
            Circ = 0x0007,
            Elastic = 0x0008,
            Back = 0x0009,
            Bounce = 0x000A,
            Zero = 0x000B,
            One = 0x000C,
            Step = 0x000D,
            In = 0x0100,
            Out = 0x0200,
            Reversed = 0x0400,
            InOut = In | Out,
            OutIn = In | Out | Reversed,
            Inverted = 0x0800,
            Constant = 0x1000,
            CLASS_MASK = 0x00FF,
            DIRECTION_MASK = 0xFF00,
        };

        inline constexpr easing_class operator|(easing_class lhs, easing_class rhs)
        {
            return static_cast<easing_class>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
        }

        inline constexpr easing_class operator&(easing_class lhs, easing_class rhs)
        {
            return static_cast<easing_class>(static_cast<std::uint32_t>(lhs) & static_cast<std::uint32_t>(rhs));
        }

        inline constexpr easing_class operator~(easing_class lhs)
        {
            return static_cast<easing_class>(~static_cast<std::uint32_t>(lhs));
        }

        enum class easing : std::uint32_t
        {
            Linear = static_cast<std::uint32_t>(easing_class::Linear),
            InLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::In),
            OutLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::Out),
            InOutLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::InOut),
            OutInLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::OutIn),
            InQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::In),
            OutQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::Out),
            InOutQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::InOut),
            OutInQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::OutIn),
            InCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::In),
            OutCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::Out),
            InOutCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::InOut),
            OutInCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::OutIn),
            InQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::In),
            OutQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::Out),
            InOutQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::InOut),
            OutInQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::OutIn),
            InQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::In),
            OutQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::Out),
            InOutQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::InOut),
            OutInQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::OutIn),
            InSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::In),
            OutSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::Out),
            InOutSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::InOut),
            OutInSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::OutIn),
            InExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::In),
            OutExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::Out),
            InOutExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::InOut),
            OutInExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::OutIn),
            InCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::In),
            OutCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::Out),
            InOutCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::InOut),
            OutInCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::OutIn),
            InElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::In),
            OutElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::Out),
            InOutElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::InOut),
            OutInElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::OutIn),
            InBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::In),
            OutBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::Out),
            InOutBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::InOut),
            OutInBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::OutIn),
            InBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::In),
            OutBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::Out),
            InOutBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::InOut),
            OutInBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::OutIn),
            InStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::In),
            OutStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::Out),
            InOutStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::InOut),
            OutInStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::OutIn),
            Zero = static_cast<std::uint32_t>(easing_class::Zero | easing_class::Constant),
            One = static_cast<std::uint32_t>(easing_class::One | easing_class::Constant),
            ReversedLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::Reversed),
            ReversedInLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::In | easing_class::Reversed),
            ReversedOutLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::Out | easing_class::Reversed),
            ReversedInOutLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::InOut | easing_class::Reversed),
            ReversedOutInLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::OutIn | easing_class::Reversed),
            ReversedInQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::In | easing_class::Reversed),
            ReversedOutQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::Out | easing_class::Reversed),
            ReversedInOutQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::InOut | easing_class::Reversed),
            ReversedOutInQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::OutIn | easing_class::Reversed),
            ReversedInCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::In | easing_class::Reversed),
            ReversedOutCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::Out | easing_class::Reversed),
            ReversedInOutCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::InOut | easing_class::Reversed),
            ReversedOutInCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::OutIn | easing_class::Reversed),
            ReversedInQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::In | easing_class::Reversed),
            ReversedOutQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::Out | easing_class::Reversed),
            ReversedInOutQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::InOut | easing_class::Reversed),
            ReversedOutInQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::OutIn | easing_class::Reversed),
            ReversedInQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::In | easing_class::Reversed),
            ReversedOutQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::Out | easing_class::Reversed),
            ReversedInOutQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::InOut | easing_class::Reversed),
            ReversedOutInQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::OutIn | easing_class::Reversed),
            ReversedInSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::In | easing_class::Reversed),
            ReversedOutSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::Out | easing_class::Reversed),
            ReversedInOutSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::InOut | easing_class::Reversed),
            ReversedOutInSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::OutIn | easing_class::Reversed),
            ReversedInExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::In | easing_class::Reversed),
            ReversedOutExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::Out | easing_class::Reversed),
            ReversedInOutExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::InOut | easing_class::Reversed),
            ReversedOutInExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::OutIn | easing_class::Reversed),
            ReversedInCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::In | easing_class::Reversed),
            ReversedOutCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::Out | easing_class::Reversed),
            ReversedInOutCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::InOut | easing_class::Reversed),
            ReversedOutInCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::OutIn | easing_class::Reversed),
            ReversedInElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::In | easing_class::Reversed),
            ReversedOutElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::Out | easing_class::Reversed),
            ReversedInOutElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::InOut | easing_class::Reversed),
            ReversedOutInElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::OutIn | easing_class::Reversed),
            ReversedInBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::In | easing_class::Reversed),
            ReversedOutBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::Out | easing_class::Reversed),
            ReversedInOutBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::InOut | easing_class::Reversed),
            ReversedOutInBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::OutIn | easing_class::Reversed),
            ReversedInBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::In | easing_class::Reversed),
            ReversedOutBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::Out | easing_class::Reversed),
            ReversedInOutBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::InOut | easing_class::Reversed),
            ReversedOutInBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::OutIn | easing_class::Reversed),
            ReversedInStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::In | easing_class::Reversed),
            ReversedOutStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::Out | easing_class::Reversed),
            ReversedInOutStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::InOut | easing_class::Reversed),
            ReversedOutInStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::OutIn | easing_class::Reversed),
            ReversedZero = static_cast<std::uint32_t>(easing_class::Zero | easing_class::Constant | easing_class::Reversed),
            ReversedOne = static_cast<std::uint32_t>(easing_class::One | easing_class::Constant | easing_class::Reversed),
            InvertedLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::Inverted),
            InvertedInLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::In | easing_class::Inverted),
            InvertedOutLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::Out | easing_class::Inverted),
            InvertedInOutLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::InOut | easing_class::Inverted),
            InvertedOutInLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::OutIn | easing_class::Inverted),
            InvertedInQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::In | easing_class::Inverted),
            InvertedOutQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::Out | easing_class::Inverted),
            InvertedInOutQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::InOut | easing_class::Inverted),
            InvertedOutInQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::OutIn | easing_class::Inverted),
            InvertedInCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::In | easing_class::Inverted),
            InvertedOutCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::Out | easing_class::Inverted),
            InvertedInOutCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::InOut | easing_class::Inverted),
            InvertedOutInCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::OutIn | easing_class::Inverted),
            InvertedInQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::In | easing_class::Inverted),
            InvertedOutQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::Out | easing_class::Inverted),
            InvertedInOutQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::InOut | easing_class::Inverted),
            InvertedOutInQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::OutIn | easing_class::Inverted),
            InvertedInQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::In | easing_class::Inverted),
            InvertedOutQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::Out | easing_class::Inverted),
            InvertedInOutQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::InOut | easing_class::Inverted),
            InvertedOutInQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::OutIn | easing_class::Inverted),
            InvertedInSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::In | easing_class::Inverted),
            InvertedOutSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::Out | easing_class::Inverted),
            InvertedInOutSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::InOut | easing_class::Inverted),
            InvertedOutInSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::OutIn | easing_class::Inverted),
            InvertedInExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::In | easing_class::Inverted),
            InvertedOutExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::Out | easing_class::Inverted),
            InvertedInOutExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::InOut | easing_class::Inverted),
            InvertedOutInExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::OutIn | easing_class::Inverted),
            InvertedInCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::In | easing_class::Inverted),
            InvertedOutCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::Out | easing_class::Inverted),
            InvertedInOutCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::InOut | easing_class::Inverted),
            InvertedOutInCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::OutIn | easing_class::Inverted),
            InvertedInElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::In | easing_class::Inverted),
            InvertedOutElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::Out | easing_class::Inverted),
            InvertedInOutElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::InOut | easing_class::Inverted),
            InvertedOutInElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::OutIn | easing_class::Inverted),
            InvertedInBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::In | easing_class::Inverted),
            InvertedOutBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::Out | easing_class::Inverted),
            InvertedInOutBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::InOut | easing_class::Inverted),
            InvertedOutInBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::OutIn | easing_class::Inverted),
            InvertedInBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::In | easing_class::Inverted),
            InvertedOutBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::Out | easing_class::Inverted),
            InvertedInOutBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::InOut | easing_class::Inverted),
            InvertedOutInBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::OutIn | easing_class::Inverted),
            InvertedInStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::In | easing_class::Inverted),
            InvertedOutStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::Out | easing_class::Inverted),
            InvertedInOutStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::InOut | easing_class::Inverted),
            InvertedOutInStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::OutIn | easing_class::Inverted),
            InvertedZero = static_cast<std::uint32_t>(easing_class::Zero | easing_class::Constant | easing_class::Inverted),
            InvertedOne = static_cast<std::uint32_t>(easing_class::One | easing_class::Constant | easing_class::Inverted),
            ReversedInvertedLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInLinear = static_cast<std::uint32_t>(easing_class::Linear | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInQuad = static_cast<std::uint32_t>(easing_class::Quad | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInCubic = static_cast<std::uint32_t>(easing_class::Cubic | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInQuart = static_cast<std::uint32_t>(easing_class::Quart | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInQuint = static_cast<std::uint32_t>(easing_class::Quint | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInSine = static_cast<std::uint32_t>(easing_class::Sine | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInExpo = static_cast<std::uint32_t>(easing_class::Expo | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInCirc = static_cast<std::uint32_t>(easing_class::Circ | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInElastic = static_cast<std::uint32_t>(easing_class::Elastic | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInBack = static_cast<std::uint32_t>(easing_class::Back | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInBounce = static_cast<std::uint32_t>(easing_class::Bounce | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::In | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::Out | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedInOutStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::InOut | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOutInStep = static_cast<std::uint32_t>(easing_class::Step | easing_class::OutIn | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedZero = static_cast<std::uint32_t>(easing_class::Zero | easing_class::Constant | easing_class::Reversed | easing_class::Inverted),
            ReversedInvertedOne = static_cast<std::uint32_t>(easing_class::One | easing_class::Constant | easing_class::Reversed | easing_class::Inverted)
        };
    }
}

begin_declare_enum(neolib::math::easing)
declare_enum_string(neolib::math::easing, Linear)
declare_enum_string(neolib::math::easing, InLinear)
declare_enum_string(neolib::math::easing, OutLinear)
declare_enum_string(neolib::math::easing, InOutLinear)
declare_enum_string(neolib::math::easing, OutInLinear)
declare_enum_string(neolib::math::easing, InQuad)
declare_enum_string(neolib::math::easing, OutQuad)
declare_enum_string(neolib::math::easing, InOutQuad)
declare_enum_string(neolib::math::easing, OutInQuad)
declare_enum_string(neolib::math::easing, InCubic)
declare_enum_string(neolib::math::easing, OutCubic)
declare_enum_string(neolib::math::easing, InOutCubic)
declare_enum_string(neolib::math::easing, OutInCubic)
declare_enum_string(neolib::math::easing, InQuart)
declare_enum_string(neolib::math::easing, OutQuart)
declare_enum_string(neolib::math::easing, InOutQuart)
declare_enum_string(neolib::math::easing, OutInQuart)
declare_enum_string(neolib::math::easing, InQuint)
declare_enum_string(neolib::math::easing, OutQuint)
declare_enum_string(neolib::math::easing, InOutQuint)
declare_enum_string(neolib::math::easing, OutInQuint)
declare_enum_string(neolib::math::easing, InSine)
declare_enum_string(neolib::math::easing, OutSine)
declare_enum_string(neolib::math::easing, InOutSine)
declare_enum_string(neolib::math::easing, OutInSine)
declare_enum_string(neolib::math::easing, InExpo)
declare_enum_string(neolib::math::easing, OutExpo)
declare_enum_string(neolib::math::easing, InOutExpo)
declare_enum_string(neolib::math::easing, OutInExpo)
declare_enum_string(neolib::math::easing, InCirc)
declare_enum_string(neolib::math::easing, OutCirc)
declare_enum_string(neolib::math::easing, InOutCirc)
declare_enum_string(neolib::math::easing, OutInCirc)
declare_enum_string(neolib::math::easing, InElastic)
declare_enum_string(neolib::math::easing, OutElastic)
declare_enum_string(neolib::math::easing, InOutElastic)
declare_enum_string(neolib::math::easing, OutInElastic)
declare_enum_string(neolib::math::easing, InBack)
declare_enum_string(neolib::math::easing, OutBack)
declare_enum_string(neolib::math::easing, InOutBack)
declare_enum_string(neolib::math::easing, OutInBack)
declare_enum_string(neolib::math::easing, InBounce)
declare_enum_string(neolib::math::easing, OutBounce)
declare_enum_string(neolib::math::easing, InOutBounce)
declare_enum_string(neolib::math::easing, OutInBounce)
declare_enum_string(neolib::math::easing, InStep)
declare_enum_string(neolib::math::easing, OutStep)
declare_enum_string(neolib::math::easing, InOutStep)
declare_enum_string(neolib::math::easing, OutInStep)
declare_enum_string(neolib::math::easing, Zero)
declare_enum_string(neolib::math::easing, One)
declare_enum_string(neolib::math::easing, ReversedLinear)
declare_enum_string(neolib::math::easing, ReversedInLinear)
declare_enum_string(neolib::math::easing, ReversedOutLinear)
declare_enum_string(neolib::math::easing, ReversedInOutLinear)
declare_enum_string(neolib::math::easing, ReversedOutInLinear)
declare_enum_string(neolib::math::easing, ReversedInQuad)
declare_enum_string(neolib::math::easing, ReversedOutQuad)
declare_enum_string(neolib::math::easing, ReversedInOutQuad)
declare_enum_string(neolib::math::easing, ReversedOutInQuad)
declare_enum_string(neolib::math::easing, ReversedInCubic)
declare_enum_string(neolib::math::easing, ReversedOutCubic)
declare_enum_string(neolib::math::easing, ReversedInOutCubic)
declare_enum_string(neolib::math::easing, ReversedOutInCubic)
declare_enum_string(neolib::math::easing, ReversedInQuart)
declare_enum_string(neolib::math::easing, ReversedOutQuart)
declare_enum_string(neolib::math::easing, ReversedInOutQuart)
declare_enum_string(neolib::math::easing, ReversedOutInQuart)
declare_enum_string(neolib::math::easing, ReversedInQuint)
declare_enum_string(neolib::math::easing, ReversedOutQuint)
declare_enum_string(neolib::math::easing, ReversedInOutQuint)
declare_enum_string(neolib::math::easing, ReversedOutInQuint)
declare_enum_string(neolib::math::easing, ReversedInSine)
declare_enum_string(neolib::math::easing, ReversedOutSine)
declare_enum_string(neolib::math::easing, ReversedInOutSine)
declare_enum_string(neolib::math::easing, ReversedOutInSine)
declare_enum_string(neolib::math::easing, ReversedInExpo)
declare_enum_string(neolib::math::easing, ReversedOutExpo)
declare_enum_string(neolib::math::easing, ReversedInOutExpo)
declare_enum_string(neolib::math::easing, ReversedOutInExpo)
declare_enum_string(neolib::math::easing, ReversedInCirc)
declare_enum_string(neolib::math::easing, ReversedOutCirc)
declare_enum_string(neolib::math::easing, ReversedInOutCirc)
declare_enum_string(neolib::math::easing, ReversedOutInCirc)
declare_enum_string(neolib::math::easing, ReversedInElastic)
declare_enum_string(neolib::math::easing, ReversedOutElastic)
declare_enum_string(neolib::math::easing, ReversedInOutElastic)
declare_enum_string(neolib::math::easing, ReversedOutInElastic)
declare_enum_string(neolib::math::easing, ReversedInBack)
declare_enum_string(neolib::math::easing, ReversedOutBack)
declare_enum_string(neolib::math::easing, ReversedInOutBack)
declare_enum_string(neolib::math::easing, ReversedOutInBack)
declare_enum_string(neolib::math::easing, ReversedInBounce)
declare_enum_string(neolib::math::easing, ReversedOutBounce)
declare_enum_string(neolib::math::easing, ReversedInOutBounce)
declare_enum_string(neolib::math::easing, ReversedOutInBounce)
declare_enum_string(neolib::math::easing, ReversedInStep)
declare_enum_string(neolib::math::easing, ReversedOutStep)
declare_enum_string(neolib::math::easing, ReversedInOutStep)
declare_enum_string(neolib::math::easing, ReversedOutInStep)
declare_enum_string(neolib::math::easing, ReversedZero)
declare_enum_string(neolib::math::easing, ReversedOne)
declare_enum_string(neolib::math::easing, InvertedLinear)
declare_enum_string(neolib::math::easing, InvertedInLinear)
declare_enum_string(neolib::math::easing, InvertedOutLinear)
declare_enum_string(neolib::math::easing, InvertedInOutLinear)
declare_enum_string(neolib::math::easing, InvertedOutInLinear)
declare_enum_string(neolib::math::easing, InvertedInQuad)
declare_enum_string(neolib::math::easing, InvertedOutQuad)
declare_enum_string(neolib::math::easing, InvertedInOutQuad)
declare_enum_string(neolib::math::easing, InvertedOutInQuad)
declare_enum_string(neolib::math::easing, InvertedInCubic)
declare_enum_string(neolib::math::easing, InvertedOutCubic)
declare_enum_string(neolib::math::easing, InvertedInOutCubic)
declare_enum_string(neolib::math::easing, InvertedOutInCubic)
declare_enum_string(neolib::math::easing, InvertedInQuart)
declare_enum_string(neolib::math::easing, InvertedOutQuart)
declare_enum_string(neolib::math::easing, InvertedInOutQuart)
declare_enum_string(neolib::math::easing, InvertedOutInQuart)
declare_enum_string(neolib::math::easing, InvertedInQuint)
declare_enum_string(neolib::math::easing, InvertedOutQuint)
declare_enum_string(neolib::math::easing, InvertedInOutQuint)
declare_enum_string(neolib::math::easing, InvertedOutInQuint)
declare_enum_string(neolib::math::easing, InvertedInSine)
declare_enum_string(neolib::math::easing, InvertedOutSine)
declare_enum_string(neolib::math::easing, InvertedInOutSine)
declare_enum_string(neolib::math::easing, InvertedOutInSine)
declare_enum_string(neolib::math::easing, InvertedInExpo)
declare_enum_string(neolib::math::easing, InvertedOutExpo)
declare_enum_string(neolib::math::easing, InvertedInOutExpo)
declare_enum_string(neolib::math::easing, InvertedOutInExpo)
declare_enum_string(neolib::math::easing, InvertedInCirc)
declare_enum_string(neolib::math::easing, InvertedOutCirc)
declare_enum_string(neolib::math::easing, InvertedInOutCirc)
declare_enum_string(neolib::math::easing, InvertedOutInCirc)
declare_enum_string(neolib::math::easing, InvertedInElastic)
declare_enum_string(neolib::math::easing, InvertedOutElastic)
declare_enum_string(neolib::math::easing, InvertedInOutElastic)
declare_enum_string(neolib::math::easing, InvertedOutInElastic)
declare_enum_string(neolib::math::easing, InvertedInBack)
declare_enum_string(neolib::math::easing, InvertedOutBack)
declare_enum_string(neolib::math::easing, InvertedInOutBack)
declare_enum_string(neolib::math::easing, InvertedOutInBack)
declare_enum_string(neolib::math::easing, InvertedInBounce)
declare_enum_string(neolib::math::easing, InvertedOutBounce)
declare_enum_string(neolib::math::easing, InvertedInOutBounce)
declare_enum_string(neolib::math::easing, InvertedOutInBounce)
declare_enum_string(neolib::math::easing, InvertedInStep)
declare_enum_string(neolib::math::easing, InvertedOutStep)
declare_enum_string(neolib::math::easing, InvertedInOutStep)
declare_enum_string(neolib::math::easing, InvertedOutInStep)
declare_enum_string(neolib::math::easing, InvertedZero)
declare_enum_string(neolib::math::easing, InvertedOne)
declare_enum_string(neolib::math::easing, ReversedInvertedLinear)
declare_enum_string(neolib::math::easing, ReversedInvertedInLinear)
declare_enum_string(neolib::math::easing, ReversedInvertedOutLinear)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutLinear)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInLinear)
declare_enum_string(neolib::math::easing, ReversedInvertedInQuad)
declare_enum_string(neolib::math::easing, ReversedInvertedOutQuad)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutQuad)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInQuad)
declare_enum_string(neolib::math::easing, ReversedInvertedInCubic)
declare_enum_string(neolib::math::easing, ReversedInvertedOutCubic)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutCubic)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInCubic)
declare_enum_string(neolib::math::easing, ReversedInvertedInQuart)
declare_enum_string(neolib::math::easing, ReversedInvertedOutQuart)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutQuart)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInQuart)
declare_enum_string(neolib::math::easing, ReversedInvertedInQuint)
declare_enum_string(neolib::math::easing, ReversedInvertedOutQuint)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutQuint)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInQuint)
declare_enum_string(neolib::math::easing, ReversedInvertedInSine)
declare_enum_string(neolib::math::easing, ReversedInvertedOutSine)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutSine)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInSine)
declare_enum_string(neolib::math::easing, ReversedInvertedInExpo)
declare_enum_string(neolib::math::easing, ReversedInvertedOutExpo)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutExpo)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInExpo)
declare_enum_string(neolib::math::easing, ReversedInvertedInCirc)
declare_enum_string(neolib::math::easing, ReversedInvertedOutCirc)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutCirc)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInCirc)
declare_enum_string(neolib::math::easing, ReversedInvertedInElastic)
declare_enum_string(neolib::math::easing, ReversedInvertedOutElastic)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutElastic)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInElastic)
declare_enum_string(neolib::math::easing, ReversedInvertedInBack)
declare_enum_string(neolib::math::easing, ReversedInvertedOutBack)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutBack)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInBack)
declare_enum_string(neolib::math::easing, ReversedInvertedInBounce)
declare_enum_string(neolib::math::easing, ReversedInvertedOutBounce)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutBounce)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInBounce)
declare_enum_string(neolib::math::easing, ReversedInvertedInStep)
declare_enum_string(neolib::math::easing, ReversedInvertedOutStep)
declare_enum_string(neolib::math::easing, ReversedInvertedInOutStep)
declare_enum_string(neolib::math::easing, ReversedInvertedOutInStep)
declare_enum_string(neolib::math::easing, ReversedInvertedZero)
declare_enum_string(neolib::math::easing, ReversedInvertedOne)
end_declare_enum(neolib::math::easing)

namespace neolib
{
    namespace math
    {
        inline constexpr easing operator|(easing lhs, easing_class rhs)
        {
            return static_cast<easing>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
        }

        inline constexpr easing operator&(easing lhs, easing_class rhs)
        {
            return static_cast<easing>(static_cast<std::uint32_t>(lhs) & static_cast<std::uint32_t>(rhs));
        }

        inline constexpr easing operator^(easing lhs, easing_class rhs)
        {
            return static_cast<easing>(static_cast<std::uint32_t>(lhs) ^ static_cast<std::uint32_t>(rhs));
        }

        typedef std::optional<easing> optional_easing;

        typedef std::array<easing, 47> standard_easings_t;

        inline const standard_easings_t& standard_easings()
        {
            static constexpr standard_easings_t STANDARD_EASINGS =
            { {
                easing::Linear,
                easing::InQuad,
                easing::OutQuad,
                easing::InOutQuad,
                easing::OutInQuad,
                easing::InCubic,
                easing::OutCubic,
                easing::InOutCubic,
                easing::OutInCubic,
                easing::InQuart,
                easing::OutQuart,
                easing::InOutQuart,
                easing::OutInQuart,
                easing::InQuint,
                easing::OutQuint,
                easing::InOutQuint,
                easing::OutInQuint,
                easing::InSine,
                easing::OutSine,
                easing::InOutSine,
                easing::OutInSine,
                easing::InExpo,
                easing::OutExpo,
                easing::InOutExpo,
                easing::OutInExpo,
                easing::InCirc,
                easing::OutCirc,
                easing::InOutCirc,
                easing::OutInCirc,
                easing::InElastic,
                easing::OutElastic,
                easing::InOutElastic,
                easing::OutInElastic,
                easing::InBack,
                easing::OutBack,
                easing::InOutBack,
                easing::OutInBack,
                easing::InBounce,
                easing::OutBounce,
                easing::InOutBounce,
                easing::OutInBounce,
                easing::InStep,
                easing::OutStep,
                easing::InOutStep,
                easing::OutInStep,
                easing::Zero,
                easing::One
            } };
            return STANDARD_EASINGS;
        }

        inline std::uint32_t standard_easing_index(easing aEasing)
        {
            // todo: optimize this; perhaps use polymorphic enum.
            auto standardEasing = std::find(standard_easings().begin(), standard_easings().end(), aEasing);
            if (standardEasing != standard_easings().end())
                return static_cast<std::uint32_t>(std::distance(standard_easings().begin(), standardEasing));
            return standard_easing_index(easing::Zero);
        }

        template <typename T>
        inline T standard_ease(easing e, T t)
        {
            switch (e)
            {
            case easing::Linear:
            case easing::InLinear:
            case easing::OutLinear:
            case easing::InOutLinear:
            case easing::OutInLinear:
                return ease_linear(t);
            case easing::InQuad:
                return ease_in_quad(t);
            case easing::OutQuad:
                return ease_out_quad(t);
            case easing::InOutQuad:
                return ease_in_out_quad(t);
            case easing::OutInQuad:
                return ease_out_in_quad(t);
            case easing::InCubic:
                return ease_in_cubic(t);
            case easing::OutCubic:
                return ease_out_cubic(t);
            case easing::InOutCubic:
                return ease_in_out_cubic(t);
            case easing::OutInCubic:
                return ease_out_in_cubic(t);
            case easing::InQuart:
                return ease_in_quart(t);
            case easing::OutQuart:
                return ease_out_quart(t);
            case easing::InOutQuart:
                return ease_in_out_quart(t);
            case easing::OutInQuart:
                return ease_out_in_quart(t);
            case easing::InQuint:
                return ease_in_quint(t);
            case easing::OutQuint:
                return ease_out_quint(t);
            case easing::InOutQuint:
                return ease_in_out_quint(t);
            case easing::OutInQuint:
                return ease_out_in_quint(t);
            case easing::InSine:
                return ease_in_sine(t);
            case easing::OutSine:
                return ease_out_sine(t);
            case easing::InOutSine:
                return ease_in_out_sine(t);
            case easing::OutInSine:
                return ease_out_in_sine(t);
            case easing::InExpo:
                return ease_in_expo(t);
            case easing::OutExpo:
                return ease_out_expo(t);
            case easing::InOutExpo:
                return ease_in_out_expo(t);
            case easing::OutInExpo:
                return ease_out_in_expo(t);
            case easing::InCirc:
                return ease_in_circ(t);
            case easing::OutCirc:
                return ease_out_circ(t);
            case easing::InOutCirc:
                return ease_in_out_circ(t);
            case easing::OutInCirc:
                return ease_out_in_circ(t);
            case easing::InElastic:
                return ease_in_elastic(t);
            case easing::OutElastic:
                return ease_out_elastic(t);
            case easing::InOutElastic:
                return ease_in_out_elastic(t);
            case easing::OutInElastic:
                return ease_out_in_elastic(t);
            case easing::InBack:
                return ease_in_back(t);
            case easing::OutBack:
                return ease_out_back(t);
            case easing::InOutBack:
                return ease_in_out_back(t);
            case easing::OutInBack:
                return ease_out_in_back(t);
            case easing::InBounce:
                return ease_in_bounce(t);
            case easing::OutBounce:
                return ease_out_bounce(t);
            case easing::InOutBounce:
                return ease_in_out_bounce(t);
            case easing::OutInBounce:
                return ease_out_in_bounce(t);
            case easing::InStep:
                return ease_in_step(t);
            case easing::OutStep:
                return ease_out_step(t);
            case easing::InOutStep:
                return ease_in_out_step(t);
            case easing::OutInStep:
                return ease_out_in_step(t);
            case easing::Zero:
                return ease_zero(t);
            case easing::One:
                return ease_one(t);
            default:
                throw std::logic_error("neolib::standard_ease: unknown easing type");
            }
        }

        template <typename T>
        inline T ease(easing e, T t)
        {
            if (static_cast<easing_class>(e & easing_class::Reversed) == easing_class::Reversed)
                t = T{ 1 } - t;
            auto result = standard_ease(e & ~(easing_class::Reversed | easing_class::Inverted), t);
            if (static_cast<easing_class>(e & easing_class::Inverted) == easing_class::Inverted)
                result = T{ 1 } - result;
            return result;
        }

        template <typename T>
        inline T ease(easing_class in, easing_class out, T t)
        {
            return (t < 0.5 ? ease(static_cast<easing>(in | easing_class::In), t / 0.5) : 1.0 + ease(static_cast<easing>(out | easing_class::Out), (t - 0.5) / 0.5)) / 2.0;
        }

        template <typename T>
        inline T ease(easing e1, easing e2, T t)
        {
            return (t < 0.5 ? ease(e1, t / 0.5) : 1.0 + ease(e2, (t - 0.5) / 0.5)) / 2.0;
        }

        template <typename T>
        inline T ease(easing e1, easing e2, easing e3, easing e4, T t)
        {
            return (t < 0.25 ? ease(e1, t / 0.25) : t < 0.5 ? 1.0 + ease(e2, (t - 0.25) / 0.25) : t < 0.75 ? 2.0 + ease(e3, (t - 0.5) / 0.25) : 3.0 + ease(e4, (t - 0.75) / 0.25)) / 4.0;
        }

        template <typename T>
        inline T ease(easing e, T t, T b, T c, T d)
        {
            return ease(e, t / d) * c + b;
        }

        /**
         * @brief Partitioned easing function based on N easing segments.
         * @author Claude (AI)
         * @author Leigh Johnston (Human)
         * @tparam T value type
         * @param segments the partitions (segments) - easing function and weighting
         * @param t time [0.0 .. 1.0]
         * @return easing result
         */
        template <typename T>
        inline T partitioned_ease(std::span<std::pair<easing, T> const> segments, T t)
        {
            if (segments.empty())
                return T{ 0 };

            auto const wTotal = std::accumulate(segments.begin(), segments.end(), T{ 0 },
                [](T sum, auto const& s) { return sum + s.second; });

            auto const scale = wTotal / static_cast<T>(segments.size());

            if (wTotal <= T{ 0 })
                return T{ 0 };

            auto cumulative = T{ 0 };
            auto outputPos = ease(segments.front().first, T{ 0 });
            for (auto const& [e, w] : segments)
            {
                if (w <= T{ 0 })
                {
                    cumulative += w;
                    continue;
                }

                auto const localStart = ease(e, T{ 0 });

                if (t * wTotal < cumulative + w || &e == &segments.back().first)
                {
                    auto const local = std::clamp((t * wTotal - cumulative) / w, T{ 0 }, T{ 1 });
                    return scale * (outputPos + (ease(e, local) - localStart) * (w / wTotal));
                }

                outputPos += (ease(e, T{ 1 }) - localStart) * (w / wTotal);
                cumulative += w;
            }

            return scale * (ease(segments.back().first, T{ 1 }));
        }

        /**
         * @brief Partitioned easing function based on N easing segments.
         * @author Claude (AI)
         * @author Leigh Johnston (Human)
         * @tparam T value type
         * @tparam N number of partitions (segments)
         * @param segments the partitions (segments) - easing function and weighting
         * @param t time [0.0 .. 1.0]
         * @return easing result
         */
        template <typename T, std::size_t N>
        inline T partitioned_ease(std::span<std::pair<easing, T> const, N> segments, T t)
        {
            return partitioned_ease(std::span<std::pair<easing, T> const>{ segments }, t);
        }

        /**
         * @brief Partitioned easing function based on N easing segments.
         * @author Claude (AI)
         * @tparam T value type
         * @param segments the partitions (segments) - easing function and weighting
         * @param t time [0.0 .. 1.0]
         * @return easing result
         */
        template <typename T>
        inline T partitioned_ease(std::initializer_list<std::pair<easing, T>> segments, T t)
        {
            return partitioned_ease(std::span<std::pair<easing, T> const>{segments.begin(), segments.size()}, t);
        }

        template <typename T>
        inline std::enable_if_t<!std::is_same_v<T, easing>, T> partitioned_ease(easing e1, T t, T w1 = T{ 1 })
        {
            if (static_cast<easing_class>(e1 & easing_class::Reversed) == easing_class::Reversed)
                return partitioned_ease({ {e1,w1},{e1 ^ easing_class::Reversed,w1} }, t);
            return partitioned_ease({ {e1,w1},{e1 ^ easing_class::Inverted,w1} }, t);
        }

        template <typename T>
        inline std::enable_if_t<!std::is_same_v<T, easing>, T> partitioned_ease(easing e1, T t, T w1, T w2)
        {
            if (static_cast<easing_class>(e1 & easing_class::Reversed) == easing_class::Reversed)
                return partitioned_ease({ {e1,w1},{e1 ^ easing_class::Reversed,w2} }, t);
            return partitioned_ease({ {e1,w1},{e1 ^ easing_class::Inverted,w2} }, t);
        }

        template <typename T>
        inline T partitioned_ease(easing e1, easing e2, T t, T w1 = T{ 1 }, T w2 = T{ 1 })
        {
            return partitioned_ease({ {e1,w1},{e2,w2} }, t);
        }

        template <typename T>
        inline T partitioned_ease(easing e1, easing e2, easing e3, easing e4, T t,
            T w1 = T{ 1 }, T w2 = T{ 1 }, T w3 = T{ 1 }, T w4 = T{ 1 })
        {
            return partitioned_ease({ {e1,w1},{e2,w2},{e3,w3},{e4,w4} }, t);
        }
    }
}
