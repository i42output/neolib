// entity.cpp
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

#include <neolib/neolib.hpp>
#include <neolib/ecs/entity.hpp>
#include <neolib/ecs/entity_info.hpp>
#include <neolib/ecs/component.hpp>

namespace neolib::ecs
{
    entity::entity(i_ecs& aEcs, entity_id aId) :
        iEcs{ aEcs }, iId{ aId }
    {
        iSink += ecs().entity_destroyed([this](entity_id aId)
        {
            if (aId == iId)
                iId = null_entity;
        });
    }

    entity::entity(i_ecs& aEcs, const entity_archetype_id& aArchetypeId) :
        entity{ aEcs, aEcs.create_entity(aArchetypeId) }
    {
    }

    entity::~entity()
    {
        if (!detached_or_destroyed())
            ecs().destroy_entity(iId);
    }

    i_ecs& entity::ecs() const
    {
        return iEcs;
    }

    entity_id entity::id() const
    {
        return iId;
    }

    bool entity::detached_or_destroyed() const
    {
        return iId == null_entity;
    }

    entity_id entity::detach()
    {
        auto& comp = ecs().component<entity_info>();
        auto& rec = comp.entity_record(id());
        auto& arc = ecs().archetype(rec.archetypeId);
        arc.populate_default_components(ecs(), id());
        auto id = iId;
        iId = null_entity;
        return id;
    }
}