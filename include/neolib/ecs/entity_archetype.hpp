// entity_archetype.hpp
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
#include <neolib/core/set.hpp>
#include <neolib/core/allocator.hpp>
#include <neolib/core/uuid.hpp>
#include <neolib/core/string.hpp>
#include <neolib/ecs/ecs_ids.hpp>
#include <neolib/ecs/i_entity_archetype.hpp>

namespace neolib::ecs
{
    class entity_archetype : public i_entity_archetype
    {
    private:
        typedef neolib::set<component_id, std::less<component_id>, neolib::fast_pool_allocator<component_id>> component_list;
    public:
        entity_archetype(const entity_archetype_id& aId, const std::string& aName, std::initializer_list<component_id> aComponents);
        entity_archetype(const std::string& aName, std::initializer_list<component_id> aComponents);
        entity_archetype(const entity_archetype& aOther);
        entity_archetype(entity_archetype&& aOther);
    public:
        const entity_archetype_id& id() const override;
        const i_string& name() const override;
        const i_set<component_id>& components() const override;
        i_set<component_id>& components() override;
        void populate_default_components(i_ecs& aEcs, entity_id aEntity) override;
    private:
        entity_archetype_id iId;
        string iName;
        component_list iComponents;
    };
}