// random.hpp - v2.1
/*
 *  Copyright (c) 2007 Leigh Johnston.
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
#include <random>

namespace neolib 
{
	template <typename T = uint32_t, typename Gen = std::mt19937>
	class random_traits
	{
	public:
		typedef T value_type;
		typedef Gen generator_type;
		typedef typename generator_type::result_type generator_result_type;
		static const std::size_t state_size = generator_type::state_size;
	private:
		template <bool IsInteger>
		struct distribution_selector;
		template <>
		struct distribution_selector<true>
		{
			typedef generator_result_type interval_value_type;
			typedef std::uniform_int_distribution<interval_value_type> distribution_type;
			static std::pair<interval_value_type, interval_value_type> interval(interval_value_type aLower, interval_value_type aUpper)
			{
				return std::make_pair(aLower, aUpper);
			}
		};
		template <>
		struct distribution_selector<false>
		{
			typedef value_type interval_value_type;
			typedef std::uniform_real_distribution<interval_value_type> distribution_type;
			static std::pair<interval_value_type, interval_value_type> interval(interval_value_type aLower, interval_value_type aUpper)
			{
				return std::make_pair(aLower, std::nextafter(aUpper, std::numeric_limits<interval_value_type>::max()));
			}
		};
		typedef typename distribution_selector<std::is_integral<value_type>::value> distribution_selector_type;
	public:
		typedef typename distribution_selector_type::distribution_type distribution_type;
		typedef typename distribution_selector_type::interval_value_type interval_value_type;
	public:
		static std::pair<interval_value_type, interval_value_type> interval(value_type aLower, value_type aUpper)
		{
			return distribution_selector_type::interval(static_cast<interval_value_type>(aLower), static_cast<interval_value_type>(aUpper));
		}
	};

	template <typename T = uint32_t, typename Gen = std::mt19937, typename Traits = random_traits<T, Gen>>
	class basic_random
	{
		// types
	public:
		typedef Traits traits_type;
		typedef typename traits_type::value_type value_type;
		typedef typename traits_type::distribution_type distribution_type;
	private:
		typedef typename traits_type::generator_type generator_type;
		typedef typename traits_type::generator_result_type generator_result_type;

		// construction
	public:
		basic_random() : iGen{ std::random_device{}() }, iSecure{ true }, iCounter{ 0 }
		{
		}
		template <typename T2>
		basic_random(T2 aSeed) : iGen{ static_cast<generator_result_type>(aSeed) }, iSecure{ false }, iCounter{ 0 }
		{
		}
		// operations
	public:
		template <typename T2>
		void seed(T2 aSeed)
		{
			iSecure = false;
			iCounter = 0;
			iGen.seed(static_cast<generator_result_type>(aSeed));
		}
		bool is_secure() const
		{
			return iSecure;
		}
		void set_secure(bool aSecure)
		{
			iSecure = aSecure;
		}
		template <typename T2>
		value_type operator()(T2 aUpper)
		{
			return get(aUpper);
		}
		template <typename T2>
		value_type operator()(T2 aLower, T2 aUpper)
		{
			return get(aLower, aUpper);
		}
		template <typename T2>
		value_type get(T2 aUpper)
		{
			increment_counter();
			return static_cast<value_type>(distribution(static_cast<value_type>(0), static_cast<value_type>(aUpper))(iGen));
		}
		template <typename T2>
		value_type get(T2 aLower, T2 aUpper)
		{
			increment_counter();
			return static_cast<value_type>(distribution(static_cast<value_type>(aLower), static_cast<value_type>(aUpper))(iGen));
		}
		// implementation
	private:
		distribution_type distribution(value_type aLower, value_type aUpper)
		{
			auto interval = traits_type::interval(aLower, aUpper);
			return distribution_type{ interval.first, interval.second };
		}
		void increment_counter()
		{
			if (iSecure && ++iCounter > traits_type::state_size)
			{
				iCounter = 0;
				iGen.seed(std::random_device{}());
			}
		}
		// attributes 
	private:
		generator_type iGen;
		bool iSecure;
		std::size_t iCounter;
	};

	using random = basic_random<uint32_t>;

	template <typename T>
	struct primes
	{
		static const T sPrimes[];
	};

	template <typename T>
	const T primes<T>::sPrimes[] = 
	{
		2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31,
		37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79,
		83, 89, 97, 103, 109, 113, 127, 137, 139, 149,
		157, 167, 179, 193, 199, 211, 227, 241, 257,
		277, 293, 313, 337, 359, 383, 409, 439, 467,
		503, 541, 577, 619, 661, 709, 761, 823, 887,
		953, 1031, 1109, 1193, 1289, 1381, 1493, 1613,
		1741, 1879, 2029, 2179, 2357, 2549, 2753, 2971,
		3209, 3469, 3739, 4027, 4349, 4703, 5087, 5503,
		5953, 6427, 6949, 7517, 8123, 8783, 9497, 10273,
		11113, 12011, 12983, 14033, 15173, 16411, 17749,
		19183, 20753, 22447, 24281, 26267, 28411, 30727,
		33223, 35933, 38873, 42043, 45481, 49201, 53201,
		57557, 62233, 67307, 72817, 78779, 85229, 92203,
		99733, 107897, 116731, 126271, 136607, 147793,
		159871, 172933, 187091, 202409, 218971, 236897,
		256279, 277261, 299951, 324503, 351061, 379787,
		410857, 444487, 480881, 520241, 562841, 608903,
		658753, 712697, 771049, 834181, 902483, 976369,
		1056323, 1142821, 1236397, 1337629, 1447153, 1565659,
		1693859, 1832561, 1982627, 2144977, 2320627, 2510653,
		2716249, 2938679, 3179303, 3439651, 3721303, 4026031,
		4355707, 4712381, 5098259, 5515729, 5967347, 6456007,
		6984629, 7556579, 8175383, 8844859, 9569143, 10352717,
		11200489, 12117689, 13109983, 14183539, 15345007,
		16601593, 17961079, 19431899, 21023161, 22744717,
		24607243, 26622317, 28802401, 31160981, 33712729,
		36473443, 39460231, 42691603, 46187573, 49969847,
		54061849, 58488943, 63278561, 68460391, 74066549,
		80131819, 86693767, 93793069, 101473717, 109783337,
		118773397, 128499677, 139022417, 150406843, 162723577,
		176048909, 190465427, 206062531, 222936881, 241193053,
		260944219, 282312799, 305431229, 330442829, 357502601,
		386778277, 418451333, 452718089, 489790921, 529899637,
		573292817, 620239453, 671030513, 725980837, 785430967,
		849749479, 919334987, 994618837, 1076067617, 1164186217,
		1259520799, 1362662261, 1474249943, 1594975441,
		1725587117, 1866894511, 2019773507
	};

	class random_traversal
	{
		// types
	public:
		typedef unsigned int value_type;
		// construction
	public:
		random_traversal(random& aRandom, value_type aNumElements) : iRandom{ aRandom }, iNumElements{ aNumElements }
		{
			reset();
		}
		// operations
	public:
		bool done() const { return iSearches == iPrime || iNumElements == 0; }
		unsigned int percent() const { return iSearches * 100 / iPrime; }
		int next()
		{
			if (done())
				return -1;

			bool found = false;
			value_type nextPosition = iCurrentPosition;
			while (!found)
			{
				nextPosition = nextPosition + iSkip;
				nextPosition %= iPrime;
				iSearches++;
				if (nextPosition < iNumElements)
				{
					iCurrentPosition = nextPosition;
					found = true;
				}
			}
			return iCurrentPosition;
		}
		int operator()() { return next(); }
		void reset()
		{
			const value_type* nextPrime = primes<value_type>::sPrimes;
			while(*nextPrime < iNumElements)
				nextPrime++;
			iPrime = *nextPrime;
			value_type a = iRandom.get(1, 13);
			value_type b = iRandom.get(1, 7);
			value_type c = iRandom.get(1, 5);
			iSkip = (a * iNumElements * iNumElements) + (b * iNumElements) + c;
			iSkip &= ~0xC0000000;
			if (iSkip % iPrime == 0)
				iSkip++;
			iCurrentPosition = iRandom.get(iNumElements-1);
			iSearches = 0;
		}
		random_traversal& operator=(const random_traversal& aOther)
		{
			iNumElements = aOther.iNumElements;
			iPrime = aOther.iPrime;
			iSkip = aOther.iSkip;
			iCurrentPosition = aOther.iCurrentPosition;
			iSearches = aOther.iSearches;
			return *this;
		}
		// attributes
	private:
		neolib::random& iRandom;
		value_type iNumElements;
		value_type iPrime;
		value_type iSkip;
		value_type iCurrentPosition;
		value_type iSearches;
	};
}
