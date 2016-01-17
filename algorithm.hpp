// algorithm.hpp
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

/* WARNING: The algorithms present here are not a substitute for any equivalent std:: 
 * algorithms available on your platform which you should be using instead.  They exist here
 * either for technical reasons or for when there is no standard library available.
  */

#pragma once

#include "neolib.hpp"
#include <algorithm>
#include <utility>
#include "detail_algorithm.hpp"

namespace neolib 
{
	template <typename InIter, typename Func> inline
	Func for_each(InIter first, InIter last, Func f)
	{
		for(; first != last; first++)
			f(*first);
		return f;
	}

	template <typename InIter1, typename InIter2, typename Func> inline
	Func for_each(InIter1 first1, InIter1 last1, InIter2 first2, Func f)
	{
		for(; first1 != last1; first1++, first2++)
			f(*first1, *first2);
		return f;
	}

	template <typename InIter, typename OutIter> inline
	OutIter copy(InIter first, InIter last, OutIter result)
	{
		return detail::copy(first, last, result, *result);
	}
	
	template <typename BidiIter1, typename BidiIter2> inline
	BidiIter2 copy_backward(BidiIter1 first, BidiIter1 last, BidiIter2 result)
	{
		return detail::copy_backward(first, last, result, *result);
	}

	template <typename RandIter> inline
	void sort(RandIter first, RandIter last)
	{
		int n = last - first;
		if (n > 0)
			detail::quicksort(first, 0, n - 1);
	}

	template <typename RandIter, typename Compare> inline
	void sort(RandIter first, RandIter last, Compare comp)
	{
		int n = last - first;
		if (n > 0)
			detail::quicksort(first, 0, n - 1, comp);
	}

	template <typename T>
	class bresenham_counter
	{
		/* operator T() returns x[0..N-1] = 0 .. R, i.e. x[n] = (R / (N-1)) * n, without
		using floating point or multiplication/division each iteration */
	public:
		bresenham_counter() {}
		bresenham_counter(T range, T number) : 
			dx(number-1), 
			dy(dx > 0 ? range % dx : 0), 
			d(2*dy - dx), incrE(2*dy), incrNE(2*(dy-dx)), 
			incrCounter(dx > 0 ? range / dx : 0), 
			incrCounterPlus1(dx > 0 ? incrCounter+1 : 0),
			counter(0) {}
		bresenham_counter(T rangeStart, T rangeEnd, T number) : 
			dx(number-1), 
			dy(dx > 0 ? rangeEnd > rangeStart ? 
				(rangeEnd - rangeStart) % dx : (rangeStart - rangeEnd) % dx : 0), 
			d(2*dy - dx), incrE(2*dy), incrNE(2*(dy-dx)), 
			incrCounter(dx > 0 ? (rangeEnd - rangeStart) / dx : 0), 
			incrCounterPlus1(dx > 0 ? rangeEnd > rangeStart ? 
				incrCounter+1 : incrCounter-1 : 0),
			counter(rangeStart) {}
		void init(T range, T number)
		{
			dx = number-1; 
			dy = dx > 0 ? range % dx : 0; 
			d = 2*dy - dx;
			incrE = 2*dy;
			incrNE = 2*(dy-dx); 
			incrCounter = dx > 0 ? range / dx : 0; 
			incrCounterPlus1 = dx > 0 ? incrCounter+1 : 0;
			counter = 0;
		}
		void init(T rangeStart, T rangeEnd, T number)
		{
			dx = number-1;
			dy = dx > 0 ? rangeEnd > rangeStart ? 
				(rangeEnd - rangeStart) % dx : (rangeStart - rangeEnd) % dx : 0;
			d = 2*dy - dx;
			incrE = 2*dy;
			incrNE = 2*(dy-dx);
			incrCounter = dx > 0 ? (rangeEnd - rangeStart) / dx : 0;
			incrCounterPlus1 = dx > 0 ? rangeEnd > rangeStart ? 
				incrCounter+1 : incrCounter-1 : 0;
			counter = rangeStart;
		}
		operator T()
		{
			if (d <= 0)
			{
				d += incrE;
				T v = counter;
				counter += incrCounter;
				return v;
			}
			else
			{
				d += incrNE;
				T v = counter;
				counter += incrCounterPlus1;
				return v;
			}
		}
		T operator()()
		{
			return operator T();
		}
	private:
		T dx, dy, d, incrE, incrNE;
		T incrCounter;
		T incrCounterPlus1;
		T counter;
	};

	template <typename T>
	class bresenham_counter_alt
	{
		/* operator T() returns x[0..N-1] = 0 .. R, i.e. x[n] = (R / (N-1)) * n, without
		using floating point or multiplication/division each iteration */
	public:
		bresenham_counter_alt() {}
		bresenham_counter_alt(T range, T number) : 
			n(number-1),
			partInt(n > 0 ? range / n : 0),
			partFract(n > 0 ? range % n : 0), 
			e(0),
			incrCounter(partInt),
			incrCounterPlus1(n > 0 ? incrCounter+1 : 0),
			counter(0) {}
		bresenham_counter_alt(T rangeStart, T rangeEnd, T number) : 
			n(number-1),
			partInt(n > 0 ? rangeEnd > rangeStart ? 
				(rangeEnd - rangeStart) / n : (rangeStart - rangeEnd) / n : 0),
			partFract(n > 0 ? rangeEnd > rangeStart ?
				(rangeEnd - rangeStart) % n : (rangeStart - rangeEnd) % n: 0), 
			e(0),
			incrCounter(rangeEnd > rangeStart ? partInt : -partInt),
			incrCounterPlus1(n > 0 ? rangeEnd > rangeStart ? incrCounter+1 : incrCounter-1 : 0),
			counter(rangeStart) {}
		void init(T range, T number)
		{
			n = number-1;
			partInt = n > 0 ? range / n : 0;
			partFract = n > 0 ? range % n : 0;
			e = 0;
			incrCounter = partInt;
			incrCounterPlus1 = n > 0 ? incrCounter+1 : 0;
			counter = 0;
		}
		void init(T rangeStart, T rangeEnd, T number)
		{
			n = number-1;
			partInt = n > 0 ? rangeEnd > rangeStart ? 
				(rangeEnd - rangeStart) / n : (rangeStart - rangeEnd) / n : 0;
			partFract = n > 0 ? rangeEnd > rangeStart ?
				(rangeEnd - rangeStart) % n : (rangeStart - rangeEnd) % n: 0;
			e = 0;
			incrCounter = rangeEnd > rangeStart ? partInt : -partInt;
			incrCounterPlus1 = n > 0 ? rangeEnd > rangeStart ? incrCounter+1 : incrCounter-1 : 0;
			counter = rangeStart;
		}
		operator T()
		{
			e += partFract;
			if (e < n)
			{
				T v = counter;
				counter += incrCounter;
				return v;
			}
			else
			{
				e -= n;
				T v = counter;
				counter += incrCounterPlus1;
				return v;
			}
		}
		T operator()()
		{
			return operator T();
		}
	private:
		T n, partInt, partFract, e;
		T incrCounter;
		T incrCounterPlus1;
		T counter;
	};

	template <typename T> inline
	void swap(T& a, T& b)
	{
		T x = a;
		a = b;
		b = x;
	}

	#undef min
	template <typename T> inline
	const T& min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	#undef max
	template <typename T> inline
	const T& max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}

	template <typename FwdIter, typename T>
	FwdIter binary_find(FwdIter first, FwdIter last, const T& value)
	{
		FwdIter it = std::lower_bound(first, last, value);
		return it == last || value < *it ? last : it;
	}

	template <typename FwdIter, typename T, typename Compare>
	FwdIter binary_find(FwdIter first, FwdIter last, const T& value, Compare comp)
	{
		FwdIter it = std::lower_bound(first, last, value, comp);
		return it == last || comp(value, *it) ? last : it;
	}
}
