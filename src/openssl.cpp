// openssl.cpp
/*
 *  Copyright (c) 2017 Leigh Johnston.
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
#include <openssl/opensslv.h>
#include <openssl/rand.h>
#include <neolib/openssl.hpp>

#if OPENSSL_VERSION_NUMBER < 0x10100000
static_assert(false, "OpenSSL version too old");
#endif

namespace neolib
{
	openssl::openssl()
	{
	}

	openssl::~openssl()
	{
	}

	openssl& openssl::instance()
	{
		static openssl sInstance;
		return sInstance;
	}

	bool openssl::generate_key(uint8_t* aKeyBuffer, std::size_t aKeySize)
	{
		while (need_entropy())
			generate_entropy();
		return RAND_bytes(aKeyBuffer, static_cast<int>(aKeySize)) == 1;
	}

	bool openssl::need_entropy() const
	{
		return RAND_status() == 0;
	}

	void openssl::generate_entropy()
	{
		thread_local std::random_device tRandomDevice;
		thread_local std::random_device::result_type tSeedBuffer[SEED_BUFFER_SIZE];
		for (std::size_t n = 0; n < SEED_BUFFER_SIZE; ++n)
			tSeedBuffer[n] = tRandomDevice();
		RAND_seed(tSeedBuffer, sizeof(tSeedBuffer));
	}
}
