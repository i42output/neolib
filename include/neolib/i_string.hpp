// i_string.hpp - v1.0
/*
 *  Copyright (c) 2007-present, Leigh Johnston.  All Rights Reserved.
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
#include <string>
#include <iostream>
#include "i_sequence_container.hpp"

namespace neolib
{
	class i_string : public i_sequence_container<char, i_random_access_const_iterator<char>, i_random_access_iterator<char>, false>
	{
	private:
		typedef i_sequence_container<char, i_random_access_const_iterator<char>, i_random_access_iterator<char> > base;
	public:
		typedef base::size_type size_type;
	public:
		virtual i_string& operator=(const i_string& aOther) = 0;
		i_string& operator=(const std::string& aOther) { assign(aOther); return *this; }
	public:
		size_type length() const { return size(); }
		virtual const char* c_str() const = 0;
		virtual const char& operator[](size_type aIndex) const = 0;
		virtual char& operator[](size_type aIndex) = 0;
		void assign(const std::string& aSource) { assign(aSource.c_str(), aSource.size()); }
		virtual void assign(const char* aSource, size_type aSourceLength) = 0;
		std::string to_std_string() const { return std::string(c_str(), size()); }
	};

	inline std::ostream& operator<<(std::ostream& aStream, const i_string& aString)
	{
		aStream << aString.to_std_string();
		return aStream;
	}

	inline std::istream& operator>>(std::istream& aStream, i_string& aString)
	{
		std::string temp;
		aStream >> temp;
		aString.assign(temp.c_str(), temp.size());
		return aStream;
	}

	inline bool operator==(const i_string& lhs, const i_string& rhs)
	{
		return lhs.size() == rhs.size() && std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
	}

	inline bool operator!=(const i_string& lhs, const i_string& rhs)
	{
		return lhs.size() != rhs.size() || std::strcmp(lhs.c_str(), rhs.c_str()) != 0;
	}

	inline bool operator<(const i_string& lhs, const i_string& rhs)
	{
		return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
	}
}
