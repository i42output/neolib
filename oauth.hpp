// oauth.h
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

#pragma once

#include "neolib.hpp"
#include <vector>
#include <string>
#include <utility>
#include "observable.hpp"
#include "string_utils.hpp"
#include "optional.hpp"
#include "io_thread.hpp"
#include "http.hpp"

namespace neolib
{
	class oauth_observer
	{
		friend class oauth;
	private:
		virtual void oauth_request_started(oauth& aRequest) = 0;
		virtual void oauth_request_completed(oauth& aRequest) = 0;
		virtual void oauth_request_failure(oauth& aRequest) = 0;
	public:
		enum notify_type { NotifyStarted, NotifyCompleted, NotifyFailure };
	};

	class oauth : public observable<oauth_observer>, private i_http_observer
	{
		// types
	public:
		typedef std::pair<http::type_e, std::string> operation;

		// construction
	public:
		oauth(io_thread& OwnerThread, const std::string& aConsumerKey, const std::string& aConsumerSecret, const operation& aRequestTokenOp, const operation& aUserAuthorizationOp, const operation& aAccessTokenOp);
		virtual ~oauth();

		// operations
	public:
		void request();

		// implementation
	private:
		// from observable<oauth_observer>
		virtual void notify_observer(oauth_observer& aObserver, oauth_observer::notify_type aType, const void* aParameter, const void* aParameter2);
		// from i_http_observer
		virtual void http_request_started(http& aRequest);
		virtual void http_request_completed(http& aRequest);
		virtual void http_request_failure(http& aRequest);

		// attributes
	private:
		http iHttpRequester;
		std::string iConsumerKey;
		std::string iConsumerSecret;
		operation iRequestTokenOp;
		operation iUserAuthorizationOp;
		operation iAccessTokenOp;
	};
}
