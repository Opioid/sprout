#pragma once

#if defined(_MSC_VER)
#	include <intrin.h>
#else
#	include <x86intrin.h>
#endif

/*
#if _SU_VECTORCALL_
#	define SU_CALLCONV __vectorcall
#else
#	define SU_CALLCONV __fastcall
#endif
*/

#define _SU_VECTORCALL_ 1

#ifdef __clang__
#	define SU_CALLCONV __attribute__((vectorcall))
#elif defined(_MSC_VER)
#	define SU_CALLCONV __vectorcall
#else
#	define SU_CALLCONV
#endif

/****************************************************************************
 *
 * Conditional intrinsics
 *
 ****************************************************************************/

#if defined(_SU_NO_MOVNT_)
#		define XM_STREAM_PS( p, a ) _mm_store_ps(p, a)
#	else
#		define SU_STREAM_PS(p, a) _mm_stream_ps(p, a)
#endif

#define SU_PERMUTE_PS(v, c) _mm_shuffle_ps(v, v, c)

	// a,b,c,d -> b,c,d,a
#define SU_ROTATE_LEFT(v) _mm_shuffle_ps(v, v, 0x39)

	// low{a,b,c,d}|high{e,f,g,h} = {c,d,g,h}
#define SU_MUX_HIGH(low, high) _mm_movehl_ps(low, high)

namespace simd {

using Vector = __m128;

// Fix-up for (1st-3rd) Vector parameters that are pass-in-register for x86 and vector call; by reference otherwise
#if (defined(_M_IX86) || _SU_VECTORCALL_ )
	using FVector = const Vector;
#else
	using FVector = const Vector&;
#endif

// Fix-up for (4th) Vector parameter to pass in-register for x64 vector call; by reference otherwise
#if ((_SU_VECTORCALL_ && !defined(_M_IX86)))
	using GVector = const Vector;
#else
	using GVector = const Vector&;
#endif

// Fix-up for (5th & 6th) Vector parameter to pass in-register for vector call; by reference otherwise
#if ( _SU_VECTORCALL_ )
	using HVector = const Vector;
#else
	using HVector = const Vector&;
#endif

// Fix-up for (7th+) Vector parameters to pass by reference
using CVector = const Vector&;

}

using Vector  = simd::Vector;
using FVector = simd::FVector;
using GVector = simd::GVector;
using HVector = simd::HVector;
