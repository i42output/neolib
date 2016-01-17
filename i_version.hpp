// i_version.hpp - v1.0
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
#include <cstdint>
#include <iostream>
#include <sstream>
#include "i_string.hpp"

namespace neolib
{
	class i_version
	{
	public:
		virtual uint32_t major() const = 0;
		virtual uint32_t minor() const = 0;
		virtual uint32_t maintenance() const = 0;
		virtual uint32_t build() const = 0;
		virtual const i_string& name() const = 0;
	};

	inline std::ostream& operator<<(std::ostream& aStream, const i_version& aVersion)
	{
		aStream << aVersion.major() << "." << aVersion.minor() << "." << aVersion.maintenance();
		if (aVersion.build() != 0)
			aStream << "." << aVersion.build();
		if (!aVersion.name().empty())
			aStream << " " << aVersion.name();
		return aStream;
	}

	inline std::string to_string(const i_version& aVersion)
	{
		std::stringstream oss;
		oss << aVersion;
		return oss.str();
	}
}
