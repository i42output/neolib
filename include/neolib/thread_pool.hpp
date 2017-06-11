// thread_pool.hpp
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

#pragma once

#include "neolib.hpp"
#include <memory>
#include <vector>
#include <future>
#include "i_task.hpp"

namespace neolib
{
	class thread_pool
	{
	public:
		thread_pool();
	public:
		void reserve(std::size_t aMaxThreads);
		std::size_t active_threads() const;
		std::size_t available_threads() const;
		std::size_t max_threads() const;
	public:
		void start(i_task& aTask, int32_t aPriority = 0);
		void start(std::shared_ptr<i_task> aTask, int32_t aPriority = 0);
		bool tryStart(i_task& aTask);
		bool tryStart(std::shared_ptr<i_task> aTask);
		std::future<void> run(std::function<void()> aFunction);
		template <typename T>
		std::future<T> run(std::function<T()> aFunction);
	public:
		static thread_pool& default_thread_pool();
	private:
		std::vector<std::unique_ptr<i_thread>> iThreads;
		std::vector<std::shared_ptr<i_task>> iTasks;
	};

	template <typename T>
	std::future<T> thread_pool::run(std::function<T()> aFunction)
	{
		/* todo */
		std::future<T> result;
		return std::move(result);
	}
}
