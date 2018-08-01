#ifndef SU_BASE_MATH_SIMD_VECTOR_INL
#define SU_BASE_MATH_SIMD_VECTOR_INL

#include "simd/simd_const.hpp"

namespace math {

/****************************************************************************
 *
 * Scalar operations
 *
 ****************************************************************************/

static inline Vector SU_CALLCONV add1(FVector a, FVector b) noexcept {
    return _mm_add_ss(a, b);
}

static inline Vector SU_CALLCONV mul1(FVector a, FVector b) noexcept {
    return _mm_mul_ss(a, b);
}

static inline Vector SU_CALLCONV min1(FVector a, FVector b) noexcept {
    return _mm_min_ss(a, b);
}

static inline Vector SU_CALLCONV max1(FVector a, FVector b) noexcept {
    return _mm_max_ss(a, b);
}

static inline Vector SU_CALLCONV sqrt1(FVector x) noexcept {
    Vector res  = _mm_rsqrt_ss(x);
    Vector muls = _mm_mul_ss(_mm_mul_ss(x, res), res);
    return _mm_mul_ss(x, _mm_mul_ss(_mm_mul_ss(simd::Half, res), _mm_sub_ss(simd::Three, muls)));
}

static inline Vector SU_CALLCONV rcp1(FVector x) noexcept {
    Vector rcp  = _mm_rcp_ss(x);
    Vector muls = _mm_mul_ss(_mm_mul_ss(rcp, rcp), x);
    return _mm_sub_ss(_mm_add_ss(rcp, rcp), muls);
}

/****************************************************************************
 *
 * Vector operations
 *
 ****************************************************************************/

static inline Vector SU_CALLCONV add(FVector a, FVector b) noexcept {
    return _mm_add_ps(a, b);
}

static inline float SU_CALLCONV horizontal_sum(FVector a) noexcept {
    //	Vector t = _mm_hadd_ps(a, a);
    //	t = _mm_hadd_ps(t, t);
    //	float r;
    //	_mm_store_ss(&r, t);
    //	return r;

    Vector shuf = _mm_movehdup_ps(a);
    Vector sums = _mm_add_ps(a, shuf);
    shuf        = _mm_movehl_ps(shuf, sums);
    sums        = _mm_add_ss(sums, shuf);
    return _mm_cvtss_f32(sums);
}

static inline Vector SU_CALLCONV sub(FVector a, FVector b) noexcept {
    return _mm_sub_ps(a, b);
}

static inline Vector SU_CALLCONV mul(FVector a, FVector b) noexcept {
    return _mm_mul_ps(a, b);
}

static inline Vector SU_CALLCONV div(FVector a, FVector b) noexcept {
    return _mm_div_ps(a, b);
}

static inline Vector SU_CALLCONV dot3(FVector a, FVector b) noexcept {
    Vector mul  = _mm_mul_ps(a, b);
    Vector shuf = _mm_movehdup_ps(mul);
    Vector sums = _mm_add_ss(mul, shuf);
    shuf        = _mm_movehl_ps(shuf, sums);
    Vector dot  = _mm_add_ss(sums, shuf);
    // Splat x
    return SU_PERMUTE_PS(dot, _MM_SHUFFLE(0, 0, 0, 0));
}

static inline Vector SU_CALLCONV dot3_1(FVector a, FVector b) noexcept {
    Vector mul  = _mm_mul_ps(a, b);
    Vector shuf = _mm_movehdup_ps(mul);
    Vector sums = _mm_add_ss(mul, shuf);
    shuf        = _mm_movehl_ps(shuf, sums);
    return _mm_add_ss(sums, shuf);
}

static inline Vector sqrt(FVector x) noexcept {
    Vector res  = _mm_rsqrt_ps(x);
    Vector muls = _mm_mul_ps(_mm_mul_ps(x, res), res);
    return _mm_mul_ps(x, _mm_mul_ps(_mm_mul_ps(simd::Half, res), _mm_sub_ps(simd::Three, muls)));
}

static inline Vector rsqrt(FVector x) noexcept {
    Vector res  = _mm_rsqrt_ps(x);
    Vector muls = _mm_mul_ps(_mm_mul_ps(x, res), res);
    return _mm_mul_ps(_mm_mul_ps(simd::Half, res), _mm_sub_ps(simd::Three, muls));
}

static inline Vector normalized3(FVector v) {
    return mul(rsqrt(dot3(v, v)), v);
}

// https://geometrian.com/programming/tutorials/cross-product/index.php
static inline Vector SU_CALLCONV cross3(FVector a, FVector b) {
    Vector tmp0 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
    Vector tmp1 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));

    tmp0 = _mm_mul_ps(tmp0, a);
    tmp1 = _mm_mul_ps(tmp1, b);

    Vector tmp2 = _mm_sub_ps(tmp0, tmp1);

    return _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
}

static inline Vector SU_CALLCONV rcp(FVector x) noexcept {
    Vector rcp  = _mm_rcp_ps(x);
    Vector muls = _mm_mul_ps(_mm_mul_ps(rcp, rcp), x);
    return _mm_sub_ps(_mm_add_ps(rcp, rcp), muls);
}

static inline Vector SU_CALLCONV reciprocal3(FVector v) noexcept {
    Vector rcp = _mm_rcp_ps(v);
    rcp        = _mm_and_ps(rcp, simd::Mask3);
    Vector mul = _mm_mul_ps(v, _mm_mul_ps(rcp, rcp));

    return _mm_sub_ps(_mm_add_ps(rcp, rcp), mul);
}

static inline Vector SU_CALLCONV min(FVector a, FVector b) noexcept {
    return _mm_min_ps(a, b);
}

static inline Vector SU_CALLCONV max(FVector a, FVector b) noexcept {
    return _mm_max_ps(a, b);
}

static inline Vector SU_CALLCONV splat_x(FVector v) noexcept {
    return SU_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
}

static inline void SU_CALLCONV sign(FVector v, uint32_t s[4]) noexcept {
    Vector  sm  = _mm_cmplt_ps(v, simd::Zero);
    __m128i smi = _mm_castps_si128(sm);
    smi         = _mm_and_si128(smi, simd::Bool_mask);
    _mm_store_si128(reinterpret_cast<__m128i*>(s), smi);
}

}  // namespace math

#endif
