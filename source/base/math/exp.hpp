#pragma once

#include "simd.inl"

namespace math {

// This is
// http://gruntthepeon.free.fr/ssemath/sse_mathfun.h

static inline Vector exp(Vector x) {
  Vector tmp = _mm_setzero_ps();
  Vector fx;

  __m128i emm0;

  x = _mm_min_ps(x, simd::Exp_hi);
  x = _mm_max_ps(x, simd::Exp_lo);

  // express exp(x) as exp(g + n*log(2))
  fx = _mm_mul_ps(x, simd::LogEst0);
  fx = _mm_add_ps(fx, simd::Half);

  // how to perform a floorf with SSE: just below
  emm0 = _mm_cvttps_epi32(fx);
  tmp  = _mm_cvtepi32_ps(emm0);

  // if greater, substract 1
  Vector mask = _mm_cmpgt_ps(tmp, fx);
  mask = _mm_and_ps(mask, simd::One);
  fx = _mm_sub_ps(tmp, mask);

  tmp = _mm_mul_ps(fx,	  simd::Cephes_exp_C1);
  Vector z = _mm_mul_ps(fx, simd::Cephes_exp_C2);
  x = _mm_sub_ps(x, tmp);
  x = _mm_sub_ps(x, z);

  z = _mm_mul_ps(x,x);

  Vector y = simd::Cephes_exp_p0;
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, simd::Cephes_exp_p1);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, simd::Cephes_exp_p2);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, simd::Cephes_exp_p3);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, simd::Cephes_exp_p4);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, simd::Cephes_exp_p5);
  y = _mm_mul_ps(y, z);
  y = _mm_add_ps(y, x);
  y = _mm_add_ps(y, simd::One);

  emm0 = _mm_cvttps_epi32(fx);
  emm0 = _mm_add_epi32(emm0, simd::Mask_0x7F);
  emm0 = _mm_slli_epi32(emm0, 23);
  Vector pow2n = _mm_castsi128_ps(emm0);

  y = _mm_mul_ps(y, pow2n);
  return y;
}

static inline float exp(float x) {
	Vector xs = _mm_load_ss(&x);
	_mm_store_ss(&x, exp(xs));
	return x;
}

}
