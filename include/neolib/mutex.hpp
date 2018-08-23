// mutex.hpp
/*
 *  Copyright (c) 2018-present, Leigh Johnston.
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
#include <mutex>
#include "lifetime.hpp"

namespace neolib
{
	struct null_mutex
	{
		void lock() {}
		void unlock() noexcept {}
		bool try_lock() { return true; }
	};

	template<class Mutex>
	class destroyable_mutex_lock_guard
	{
	public:
		struct lock_failure : std::logic_error { lock_failure() : std::logic_error("neolib::destroyable_mutex_lock_guard::lock_failure") {} };
	public:
		typedef Mutex mutex_type;
	public:
		explicit destroyable_mutex_lock_guard(Mutex& aMutex) :
			iMutex{ aMutex }, iMutexDestroyed{ aMutex }
		{
			iMutex.lock();
		}
		template <typename RetryDuration>
		destroyable_mutex_lock_guard(Mutex& aMutex, const RetryDuration& aRetryDuration, uint32_t aMaxRetries) :
			iMutex{ aMutex }, iMutexDestroyed{ aMutex }
		{
			uint32_t attempt = 0;
			while (!iMutex.try_lock())
			{
				if (aMaxRetries == 0 || ++attempt < aMaxRetries)
					std::this_thread::sleep_for(aRetryDuration);
				else
					throw lock_failure();
			}
		}
		destroyable_mutex_lock_guard(Mutex& aMutex, std::adopt_lock_t) :
			iMutex{ aMutex }, iMutexDestroyed{ aMutex }
		{
		}
		~destroyable_mutex_lock_guard() noexcept
		{
			if (!iMutexDestroyed)
				iMutex.unlock();
		}
		destroyable_mutex_lock_guard(const destroyable_mutex_lock_guard&) = delete;
		destroyable_mutex_lock_guard& operator=(const destroyable_mutex_lock_guard&) = delete;
	private:
		mutex_type& iMutex;
		destroyed_flag iMutexDestroyed;
	};
}
