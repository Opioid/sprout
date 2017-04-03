#pragma once

#include "simd.inl"

namespace simd {

SU_GLOBALCONST(Vector_f32) Exp_hi			= {{{ 88.3762626647949f,  88.3762626647949f,  88.3762626647949f,  88.3762626647949f }}};
SU_GLOBALCONST(Vector_f32) Exp_lo			= {{{-88.3762626647949f, -88.3762626647949f, -88.3762626647949f, -88.3762626647949f }}};
SU_GLOBALCONST(Vector_f32) Cephes_exp_C1	= {{{ 0.693359375f, 0.693359375f, 0.693359375f, 0.693359375f }}};
SU_GLOBALCONST(Vector_f32) Cephes_exp_C2	= {{{-2.12194440e-4f, -2.12194440e-4f, -2.12194440e-4f, -2.12194440e-4f }}};
SU_GLOBALCONST(Vector_f32) Cephes_exp_p0	= {{{ 1.9875691500E-4f, 1.9875691500E-4f, 1.9875691500E-4f, 1.9875691500E-4f }}};
SU_GLOBALCONST(Vector_f32) Cephes_exp_p1	= {{{ 1.3981999507E-3f, 1.3981999507E-3f, 1.3981999507E-3f, 1.3981999507E-3f }}};
SU_GLOBALCONST(Vector_f32) Cephes_exp_p2	= {{{ 8.3334519073E-3f, 8.3334519073E-3f, 8.3334519073E-3f, 8.3334519073E-3f }}};
SU_GLOBALCONST(Vector_f32) Cephes_exp_p3	= {{{ 4.1665795894E-2f, 4.1665795894E-2f, 4.1665795894E-2f, 4.1665795894E-2f }}};
SU_GLOBALCONST(Vector_f32) Cephes_exp_p4	= {{{ 1.6666665459E-1f, 1.6666665459E-1f, 1.6666665459E-1f, 1.6666665459E-1f }}};
SU_GLOBALCONST(Vector_f32) Cephes_exp_p5	= {{{ 5.0000001201E-1f, 5.0000001201E-1f, 5.0000001201E-1f, 5.0000001201E-1f }}};
SU_GLOBALCONST(Vector_f32) LogEst0			= {{{ 1.44269504088896341f, 1.44269504088896341f, 1.44269504088896341f, 1.44269504088896341f }}};

}

namespace math {

// This is exp from
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

static inline float exp(float s) {
	Vector tmp = _mm_setzero_ps();
	Vector fx;

	__m128i emm0;

	Vector x = _mm_load_ss(&s);
	x = _mm_min_ss(x, simd::Exp_hi);
	x = _mm_max_ss(x, simd::Exp_lo);

	// express exp(x) as exp(g + n*log(2))
	fx = _mm_mul_ss(x, simd::LogEst0);
	fx = _mm_add_ss(fx, simd::Half);

	// how to perform a floorf with SSE: just below
	emm0 = _mm_cvttps_epi32(fx);
	tmp  = _mm_cvtepi32_ps(emm0);

	// if greater, substract 1
	Vector mask = _mm_cmpgt_ss(tmp, fx);
	mask = _mm_and_ps(mask, simd::One);
	fx = _mm_sub_ss(tmp, mask);

	tmp = _mm_mul_ss(fx,	  simd::Cephes_exp_C1);
	Vector z = _mm_mul_ss(fx, simd::Cephes_exp_C2);
	x = _mm_sub_ss(x, tmp);
	x = _mm_sub_ss(x, z);

	z = _mm_mul_ss(x,x);

	Vector y = simd::Cephes_exp_p0;
	y = _mm_mul_ss(y, x);
	y = _mm_add_ss(y, simd::Cephes_exp_p1);
	y = _mm_mul_ss(y, x);
	y = _mm_add_ss(y, simd::Cephes_exp_p2);
	y = _mm_mul_ss(y, x);
	y = _mm_add_ss(y, simd::Cephes_exp_p3);
	y = _mm_mul_ss(y, x);
	y = _mm_add_ss(y, simd::Cephes_exp_p4);
	y = _mm_mul_ss(y, x);
	y = _mm_add_ss(y, simd::Cephes_exp_p5);
	y = _mm_mul_ss(y, z);
	y = _mm_add_ss(y, x);
	y = _mm_add_ss(y, simd::One);

	emm0 = _mm_cvttps_epi32(fx);
	emm0 = _mm_add_epi32(emm0, simd::Mask_0x7F);
	emm0 = _mm_slli_epi32(emm0, 23);
	Vector pow2n = _mm_castsi128_ps(emm0);

	y = _mm_mul_ss(y, pow2n);
	return _mm_cvtss_f32(y);
}

}
