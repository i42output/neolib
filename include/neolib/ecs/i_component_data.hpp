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

    enum class component_data_field_type : std::uint64_t
    {
        Invalid             = 0x0000000000000000,
        Bool                = 0x0000000000000001,
        Int8                = 0x0000000000000002,
        Uint8               = 0x0000000000000003,
        Int16               = 0x0000000000000004,
        Uint16              = 0x0000000000000005,
        Int32               = 0x0000000000000006,
        Uint32              = 0x0000000000000007,
        Int64               = 0x0000000000000008,
        Uint64              = 0x0000000000000009,
        Float32             = 0x000000000000000A,
        Float64             = 0x000000000000000B,
        Scalar              = Float64,
        BasicVec2           = 0x0000000000000100,
        BasicVec3           = 0x0000000000000200,
        BasicVec4           = 0x0000000000000300,
        Vec2                = BasicVec2 | Float64,
        Vec3                = BasicVec3 | Float64,
        Vec4                = BasicVec4 | Float64,
        Vec2f               = BasicVec2 | Float32,
        Vec3f               = BasicVec3 | Float32,
        Vec4f               = BasicVec4 | Float32,
        Vec2i32             = BasicVec2 | Int32,
        Vec3i32             = BasicVec3 | Int32,
        Vec4i32             = BasicVec4 | Int32,
        Vec2u32             = BasicVec2 | Uint32,
        Vec3u32             = BasicVec3 | Uint32,
        Vec4u32             = BasicVec4 | Uint32,
        Vec2i64             = BasicVec2 | Int64,
        Vec3i64             = BasicVec3 | Int64,
        Vec4i64             = BasicVec4 | Int64,
        Vec2u64             = BasicVec2 | Uint64,
        Vec3u64             = BasicVec3 | Uint64,
        Vec4u64             = BasicVec4 | Uint64,
        Triangle            = Vec3u32,
        Face                = Triangle,
        BasicMat22          = 0x0000000000000400,
        BasicMat33          = 0x0000000000000500,
        BasicMat44          = 0x0000000000000600,
        Mat22               = BasicMat22 | Float64,
        Mat33               = BasicMat33 | Float64,
        Mat44               = BasicMat44 | Float64,
        Mat22f              = BasicMat22 | Float32,
        Mat33f              = BasicMat33 | Float32,
        Mat44f              = BasicMat44 | Float32,
        Aabb3d              = 0x0000000000001000 | Float64,
        Aabb2d              = 0x0000000000002000 | Float64,
        Aabb3df             = 0x0000000000001000 | Float32,
        Aabb2df             = 0x0000000000002000 | Float32,
        Aabb                = Aabb3d,
        Aabbf               = Aabb3df,
        String              = 0x0000000000010000,
        Enum                = 0x00000000000B0000,
        Uuid                = 0x00000000000C0000,
        Id                  = 0x00000000000D0000,
        SmallId             = 0x00000000000E0000,
        ComponentData       = 0x00000000000F0000,
        Generator0          = 0x0000000000100000,
        Generator1          = 0x0000000000200000,
        Generator2          = 0x0000000000300000,
        Generator3          = 0x0000000000400000,
        Generator4          = 0x0000000000500000,
        Optional            = 0x0000010000000000,
        Array               = 0x0000020000000000,
        Shared              = 0x0000040000000000,
        Atomic              = 0x0000080000000000,
        GeneratorFactory    = 0x0000100000000000,
        Cache               = 0x4000000000000000,
        Internal            = 0x8000000000000000
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