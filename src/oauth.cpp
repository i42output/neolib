// oauth.cpp
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
#include "oauth.hpp"

namespace neolib
{
	oauth::oauth(io_thread& OwnerThread, const std::string& aConsumerKey, const std::string& aConsumerSecret, 
		const operation& aRequestTokenOp, const operation& aUserAuthorizationOp, const operation& aAccessTokenOp) : 
		iHttpRequester(OwnerThread), iConsumerKey(aConsumerKey), iConsumerSecret(aConsumerSecret), 
			iRequestTokenOp(aRequestTokenOp), iUserAuthorizationOp(aUserAuthorizationOp), iAccessTokenOp(aAccessTokenOp)
	{
		iHttpRequester.add_observer(*this);
	}

	oauth::~oauth()
	{
		iHttpRequester.remove_observer(*this);
	}

	void oauth::request()
	{
		// TODO
	}

	void oauth::notify_observer(oauth_observer& aObserver, oauth_observer::notify_type aType, const void* aParameter, const void* aParameter2)
	{
		switch(aType)
		{
		case oauth_observer::NotifyStarted:
			aObserver.oauth_request_started(*this);
			break;
		case oauth_observer::NotifyCompleted:
			aObserver.oauth_request_completed(*this);
			break;
		case oauth_observer::NotifyFailure:
			aObserver.oauth_request_failure(*this);
			break;
		}
	}

	void oauth::http_request_started(http& aRequest)
	{
	}

	void oauth::http_request_completed(http& aRequest)
	{
	}

	void oauth::http_request_failure(http& aRequest)
	{
	}
}