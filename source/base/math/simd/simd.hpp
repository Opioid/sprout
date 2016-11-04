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
