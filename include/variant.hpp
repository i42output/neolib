// variant.hpp - v1.0
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
#include <type_traits>
#include <boost/variant.hpp>

namespace neolib
{
	template <unsigned int N>
	struct unused_variant_type
	{ 
		bool operator==(const unused_variant_type&) const { return true; } 
		bool operator!=(const unused_variant_type&) const { return false; } 
		bool operator<(const unused_variant_type& rhs) const { return false; }
	};

	template <
		typename T1, 
		typename T2 = unused_variant_type<2>, typename T3 = unused_variant_type<3>, typename T4 = unused_variant_type<4>, typename T5 = unused_variant_type<5>, 
		typename T6 = unused_variant_type<6>, typename T7 = unused_variant_type<7>, typename T8 = unused_variant_type<8>, typename T9 = unused_variant_type<9>, 
		typename T10 = unused_variant_type<10>, typename T11 = unused_variant_type<11>, typename T12 = unused_variant_type<12>, typename T13 = unused_variant_type<13>, 
		typename T14 = unused_variant_type<14>, typename T15 = unused_variant_type<15>, typename T16 = unused_variant_type<16>, typename T17 = unused_variant_type<17>, 
		typename T18 = unused_variant_type<18>, typename T19 = unused_variant_type<19>, typename T20 = unused_variant_type<20>, typename T21 = unused_variant_type<21>, 
		typename T22 = unused_variant_type<22>, typename T23 = unused_variant_type<23>, typename T24 = unused_variant_type<24>, typename T25 = unused_variant_type<25>, 
		typename T26 = unused_variant_type<26>, typename T27 = unused_variant_type<27>, typename T28 = unused_variant_type<28>, typename T29 = unused_variant_type<29>,
		typename T30 = unused_variant_type<30> >
	class variant
	{
		// friends
	private:
		template <typename T, typename Variant>
		friend T static_variant_cast(Variant& aVariant);
		template <typename T, typename Variant>
		friend T static_variant_cast(const Variant& aVariant);

		// types
	private:
		typedef boost::variant<boost::blank, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30> boost_type;
	public:
		typedef T1 type_1; typedef T2 type_2; typedef T3 type_3; typedef T4 type_4; typedef T5 type_5; 
		typedef T6 type_6; typedef T7 type_7; typedef T8 type_8; typedef T9 type_9; typedef T10 type_10;
		typedef T11 type_11; typedef T12 type_12; typedef T13 type_13; typedef T14 type_14; typedef T15 type_15; 
		typedef T16 type_16; typedef T17 type_17; typedef T18 type_18; typedef T19 type_19; typedef T20 type_20;
		typedef T21 type_21; typedef T22 type_22; typedef T23 type_23; typedef T24 type_24; typedef T25 type_25; 
		typedef T26 type_26; typedef T27 type_27; typedef T28 type_28; typedef T29 type_29; typedef T30 type_30;

		// construction
	public:
		variant()
		{
		}
		variant(const variant& other) : iContents(other.iContents)
		{
		}
		template <typename T>
		variant(const T& other) : iContents(other)
		{
		}

		// operations
	public:
		bool valid() const
		{
			return !is<boost::blank>();
		}
		bool empty() const
		{
			return is<boost::blank>();
		}
		void clear() 
		{
			iContents = boost::blank();
		}
		template <typename T>
		bool is() const
		{
			return iContents.type() == typeid(T);
		}
		int which() const
		{
			return iContents.which();
		}
		bool operator==(const variant& rhs) const 
		{ 
			return iContents == rhs.iContents;
		} 
		bool operator!=(const variant& rhs) const
		{ 
			return !(iContents == rhs.iContents);
		} 
		bool operator<(const variant& rhs) const
		{ 
			return iContents < rhs.iContents;
		} 

		// element access
	public:
		operator T1&() { return boost::get<T1>(iContents); }
		operator T2&() { return boost::get<T2>(iContents); }
		operator T3&() { return boost::get<T3>(iContents); }
		operator T4&() { return boost::get<T4>(iContents); }
		operator T5&() { return boost::get<T5>(iContents); }
		operator T6&() { return boost::get<T6>(iContents); }
		operator T7&() { return boost::get<T7>(iContents); }
		operator T8&() { return boost::get<T8>(iContents); }
		operator T9&() { return boost::get<T9>(iContents); }
		operator T10&() { return boost::get<T10>(iContents); }
		operator T11&() { return boost::get<T11>(iContents); }
		operator T12&() { return boost::get<T12>(iContents); }
		operator T13&() { return boost::get<T13>(iContents); }
		operator T14&() { return boost::get<T14>(iContents); }
		operator T15&() { return boost::get<T15>(iContents); }
		operator T16&() { return boost::get<T16>(iContents); }
		operator T17&() { return boost::get<T17>(iContents); }
		operator T18&() { return boost::get<T18>(iContents); }
		operator T19&() { return boost::get<T19>(iContents); }
		operator T20&() { return boost::get<T20>(iContents); }
		operator T21&() { return boost::get<T21>(iContents); }
		operator T22&() { return boost::get<T22>(iContents); }
		operator T23&() { return boost::get<T23>(iContents); }
		operator T24&() { return boost::get<T24>(iContents); }
		operator T25&() { return boost::get<T25>(iContents); }
		operator T26&() { return boost::get<T26>(iContents); }
		operator T27&() { return boost::get<T27>(iContents); }
		operator T28&() { return boost::get<T28>(iContents); }
		operator T29&() { return boost::get<T29>(iContents); }
		operator T30&() { return boost::get<T30>(iContents); }
		operator const T1&() const { return boost::get<T1>(iContents); }
		operator const T2&() const { return boost::get<T2>(iContents); }
		operator const T3&() const { return boost::get<T3>(iContents); }
		operator const T4&() const { return boost::get<T4>(iContents); }
		operator const T5&() const { return boost::get<T5>(iContents); }
		operator const T6&() const { return boost::get<T6>(iContents); }
		operator const T7&() const { return boost::get<T7>(iContents); }
		operator const T8&() const { return boost::get<T8>(iContents); }
		operator const T9&() const { return boost::get<T9>(iContents); }
		operator const T10&() const { return boost::get<T10>(iContents); }
		operator const T11&() const { return boost::get<T11>(iContents); }
		operator const T12&() const { return boost::get<T12>(iContents); }
		operator const T13&() const { return boost::get<T13>(iContents); }
		operator const T14&() const { return boost::get<T14>(iContents); }
		operator const T15&() const { return boost::get<T15>(iContents); }
		operator const T16&() const { return boost::get<T16>(iContents); }
		operator const T17&() const { return boost::get<T17>(iContents); }
		operator const T18&() const { return boost::get<T18>(iContents); }
		operator const T19&() const { return boost::get<T19>(iContents); }
		operator const T20&() const { return boost::get<T20>(iContents); }
		operator const T21&() const { return boost::get<T21>(iContents); }
		operator const T22&() const { return boost::get<T22>(iContents); }
		operator const T23&() const { return boost::get<T23>(iContents); }
		operator const T24&() const { return boost::get<T24>(iContents); }
		operator const T25&() const { return boost::get<T25>(iContents); }
		operator const T26&() const { return boost::get<T26>(iContents); }
		operator const T27&() const { return boost::get<T27>(iContents); }
		operator const T28&() const { return boost::get<T28>(iContents); }
		operator const T29&() const { return boost::get<T29>(iContents); }
		operator const T30&() const { return boost::get<T30>(iContents); }

		// implementation
	private:
		void* address()
		{
			switch(which())
			{
			case 1: return &boost::get<T1>(iContents); 
			case 2: return &boost::get<T2>(iContents); 
			case 3: return &boost::get<T3>(iContents); 
			case 4: return &boost::get<T4>(iContents); 
			case 5: return &boost::get<T5>(iContents); 
			case 6: return &boost::get<T6>(iContents); 
			case 7: return &boost::get<T7>(iContents); 
			case 8: return &boost::get<T8>(iContents); 
			case 9: return &boost::get<T9>(iContents); 
			case 10: return &boost::get<T10>(iContents); 
			case 11: return &boost::get<T11>(iContents); 
			case 12: return &boost::get<T12>(iContents); 
			case 13: return &boost::get<T13>(iContents); 
			case 14: return &boost::get<T14>(iContents); 
			case 15: return &boost::get<T15>(iContents); 
			case 16: return &boost::get<T16>(iContents); 
			case 17: return &boost::get<T17>(iContents); 
			case 18: return &boost::get<T18>(iContents); 
			case 19: return &boost::get<T19>(iContents); 
			case 20: return &boost::get<T20>(iContents); 
			case 21: return &boost::get<T21>(iContents); 
			case 22: return &boost::get<T22>(iContents); 
			case 23: return &boost::get<T23>(iContents); 
			case 24: return &boost::get<T24>(iContents); 
			case 25: return &boost::get<T25>(iContents); 
			case 26: return &boost::get<T26>(iContents); 
			case 27: return &boost::get<T27>(iContents); 
			case 28: return &boost::get<T28>(iContents); 
			case 29: return &boost::get<T29>(iContents); 
			case 30: return &boost::get<T30>(iContents); 
			default: return 0;
			}
		}
		const void* address() const
		{
			switch(which())
			{
			case 1: return &boost::get<T1>(iContents); 
			case 2: return &boost::get<T2>(iContents); 
			case 3: return &boost::get<T3>(iContents); 
			case 4: return &boost::get<T4>(iContents); 
			case 5: return &boost::get<T5>(iContents); 
			case 6: return &boost::get<T6>(iContents); 
			case 7: return &boost::get<T7>(iContents); 
			case 8: return &boost::get<T8>(iContents); 
			case 9: return &boost::get<T9>(iContents); 
			case 10: return &boost::get<T10>(iContents); 
			case 11: return &boost::get<T11>(iContents); 
			case 12: return &boost::get<T12>(iContents); 
			case 13: return &boost::get<T13>(iContents); 
			case 14: return &boost::get<T14>(iContents); 
			case 15: return &boost::get<T15>(iContents); 
			case 16: return &boost::get<T16>(iContents); 
			case 17: return &boost::get<T17>(iContents); 
			case 18: return &boost::get<T18>(iContents); 
			case 19: return &boost::get<T19>(iContents); 
			case 20: return &boost::get<T20>(iContents); 
			case 21: return &boost::get<T21>(iContents); 
			case 22: return &boost::get<T22>(iContents); 
			case 23: return &boost::get<T23>(iContents); 
			case 24: return &boost::get<T24>(iContents); 
			case 25: return &boost::get<T25>(iContents); 
			case 26: return &boost::get<T26>(iContents); 
			case 27: return &boost::get<T27>(iContents); 
			case 28: return &boost::get<T28>(iContents); 
			case 29: return &boost::get<T29>(iContents); 
			case 30: return &boost::get<T30>(iContents); 
			default: return 0;
			}
		}

		// attributes:
	private:
		boost_type iContents;
	};

	template <typename T, typename Variant> T static_variant_cast(Variant& aVariant) 
	{ 
		return *static_cast<typename std::remove_reference<T>::type*>(aVariant.address());
	}
	template <typename T, typename Variant> T static_variant_cast(const Variant& aVariant)
	{ 
		return *static_cast<const typename std::remove_reference<T>::type*>(aVariant.address());
	}

	template <typename Variant, typename T>
	struct variant_type_id;

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_1>
	{
		static const int value = 1;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_2>
	{
		static const int value = 2;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_3>
	{
		static const int value = 3;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_4>
	{
		static const int value = 4;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_5>
	{
		static const int value = 5;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_6>
	{
		static const int value = 6;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_7>
	{
		static const int value = 7;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_8>
	{
		static const int value = 8;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_9>
	{
		static const int value = 9;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_10>
	{
		static const int value = 10;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_11>
	{
		static const int value = 11;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_12>
	{
		static const int value = 12;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_13>
	{
		static const int value = 13;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_14>
	{
		static const int value = 14;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_15>
	{
		static const int value = 15;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_16>
	{
		static const int value = 16;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_17>
	{
		static const int value = 17;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_18>
	{
		static const int value = 18;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_19>
	{
		static const int value = 19;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_20>
	{
		static const int value = 20;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_21>
	{
		static const int value = 21;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_22>
	{
		static const int value = 22;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_23>
	{
		static const int value = 23;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_24>
	{
		static const int value = 24;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_25>
	{
		static const int value = 25;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_26>
	{
		static const int value = 26;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_27>
	{
		static const int value = 27;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_28>
	{
		static const int value = 28;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_29>
	{
		static const int value = 29;
	};

	template <typename Variant>
	struct variant_type_id<Variant, typename Variant::type_30>
	{
		static const int value = 30;
	};
}

using neolib::static_variant_cast;