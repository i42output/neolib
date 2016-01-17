// i_setting.hpp - v1.0
/*
 *  Copyright (c) 2014 Leigh Johnston.
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

#include "neolib.hpp"
#include "i_reference_counted.hpp"
#include "i_vector.hpp"
#include "i_string.hpp"
#include "i_simple_variant.hpp"

namespace neolib
{
	class i_settings;

	class i_setting : public i_reference_counted
	{
		friend class settings;
	public:
		typedef uint32_t id_type;
	public:
		virtual i_settings& manager() const = 0;
		virtual const id_type id() const = 0;
		virtual const i_string& category() const = 0;
		virtual const i_string& name() const = 0;
		virtual i_simple_variant::type_e type() const = 0;
		virtual const i_simple_variant& value() const = 0;
		virtual void set(const i_simple_variant& aNewValue) = 0;
		virtual const i_simple_variant& new_value() const = 0;
		virtual bool dirty() const = 0;
		virtual bool hidden() const = 0;
	private:
		virtual bool apply_change() = 0;
		virtual bool discard_change() = 0;
	};
}
