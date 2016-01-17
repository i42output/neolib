// thread.cpp - v3.0
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
#include "io_thread.hpp"
#ifdef _WIN32
#include "win32_message_queue.hpp"
#endif

namespace neolib
{
	io_thread::io_thread(const std::string& aName, bool aAttachToCurrentThread) : 
		thread(aName, aAttachToCurrentThread), iTimerIoService(*this), iNetworkingIoService(*this), iHalted(false)
	{
	}

	namespace
	{
		const std::size_t kMaxiumPollIterations = 256;
	}

	bool io_service::do_io(bool aProcessEvents)
	{
		std::size_t iterationsLeft = kMaxiumPollIterations;
		bool didSome = false;
		while (iterationsLeft-- > 0)
		{
			bool didSomeThisIteration = false;
			if (aProcessEvents)
				didSomeThisIteration = (iThread.process_events() || didSomeThisIteration);
			didSomeThisIteration = (iNativeIoService.poll_one() != 0 || didSomeThisIteration);
			if (!didSomeThisIteration)
				break;
			didSome = true;
		}
		return didSome;
	}

	bool io_thread::do_io(bool aYieldIfNoWork)
	{
		if (iHalted)
			return false;
		bool didSome = false;
		didSome = (iTimerIoService.do_io(false) || didSome);
		didSome = (iNetworkingIoService.do_io(false) || didSome);
		didSome = (process_events() || didSome);
		if (!didSome && aYieldIfNoWork)
		{
			if (have_message_queue())
				message_queue().idle();
			sleep(1);
		}
		return didSome;
	}

	bool io_thread::have_message_queue() const
	{
		return iMessageQueue.get() != 0;
	}

	bool io_thread::have_messages() const
	{
		return have_message_queue() && message_queue().have_message();
	}

	void io_thread::create_message_queue(std::function<bool()> aIdleFunction)
	{
		#ifdef _WIN32
		iMessageQueue = message_queue_pointer(new win32_message_queue(*this, aIdleFunction));
		#endif
	}

	const neolib::message_queue& io_thread::message_queue() const
	{
		if (iMessageQueue.get() == 0)
			throw no_message_queue();
		return *iMessageQueue;
	}

	neolib::message_queue& io_thread::message_queue()
	{
		if (iMessageQueue.get() == 0)
			throw no_message_queue();
		return *iMessageQueue;
	}

	bool io_thread::process_events()
	{
		bool didWork = false;
		while (have_messages())
		{
			message_queue().get_message();
			didWork = true;
		}
		if (iEventProcessor)
			didWork = iEventProcessor() || didWork;
		return didWork;
	}

	void io_thread::set_event_processor(std::function<bool()> aEventProcessor)
	{
		iEventProcessor = aEventProcessor;
	}

	bool io_thread::halted() const
	{
		return iHalted;
	}

	void io_thread::halt()
	{
		iHalted = true;
	}

	void io_thread::task()
	{
		while(!finished())
			do_io();
	}

} // namespace neolib
