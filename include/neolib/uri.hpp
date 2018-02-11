// uri.hpp
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
#include <unordered_set>
#include <boost/optional.hpp>

namespace neolib
{
	class uri_authority
	{
	public:
		typedef std::string user_information_type;
		typedef std::string host_type;
		typedef uint16_t port_type;
		typedef boost::optional<user_information_type> optional_user_information;
		typedef boost::optional<host_type> optional_host;
		typedef boost::optional<port_type> optional_port;
	public:
		uri_authority();
		uri_authority(const std::string& aAuthority);
	public:
		const optional_user_information& user_information() const;
		const optional_host& host() const;
		const optional_port& port() const;
	private:
		optional_user_information iUserInformation;
		optional_host iHost;
		optional_port iPort;
	};

	class uri
	{
	public:
		uri();
		uri(const std::string& aUri);
	public:
		std::string to_string() const;
		const std::string& scheme() const;
		const uri_authority& authority() const;
		const std::string& path() const;
		const std::string& query() const;
		const std::string& fragment() const;
		void set_scheme(const std::string& aScheme);
		void set_authority(const uri_authority& aAuthority);
		void set_path(const std::string& aPath);
		void set_query(const std::string& aQuery);
		void set_fragment(const std::string& aFragment);
	public:
		static std::string escaped(const std::string& aString);
		static std::string unescaped(const std::string& String);
	private:
		void parse_authority(const std::string& aRest);
		std::string parse_path(const std::string& aRest);
		std::string parse_query(const std::string& aRest);
		std::string parse_fragment(const std::string& aRest);
		std::string parse_scheme(const std::string& aRest);
	private:
		std::string iScheme;
		uri_authority iAuthority;
		std::string iPath;
		std::string iQuery;
		std::string iFragment;
	};

	template <typename Elem, typename Traits>
	inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const uri_authority& aUriAuthority)
	{
		if (aUriAuthority.user_information() != boost::none)
			aStream << uri::escaped(*aUriAuthority.user_information()) << "@";
		if (aUriAuthority.host() != boost::none)
			aStream << uri::escaped(*aUriAuthority.host());
		if (aUriAuthority.port() != boost::none)
			aStream << ":" << *aUriAuthority.port();
		return aStream;
	}

	template <typename Elem, typename Traits>
	inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const uri& aUri)
	{
		aStream << uri::escaped(aUri.scheme()) << "://" << aUri.authority() << "/" << uri::escaped(aUri.path());
		if (!aUri.query().empty())
			aStream << "?" << uri::escaped(aUri.query());
		if (!aUri.fragment().empty())
			aStream << "#" << uri::escaped(aUri.fragment());
		return aStream;
	}
}
