#ifndef SU_BASE_SIMD_SIMD_HPP
#define SU_BASE_SIMD_SIMD_HPP

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <x86intrin.h>
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
#define SU_CALLCONV __attribute__((vectorcall))
#elif defined(_MSC_VER)
#define SU_CALLCONV __vectorcall
#else
#define SU_CALLCONV
#endif

/****************************************************************************
 *
 * Conditional intrinsics
 *
 ****************************************************************************/

#if defined(_SU_NO_MOVNT_)
#define XM_STREAM_PS(p, a) _mm_store_ps(p, a)
#else
#define SU_STREAM_PS(p, a) _mm_stream_ps(p, a)
#endif

#define SU_PERMUTE_PS(v, c) _mm_shuffle_ps(v, v, c)

// a,b,c,d -> b,c,d,a
#define SU_ROTATE_LEFT(v) _mm_shuffle_ps(v, v, 0x39)

// low{a,b,c,d}|high{e,f,g,h} = {c,d,g,h}
#define SU_MUX_HIGH(low, high) _mm_movehl_ps(low, high)

namespace simd {

using Vector = __m128;

// Fix-up for (1st-6th) Vector parameters that are pass-in-register for x64 unix and vector call;
// by reference otherwise
using FVector = Vector const;

// Fix-up for (7th+) Vector parameters to pass by reference
using RVector = Vector const&;

}  // namespace simd

using Vector  = simd::Vector;
using FVector = simd::FVector;
using RVector = simd::RVector;

#endif
