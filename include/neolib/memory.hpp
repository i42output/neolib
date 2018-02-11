// memory.hpp v2.4.1
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

/* WARNING: The classes present here are not a substitute for any equivalent std:: 
 * classes available on your platform which you should be using instead.  They exist here
 * either for technical reasons or for when there is no standard library available.
  */

#pragma once

#include "neolib.hpp"
#include <new>
#include <stdexcept>
#include <memory>
#include "detail_memory.hpp"

namespace neolib
{
	template <typename InIter, typename OutIter> inline
	OutIter uninitialized_copy(InIter first, InIter last, OutIter result)
	{
		return detail::uninitialized_copy(first, last, result, *result);
	}

	template <typename T, std::size_t ChunkSize = 8 * 1024 - 16, std::size_t Instance = 0>
	class chunk_allocator
	{
	public:
		typedef T value_type;
		typedef T* pointer;
		typedef T& reference;
		typedef const T* const_pointer;
		typedef const T& const_reference;
		typedef std::size_t size_type;
		typedef ptrdiff_t difference_type;

	// implementation
	private:
		struct link { link* iNext; };
		struct chunk
		{
			enum { size = ChunkSize};
			alignas(T) char iMem[size];
			chunk* iNext;
		};
		struct pool
		{
			size_type iElementSize;
			chunk* iChunks;
			link* iHead;
			pool() : iElementSize(sizeof(T) < sizeof(link) ? sizeof(link) : sizeof(T)), iChunks(0), iHead(0) {}
			~pool()
			{
				chunk* n = iChunks;
				while(n)
				{
					chunk* p = n;
					n = n->iNext;
					delete p;
				}
			}
			void* allocate()
			{
				if (iHead == 0)
					grow();
				link* p = iHead;
				iHead = p->iNext;
				return p;
			}
			void deallocate(void* aObject)
			{
				link* p = reinterpret_cast<link*>(aObject);
				p->iNext = iHead;
				iHead = p;
			}
			void grow()
			{
				chunk* n = new chunk;
				n->iNext = iChunks;
				iChunks = n;

				const std::size_t nelem = chunk::size / iElementSize;
				char* start = n->iMem;
				char* last = &start[(nelem-1) * iElementSize];
				for (char* p = start; p < last; p+= iElementSize)
					reinterpret_cast<link*>(p)->iNext = reinterpret_cast<link*>(p+iElementSize);
				reinterpret_cast<link*>(last)->iNext = 0;
				iHead = reinterpret_cast<link*>(start);
			}
		};

	// construction
	public:
		chunk_allocator() {}
		chunk_allocator(const chunk_allocator& rhs) {}
		template <typename U>
		chunk_allocator(const chunk_allocator<U, ChunkSize, Instance>& /*rhs*/) {}
		~chunk_allocator() {}

	// operations
	public:
		pointer allocate(size_type aCount = 1)
		{
			if (aCount != 1)
				throw std::bad_alloc();
			return reinterpret_cast<pointer>(sPool.allocate());
		}

		void deallocate(pointer aObject, size_type aCount = 1)
		{
			if (aCount != 1)
				throw std::logic_error("neolib::chunk_allocator::deallocate");
			sPool.deallocate(aObject);
		}

		void construct(pointer aObject, const_reference val)
		{
			new (aObject) T(val);
		}
		
		void destroy(pointer aObject)
		{
			aObject;
			aObject->~T();
		}

		template <typename U>
		struct rebind
		{
			typedef chunk_allocator<U, ChunkSize, Instance> other;
		};

		// this should really return 1 but popular implementations assume otherwise
		size_type max_size() const { return std::allocator<T>().max_size(); }

		bool operator==(const chunk_allocator&) const { return true; }

	// attributes
	private:
		static pool sPool;
	};

	template <typename T, std::size_t ChunkSize, std::size_t Instance>
	typename chunk_allocator<T, ChunkSize, Instance>::pool chunk_allocator<T, ChunkSize, Instance>::sPool;

	template <typename T, std::size_t N, std::size_t Instance = 0>
	class reserve_allocator
	{
	public:
		typedef T value_type;
		typedef T* pointer;
		typedef T& reference;
		typedef const T* const_pointer;
		typedef const T& const_reference;
		typedef std::size_t size_type;
		typedef ptrdiff_t difference_type;

	// implementation
	private:
		struct link { link* iNext; };
		struct block
		{
			size_type iElementSize;
			union
			{
				union
				{
					alignas(T) char a[sizeof(T)];
					alignas(link) char b[sizeof(link)];
				} iBuffer[N];
			} iBuffer;
			char* iMem;
			link* iHead;
			block() : iElementSize(sizeof(T) < sizeof(link) ? sizeof(link) : sizeof(T)), iMem(reinterpret_cast<char*>(iBuffer.iBuffer)), iHead(reinterpret_cast<link*>(iMem)) { init(); }
			void init()
			{
				char* start = iMem;
				char* last = &start[(N-1) * iElementSize];
				for (char* p = start; p < last; p+= iElementSize)
					reinterpret_cast<link*>(p)->iNext = reinterpret_cast<link*>(p+iElementSize);
				reinterpret_cast<link*>(last)->iNext = 0;
			}
			void* allocate()
			{
				if (iHead == 0)
					throw std::bad_alloc("neolib::reserve_allocator::allocate() when full");
				link* p = iHead;
				iHead = p->iNext;
				return p;
			}
			void deallocate(void* aObject)
			{
				link* p = reinterpret_cast<link*>(aObject);
				p->iNext = iHead;
				iHead = p;
			}
		};

	// construction
	public:
		reserve_allocator() {}
		reserve_allocator(const reserve_allocator& rhs) {}
		template <typename U>
		reserve_allocator(const reserve_allocator<U, N, Instance>& rhs) {}
		~reserve_allocator() {}

	// operations
	public:
		T* allocate(size_type aCount = 1)
		{
			if (aCount != 1)
				throw std::bad_alloc();
			return reinterpret_cast<T*>(sBlock.allocate());
		}

		void deallocate(T* aObject, size_type aCount = 1)
		{
			if (aCount != 1)
				throw std::logic_error("neolib::reserve_allocator::deallocate");
			sBlock.deallocate(aObject);
		}

		void construct(pointer aObject, const_reference val)
		{
			new (aObject) T(val);
		}

		void destroy(pointer aObject)
		{
			aObject->~T();
		}

		template <typename U>
		struct rebind
		{
			typedef reserve_allocator<U, N, Instance> other;
		};

		// this should really return 1 but popular implementations assume otherwise
		size_type max_size() const { return std::allocator<T>().max_size(); }

	// attributes
	private:
		static block sBlock;
	};

	template <typename T, std::size_t N, std::size_t Instance>
	typename reserve_allocator<T, N, Instance>::block reserve_allocator<T, N, Instance>::sBlock;
}
