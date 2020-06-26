// setting.hpp
/*
 *  Copyright (c) 2007 Leigh Johnston.
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
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/string.hpp>
#include <neolib/plugin/simple_variant.hpp>
#include <neolib/app/i_setting.hpp>
#include <neolib/app/i_settings.hpp>
#include <neolib/app/setting_constraints.hpp>

namespace neolib
{
    class setting : public reference_counted<i_setting>
    {
        friend class settings;
    public:
        typedef id_type key_type;
    public:
        setting(i_settings& aManager, id_type aId, const i_string& aCategory, const i_string& aName, simple_variant_type aType, const i_setting_constraints& aConstraints = setting_constraints{}, const simple_variant& aValue = simple_variant{}, bool aHidden = false) :
            iManager{ aManager }, iId{ aId }, iCategory{ aCategory }, iName{ aName }, iType{ aType }, iConstraints{ aConstraints }, iValue{ aValue }, iHidden{ aHidden } {}
        setting(const i_setting& aSetting) :
            iManager{ aSetting.manager() }, iId{ aSetting.id() }, iCategory{ aSetting.category() }, iName{ aSetting.name() }, iType{ aSetting.type() }, iConstraints{ aSetting.constraints() }, iValue{ aSetting.value() }, iHidden{ aSetting.hidden() } {}
    public:
        i_settings& manager() const override { return iManager; }
        id_type id() const override { return iId; }
        string const& category() const override { return iCategory; }
        string const& name() const override { return iName; }
        simple_variant_type type() const override { return iType; }
        setting_constraints const& constraints() const override { return iConstraints; }
        simple_variant const& value() const override { return iValue; }
        using i_setting::set;
        void set(const i_simple_variant& aNewValue) override;
        simple_variant const& new_value() const override { if (!iNewValue.empty()) return iNewValue; return iValue; }
        bool dirty() const override { return !iNewValue.empty(); }
        bool hidden() const override { return iHidden; }
    public:
        operator key_type() const { return key_type(iId); }
    private:
        bool apply_change() override;
        bool discard_change() override;
    private:
        i_settings& iManager;
        id_type iId;
        string iCategory;
        string iName;
        simple_variant_type iType;
        setting_constraints iConstraints;
        simple_variant iValue;
        simple_variant iNewValue;
        bool iHidden;
    };
}
