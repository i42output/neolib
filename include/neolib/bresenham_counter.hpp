// bresenham_counter.hpp
/*
 *  Copyright (c) 2007-present, Leigh Johnston.
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

namespace neolib 
{
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
}
