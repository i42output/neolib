// queue.hpp - v1.4
/*
 *  Copyright (c) 2012-present, Leigh Johnston.
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
#include <list>
#include <vector>
#include "interlockable.hpp"
#include "event.hpp"

namespace neolib
{
	template <typename QueueItem>
	class queue : public interlockable
	{
		// types
	public:
		typedef QueueItem value_type;
		class sink
		{
		public:
			virtual void from_queue(queue& aQueue, QueueItem& aItem) = 0;
		};
		struct no_sink : std::logic_error { no_sink() : std::logic_error("neolib::queue::no_sink") {} };
	private:
		class entry
		{
			// types
		public:
			enum type_e { Send, Post };
			// construction
		public:
			entry(const QueueItem& aItem, type_e aType) : iItem(aItem), iType(aType), iEvent(0), iFrom(0) {}
			entry(const QueueItem& aItem, type_e aType, const event& aEvent) : iItem(aItem), iType(aType), iEvent(&aEvent), iFrom(0) {}
			entry(const QueueItem& aItem, type_e aType, const interlockable& aFrom) : iItem(aItem), iType(aType), iEvent(0), iFrom(&aFrom) {}
			entry(const QueueItem& aItem, type_e aType, const event& aEvent, const interlockable& aFrom) : iItem(aItem), iType(aType), iEvent(&aEvent), iFrom(&aFrom) {}
			// operations
		public:
			const QueueItem& item() const { return iItem; }
			QueueItem& item() { return iItem; }
			type_e type() const { return iType; }
			const interlockable* from() const { return iFrom; }
			void signal() const { if (iEvent != 0) iEvent->signal_one(); }
			// attributes
		private:
			QueueItem iItem;
			type_e iType;
			const event* iEvent;
			const interlockable* iFrom;
		};
		typedef std::list<entry> container_type;
		typedef typename container_type::const_iterator const_iterator;
		typedef typename container_type::iterator iterator;
		typedef std::vector<iterator> working_list;
		// construction
	public:
		queue() :  iSink(0) {}
		queue(sink& aSink) : iSink(&aSink) {}
		// operations
	public:
		void wait()
		{
			return iNewItemEvent.wait();
		}
		bool wait(uint32_t aTimeout_ms)
		{
			return iNewItemEvent.wait(aTimeout_ms);
		}
		void loop(bool (*aYieldProc)() = 0)
		{
			if (iSink == 0)
				throw std::logic_error("neolib::queue::loop");
			while (!aYieldProc || !aYieldProc())
			{
				wait();
				process_queue();
			}
		}
		void loop(uint32_t aTimeout_ms, bool (*aYieldProc)() = 0)
		{
			if (iSink == 0)
				throw std::logic_error("neolib::queue::loop");
			while ((!aYieldProc || !aYieldProc()) && wait(aTimeout_ms))
				process_queue();
		}
		void send(const QueueItem& aItem)
		{
			send(iProcessedItemEvent, aItem);
		}
		bool send(const QueueItem& aItem, uint32_t aTimeout_ms)
		{
			return send(iProcessedItemEvent, aItem, aTimeout_ms);
		}
		void send(const event& aEvent, const QueueItem& aItem)
		{
			lock();
			iItems.push_back(entry(aItem, entry::Send, aEvent));
			unlock();
			iNewItemEvent.signal();
			return aEvent.wait();
		}
		bool send(const event& aEvent, const QueueItem& aItem, uint32_t aTimeout_ms)
		{
			lock();
			iItems.push_back(entry(aItem, entry::Send, aEvent));
			unlock();
			iNewItemEvent.signal();
			return aEvent.timed_wait(aTimeout_ms);
		}
		void interlocked_send(const interlockable& aOther, const QueueItem& aItem)
		{
			interlocked_send(aOther, iProcessedItemEvent, aItem);
		}
		bool interlocked_send(const interlockable& aOther, const QueueItem& aItem, uint32_t aTimeout_ms)
		{
			return interlocked_send(aOther, iProcessedItemEvent, aItem, aTimeout_ms);
		}
		void interlocked_send(const interlockable& aOther, const event& aEvent, const QueueItem& aItem)
		{
			interlock_acquire(aOther);
			lock();
			iItems.push_back(entry(aItem, entry::Send, aEvent, aOther));
			unlock();
			iNewItemEvent.signal_one();
			interlock_release(aOther);
			aEvent.wait();
		}
		bool interlocked_send(const interlockable& aOther, const event& aEvent, const QueueItem& aItem, uint32_t aTimeout_ms)
		{
			interlock_acquire(aOther);
			lock();
			iItems.push_back(entry(aItem, entry::Send, aEvent, aOther));
			unlock();
			iNewItemEvent.signal_one();
			interlock_release(aOther);
			return aEvent.wait(aTimeout_ms);
		}
		void post(const QueueItem& aItem)
		{
			lock();
			iItems.push_back(entry(aItem, entry::Post));
			unlock();
			iNewItemEvent.signal_one();
		}
		QueueItem& next()
		{
			neolib::lock lock(*this);
			iterator nextEntry = next_available();
			iWorkingList.push_back(nextEntry);
			return nextEntry->item();
		}
		void pop_next()
		{
			neolib::lock lock(*this);
			iterator nextEntry = iWorkingList.back();
			iWorkingList.pop_back();
			if (nextEntry->type() == entry::Send)
				nextEntry->signal();
			iItems.erase(nextEntry);
		}
		bool any() const
		{
			neolib::lock lock(*this);
			return next_available() != iItems.end();
		}
		void clear()
		{
			neolib::lock lock(*this);
			iItems.clear();
		}
		// implementation
	private:
		const_iterator next_available() const
		{
			const_iterator nextAvailable;
			if (iWorkingList.empty())
				nextAvailable = iItems.begin();
			else
			{
				nextAvailable = iWorkingList.back();
				++nextAvailable;
			}
			return nextAvailable;
		}
		iterator next_available()
		{
			iterator nextAvailable;
			if (iWorkingList.empty())
				nextAvailable = iItems.begin();
			else
			{
				nextAvailable = iWorkingList.back();
				++nextAvailable;
			}
			return nextAvailable;
		}
		void process_queue()
		{
			if (iSink == 0)
				throw no_sink();
			while (any())
			{
				QueueItem& nextItem = next();
				iSink->from_queue(*this, nextItem);
				pop_next();
			}
		}
		// from interlockable
		virtual bool purge(const interlockable& aOther)
		{
			if (iSink == 0)
				throw std::logic_error("neolib::queue::purge");
			neolib::lock lock(*this);
			for (working_list::const_iterator i = iWorkingList.begin(); i != iWorkingList.end(); ++i)
			{
				entry& theEntry = **i;
				if (theEntry.type() == entry::Send && theEntry.from() == &aOther)
					return false;
			}
			if (wait(event::ShortTimeout_ms))
				process_queue();
			return true;
		}
		// attributes
	private:
		container_type iItems;
		working_list iWorkingList;
		event iNewItemEvent;
		event iProcessedItemEvent;
		sink* iSink;
	};
}
