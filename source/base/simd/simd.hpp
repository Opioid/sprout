#ifndef SU_BASE_SIMD_SIMD_HPP
#define SU_BASE_SIMD_SIMD_HPP

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

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

#endif
