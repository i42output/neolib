// i_component_data.hpp
/*
 *  Copyright (c) 2018, 2020 Leigh Johnston.
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
#include <neolib/core/numerical.hpp>
#include <neolib/ecs/ecs_ids.hpp>

namespace neolib::ecs
{
    using namespace neolib::math;

    template<class _Ty>
    using ecs_data_type_t = std::remove_cv_t<std::remove_reference_t<_Ty>>;

    class i_ecs;

    enum class component_data_field_type : uint32_t
    {
        Invalid             = 0x00000000,
        Bool                = 0x00000001,
        Int8                = 0x00000002,
        Uint8               = 0x00000003,
        Int16               = 0x00000004,
        Uint16              = 0x00000005,
        Int32               = 0x00000006,
        Uint32              = 0x00000007,
        Int64               = 0x00000008,
        Uint64              = 0x00000009,
        Float32             = 0x0000000A,
        Float64             = 0x0000000B,
        Scalar              = Float64,
        BasicVec2           = 0x00000100,
        BasicVec3           = 0x00000200,
        BasicVec4           = 0x00000300,
        Vec2                = BasicVec2 | Float64,
        Vec3                = BasicVec3 | Float64,
        Vec4                = BasicVec4 | Float64,
        Triangle            = BasicVec3 | Uint32,
        Face                = Triangle,
        BasicMat22          = 0x00000400,
        BasicMat33          = 0x00000500,
        BasicMat44          = 0x00000600,
        Mat22               = BasicMat22 | Float64,
        Mat33               = BasicMat33 | Float64,
        Mat44               = BasicMat44 | Float64,
        Aabb                = 0x00001000,
        Aabb2d              = 0x00002000,
        String              = 0x00010000,
        Enum                = 0x000B0000,
        Uuid                = 0x000C0000,
        Id                  = 0x000D0000,
        ComponentData       = 0x000E0000,
        Optional            = 0x01000000,
        Array               = 0x02000000,
        Shared              = 0x04000000,
        Internal            = 0x80000000,
    };

    inline constexpr component_data_field_type operator|(component_data_field_type aLhs, component_data_field_type aRhs)
    {
        return static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr component_data_field_type operator&(component_data_field_type aLhs, component_data_field_type aRhs)
    {
        return static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline constexpr component_data_field_type& operator|=(component_data_field_type& aLhs, component_data_field_type aRhs)
    {
        return aLhs = static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr component_data_field_type& operator&=(component_data_field_type& aLhs, component_data_field_type aRhs)
    {
        return aLhs = static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    struct i_component_data
    {
        struct meta
        {
            struct invalid_field_index : std::logic_error { invalid_field_index() : std::logic_error("ecs::i_component_data::meta::invalid_field_index") {} };

            static neolib::uuid field_type_id(uint32_t)
            {
                return neolib::uuid{};
            }

            static constexpr bool has_handles = false;
            static constexpr bool has_updater = false;
        };
    };
}