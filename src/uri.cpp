// uri.cpp
/*
 *  Copyright (c) 2012 Leigh Johnston.
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

#include "neolib.hpp"
#include "vecarray.hpp"
#include "string_utils.hpp"
#include "uri.hpp"

namespace neolib
{
	uri_authority::uri_authority()
	{
	}

	uri_authority::uri_authority(const std::string& aAuthority)
	{
	}
	
	const uri_authority::optional_user_information& uri_authority::user_information() const
	{
		return iUserInformation;
	}

	const uri_authority::optional_host& uri_authority::host() const
	{
		return iHost;
	}

	const uri_authority::optional_port& uri_authority::port() const
	{
		return iPort;
	}

	uri::uri()
	{
	}

	uri::uri(const std::string& aUri)
	{
		parse_authority(parse_path(parse_query(parse_fragment(parse_scheme(aUri)))));
	}

	const std::string& uri::scheme() const
	{
		return iScheme;
	}

	const uri_authority& uri::authority() const
	{
		return iAuthority;
	}

	const std::string& uri::path() const
	{
		return iPath;
	}

	const std::string& uri::query() const
	{
		return iQuery;
	}

	const std::string& uri::fragment() const
	{
		return iFragment;
	}

	void uri::set_scheme(const std::string& aScheme)
	{
		iScheme = aScheme;
	}

	void uri::set_authority(const uri_authority& aAuthority)
	{
		iAuthority = aAuthority;
	}

	void uri::set_path(const std::string& aPath)
	{
		iPath = aPath;
	}

	void uri::set_query(const std::string& aQuery)
	{
		iQuery = aQuery;
	}

	void uri::set_fragment(const std::string& aFragment)
	{
		iFragment = aFragment;
	}

	void uri::parse_authority(const std::string& aRest)
	{
		iAuthority = uri_authority(aRest);
	}

	std::string uri::parse_path(const std::string& aRest)
	{
		neolib::vecarray<std::pair<std::string::const_iterator, std::string::const_iterator>, 2> bits;
		std::string sep("/");
		neolib::tokens(aRest.begin(), aRest.end(), sep.begin(), sep.end(), bits, 2, false, false);
		if (bits.size() == 2)
			iPath = std::string(bits[1].first, aRest.end());
		return std::string(bits.front().first, bits.front().second);
	}

	std::string uri::parse_query(const std::string& aRest)
	{
		neolib::vecarray<std::string, 2> bits;
		neolib::tokens(aRest, std::string("?"), bits, 2, false, false);
		if (bits.size() == 2)
			iQuery = bits[1];
		return bits.front();
	}

	std::string uri::parse_fragment(const std::string& aRest)
	{
		neolib::vecarray<std::string, 2> bits;
		neolib::tokens(aRest, std::string("#"), bits, 2, false, false);
		if (bits.size() == 2)
			iFragment = bits[1];
		return bits.front();
	}

	std::string uri::parse_scheme(const std::string& aRest)
	{
		neolib::vecarray<std::string, 2> bits;
		neolib::tokens(aRest, std::string("://"), bits, 2, false, true);
		if (bits.size() == 2)
			iScheme = bits[0];
		return bits.back();
	}
}
