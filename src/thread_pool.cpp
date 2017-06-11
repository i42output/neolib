// thread_pool.cpp
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
#include <neolib/thread_pool.hpp>

namespace neolib
{
	thread_pool::thread_pool()
	{
	}

	void thread_pool::reserve(std::size_t aMaxThreads)
	{
		/* todo */
	}

	std::size_t thread_pool::active_threads() const
	{
		/* todo */
		return 0;
	}

	std::size_t thread_pool::available_threads() const
	{
		/* todo */
		return 0;
	}

	std::size_t thread_pool::max_threads() const
	{
		/* todo */
		return 0;
	}

	void thread_pool::start(i_task& aTask, int32_t aPriority)
	{
		/* todo */
	}

	void thread_pool::start(std::shared_ptr<i_task> aTask, int32_t aPriority)
	{
		/* todo */
	}

	bool thread_pool::tryStart(i_task& aTask)
	{
		/* todo */
		return false;
	}

	bool thread_pool::tryStart(std::shared_ptr<i_task> aTask)
	{
		/* todo */
		return false;
	}

	std::future<void> thread_pool::run(std::function<void()> aFunction)
	{
		/* todo */
		std::future<void> result;
		return std::move(result);
	}

	thread_pool& thread_pool::default_thread_pool()
	{
		static thread_pool sDefaultThreadPool;
		return sDefaultThreadPool;
	}
}
