// detail_algorithm.hpp
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
#include <cstring>
#include <type_traits>

namespace neolib 
{
	namespace detail
	{
		template <typename InIter, typename OutIter> inline
		OutIter copy_dispatch(InIter first, InIter last, OutIter result, std::tr1::false_type)
		{
			while(first != last)
				*result++ = *first++;
			return result;
		}

		template <typename T> inline
		T* copy_dispatch(const T* first, const T* last, T* result, std::tr1::true_type)
		{
			memmove(result, first, (last - first) * sizeof(T));
			result += (last - first);
			return result;
		}

		template <typename InIter, typename OutIter, typename T> inline
		OutIter copy(InIter first, InIter last, OutIter result, const T&)
		{
			return copy_dispatch(first, last, result, std::tr1::is_scalar<T>::type());
		}
		
		template <typename InIter, typename OutIter> inline
		OutIter copy_backward_dispatch(InIter first, InIter last, OutIter result, std::tr1::false_type)
		{
			while(first != last)
				*--result = *--last;
			return result;
		}

		template <typename T> inline
		T* copy_backward_dispatch(const T* first, const T* last, T* result, std::tr1::true_type)
		{
			memmove(result - (last - first), first, (last - first) * sizeof(T));
			return result - (last - first);
		}

		template <typename BidiIter1, typename BidiIter2, typename T> inline
		BidiIter2 copy_backward(BidiIter1 first, BidiIter1 last, BidiIter2 result, const T&)
		{
			return copy_backward_dispatch(first, last, result, std::tr1::is_scalar<T>::type());
		}

		template <typename RandIter, typename T> inline
		int partition(RandIter a, T x, int p, int r)
		{
			int i = p - 1;
			for (int j = p; j <= r - 1; j++)
			{
				if (a[j] < x)
				{
					i++;
					swap(a[i], a[j]);
				}
			}
			swap(a[i + 1], a[r]);
			return i + 1;
		}

		template <typename RandIter> inline
		void quicksort(RandIter a, int p, int r)
		{
			if (p < r)
			{
				int q = partition(a, a[r], p, r);
				quicksort(a, p, q - 1);
				quicksort(a, q + 1, r);
			}
		}

		template <typename RandIter, typename T, typename Compare> inline
		int partition(RandIter a, T x, int p, int r, Compare comp)
		{
			int i = p - 1;
			for (int j = p; j <= r - 1; j++)
			{
				if (comp(a[j], x))
				{
					i++;
					swap(a[i], a[j]);
				}
			}
			swap(a[i + 1], a[r]);
			return i + 1;
		}

		template <typename RandIter, typename Compare> inline
		void quicksort(RandIter a, int p, int r, Compare comp)
		{
			if (p < r)
			{
				int q = partition(a, a[r], p, r, comp);
				quicksort(a, p, q - 1, comp);
				quicksort(a, q + 1, r, comp);
			}
		}

		template <typename BidiIter> inline
		void bubblesort(BidiIter first, BidiIter last)
		{
			if (first == last) // empty
				return;
			--last;
			if (first == last) // just one
				return;
			bool swapped = false;
			do
			{
				BidiIter nextnext = first;
				BidiIter next = nextnext++;
				while(next != last)
				{
					if (!(*next < *nextnext))
					{
						swap(*next, *nextnext);
						swapped = true;
					}
					next++;
					nextnext++;
				}
				--last;
			} while (swapped && last != first);
		}

		template <typename BidiIter, typename Compare> inline
		void bubblesort(BidiIter first, BidiIter last, Compare comp)
		{
			if (first == last) // empty
				return;
			--last;
			if (first == last) // just one
				return;
			bool swapped = false;
			do
			{
				BidiIter nextnext = first;
				BidiIter next = nextnext++;
				while(next != last)
				{
					if (!comp(*next,*nextnext))
					{
						swap(*next, *nextnext);
						swapped = true;
					}
					next++;
					nextnext++;
				}
				--last;
			} while (swapped && last != first);
		}
	}
}
