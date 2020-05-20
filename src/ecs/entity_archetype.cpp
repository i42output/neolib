// entity_archetype.cpp
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
#include <neolib/ecs/entity_archetype.hpp>
#include <neolib/ecs/i_ecs.hpp>

namespace neolib::ecs
{
    entity_archetype::entity_archetype(const entity_archetype_id& aId, const std::string& aName, std::initializer_list<component_id> aComponents) :
        iId{ aId }, iName{ aName }, iComponents{ aComponents }
    {
    }
    
    entity_archetype::entity_archetype(const std::string& aName, std::initializer_list<component_id> aComponents) :
        iId{ neolib::generate_uuid() }, iName{ aName }, iComponents{ aComponents }
    {
    }
    
    entity_archetype::entity_archetype(const entity_archetype& aOther) :
        iId{ aOther.iId }, iName{ aOther.iName }, iComponents{ aOther.iComponents }
    {
    }

    entity_archetype::entity_archetype(entity_archetype&& aOther) :
        iId{ aOther.iId }, iName{ std::move(aOther.iName) }, iComponents{ std::move(aOther.iComponents) }
    {
    }

    const entity_archetype_id& entity_archetype::id() const
    {
        return iId;
    }

    const i_string& entity_archetype::name() const
    {
        return iName;
    }

    const i_set<component_id>& entity_archetype::components() const
    {
        return iComponents;
    }

    i_set<component_id>& entity_archetype::components()
    {
        return iComponents;
    }

    void entity_archetype::populate_default_components(i_ecs&, entity_id)
    {
        // nothing to do.
    }
}