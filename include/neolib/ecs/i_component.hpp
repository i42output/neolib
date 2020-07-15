// i_component.hpp
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
#include <neolib/core/string.hpp>
#include <neolib/core/i_mutex.hpp>
#include <neolib/ecs/ecs_ids.hpp>
#include <neolib/ecs/i_component_data.hpp>

namespace neolib::ecs
{
    class i_ecs;

    class i_component_base
    {
    public:
        virtual ~i_component_base() = default;
    public:
        virtual i_ecs& ecs() const = 0;
        virtual const component_id& id() const = 0;
    public:
        virtual neolib::i_lockable& mutex() const = 0;
    public:
        virtual bool is_data_optional() const = 0;
        virtual const neolib::i_string& name() const = 0;
        virtual uint32_t field_count() const = 0;
        virtual component_data_field_type field_type(uint32_t aFieldIndex) const = 0;
        virtual neolib::uuid field_type_id(uint32_t aFieldIndex) const = 0;
        virtual const neolib::i_string& field_name(uint32_t aFieldIndex) const = 0;
    };

    class i_shared_component : public i_component_base
    {
    public:
        virtual const void* populate(const std::string& aName, const void* aComponentData, std::size_t aComponentDataSize) = 0;
        template <typename ComponentData>
        const void* populate(const std::string& aName, ComponentData&& aComponentData)
        {
            return populate(aName, &std::forward<ComponentData>(aComponentData), sizeof(ComponentData));
        }
    };

    class i_component : public i_component_base
    {
    public:
        virtual bool has_entity_record_no_lock(entity_id aEntity) const = 0;
        virtual bool has_entity_record(entity_id aEntity) const = 0;
        virtual void destroy_entity_record(entity_id aEntity) = 0;
    public:
        virtual const void* populate(entity_id aEntity, const void* aComponentData, std::size_t aComponentDataSize) = 0;
        template <typename ComponentData>
        const void* populate(entity_id aEntity, ComponentData&& aComponentData)
        {
            return populate(aEntity, &std::forward<ComponentData>(aComponentData), sizeof(ComponentData));
        }
    };

    template <typename ComponentData>
    class static_component;

    template <typename ComponentData>
    class static_shared_component;
}