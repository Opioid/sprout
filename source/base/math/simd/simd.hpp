#pragma once

#if !defined(_SU_SSE_INTRINSICS_) && !defined(_SU_NO_INTRINSICS_)
#	define _SU_SSE_INTRINSICS_
#endif // !_SU_SSE_INTRINSICS_ && !_SU_NO_INTRINSICS_

#if defined(_SU_SSE_INTRINSICS_)
#ifndef _SU_NO_INTRINSICS_
#	include <xmmintrin.h>
#	include <emmintrin.h>
#endif
#endif

/*
#if defined(_MSC_VER) && !defined(_M_ARM) && (!_MANAGED) && (!_M_CEE) && (!defined(_M_IX86_FP) || (_M_IX86_FP > 1)) && !defined(_XM_NO_INTRINSICS_) && !defined(_XM_VECTORCALL_)
#	if ((_MSC_FULL_VER >= 170065501) && (_MSC_VER < 1800)) || (_MSC_FULL_VER >= 180020418)
#		define _XM_VECTORCALL_ 1
#	endif
#endif

#if _SU_VECTORCALL_
#	define SU_CALLCONV __vectorcall
#else
#	define SU_CALLCONV __fastcall
#endif
*/

#define _SU_VECTORCALL_ 1

#ifdef __GNUG__
#	define SU_CALLCONV __attribute__((vectorcall))
#elif defined(_MSC_VER)
#	define SU_CALLCONV __vectorcall
#endif

/****************************************************************************
 *
 * Conditional intrinsics
 *
 ****************************************************************************/

#if defined(_SU_SSE_INTRINSICS_) && !defined(_SU_NO_INTRINSICS_)
#	if defined(_SU_NO_MOVNT_)
#		define XM_STREAM_PS( p, a ) _mm_store_ps(p, a)
#	else
#		define SU_STREAM_PS(p, a) _mm_stream_ps(p, a)
#	endif
#	define SU_PERMUTE_PS(v, c) _mm_shuffle_ps(v, v, c)
#endif // _SU_SSE_INTRINSICS_ && !_SU_NO_INTRINSICS_

#if defined(_SU_NO_INTRINSICS_)
// The __vector4 structure is an intrinsic on Xbox but must be separately defined
// for x86/x64
struct __vector4 {
	union {
		float       vector4_f32[4];
		uint32_t    vector4_u32[4];
	};
};
#endif // _SU_NO_INTRINSICS_

namespace math { namespace simd {

// Vector intrinsic: Four 32 bit floating point components aligned on a 16 byte
// boundary and mapped to hardware vector registers
#if defined(_SU_SSE_INTRINSICS_) && !defined(_SU_NO_INTRINSICS_)
	using Vector = __m128;
#else
	using Vector = __vector4;
#endif

// Fix-up for (1st-3rd) Vector parameters that are pass-in-register for x86 and vector call; by reference otherwise
#if (defined(_M_IX86) || _SU_VECTORCALL_ ) && !defined(_SU_NO_INTRINSICS_)
	using FVector = const Vector;
#else
	using FVector = const Vector&;
#endif

// Fix-up for (4th) Vector parameter to pass in-register for x64 vector call; by reference otherwise
#if ((_SU_VECTORCALL_ && !defined(_M_IX86))) && !defined(_SU_NO_INTRINSICS_)
	using GVector = const Vector;
#else
	using GVector = const Vector&;
#endif

// Fix-up for (5th & 6th) Vector parameter to pass in-register for vector call; by reference otherwise
#if ( _SU_VECTORCALL_ ) && !defined(_SU_NO_INTRINSICS_)
	using HVector = const Vector;
#else
	using HVector = const Vector&;
#endif

// Fix-up for (7th+) Vector parameters to pass by reference
using CVector = const Vector&;

}}
