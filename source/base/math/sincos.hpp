#ifndef SU_BASE_MATH_SINCOS_HPP
#define SU_BASE_MATH_SINCOS_HPP

#include "simd.inl"
#include "vector3.inl"

namespace simd {

inline Vector_f32 constexpr Minus_cephes_DP1 = {
    {{-0.78515625f, -0.78515625f, -0.78515625f, -0.78515625f}}};
inline Vector_f32 constexpr Minus_cephes_DP2 = {
    {{-2.4187564849853515625e-4f, -2.4187564849853515625e-4f, -2.4187564849853515625e-4f,
      -2.4187564849853515625e-4f}}};
inline Vector_f32 constexpr Minus_cephes_DP3 = {
    {{-3.77489497744594108e-8f, -3.77489497744594108e-8f, -3.77489497744594108e-8f,
      -3.77489497744594108e-8f}}};
inline Vector_f32 constexpr Sincof_p0 = {
    {{-1.9515295891e-4f, -1.9515295891e-4f, -1.9515295891e-4f, -1.9515295891e-4f}}};
inline Vector_f32 constexpr Sincof_p1 = {
    {{8.3321608736e-3f, 8.3321608736e-3f, 8.3321608736e-3f, 8.3321608736e-3f}}};
inline Vector_f32 constexpr Sincof_p2 = {
    {{-1.6666654611e-1f, -1.6666654611e-1f, -1.6666654611e-1f, -1.6666654611e-1f}}};
inline Vector_f32 constexpr Coscof_p0   = {{{2.443315711809948e-005f, 2.443315711809948e-005f,
                                           2.443315711809948e-005f, 2.443315711809948e-005f}}};
inline Vector_f32 constexpr Coscof_p1   = {{{-1.388731625493765e-003f, -1.388731625493765e-003f,
                                           -1.388731625493765e-003f, -1.388731625493765e-003f}}};
inline Vector_f32 constexpr Coscof_p2   = {{{4.166664568298827e-002f, 4.166664568298827e-002f,
                                           4.166664568298827e-002f, 4.166664568298827e-002f}}};
inline Vector_f32 constexpr Cephes_FOPI = {
    {{1.27323954473516f, 1.27323954473516f, 1.27323954473516f, 1.27323954473516f}}};  // 4 / Pi

}  // namespace simd

namespace math {

// This is sincos from
// http://gruntthepeon.free.fr/ssemath/sse_mathfun.h

static inline __m128 sin(__m128 x) {
    __m128  xmm1, xmm2, xmm3, sign_bit, y;
    __m128i emm0, emm2;

    sign_bit = x;
    // take the absolute value
    x = _mm_and_ps(x, simd::Inverse_sign_mask);
    // extract the sign bit (upper one)
    sign_bit = _mm_and_ps(sign_bit, simd::Sign_mask);

    // scale by 4/Pi
    y = _mm_mul_ps(x, simd::Cephes_FOPI);

    // store the integer part of y in mm2
    emm2 = _mm_cvttps_epi32(y);
    // j=(j+1) & (~1) (see the cephes sources)
    emm2 = _mm_add_epi32(emm2, simd::One_i);
    emm2 = _mm_and_si128(emm2, simd::Inverse_one_i);
    y    = _mm_cvtepi32_ps(emm2);

    // get the swap sign flag
    emm0 = _mm_and_si128(emm2, simd::Four_i);
    emm0 = _mm_slli_epi32(emm0, 29);
    // get the polynom selection mask
    // there is one polynom for 0 <= x <= Pi/4
    // and another one for Pi/4<x<=Pi/2
    // Both branches will be computed.

    emm2 = _mm_and_si128(emm2, simd::Two_i);
    emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());

    __m128 swap_sign_bit = _mm_castsi128_ps(emm0);
    __m128 poly_mask     = _mm_castsi128_ps(emm2);
    sign_bit             = _mm_xor_ps(sign_bit, swap_sign_bit);

    // The magic pass: "Extended precision modular arithmetic"
    // x = ((x - y * DP1) - y * DP2) - y * DP3;
    xmm1 = simd::Minus_cephes_DP1;
    xmm2 = simd::Minus_cephes_DP2;
    xmm3 = simd::Minus_cephes_DP3;
    xmm1 = _mm_mul_ps(y, xmm1);
    xmm2 = _mm_mul_ps(y, xmm2);
    xmm3 = _mm_mul_ps(y, xmm3);
    x    = _mm_add_ps(x, xmm1);
    x    = _mm_add_ps(x, xmm2);
    x    = _mm_add_ps(x, xmm3);

    // Evaluate the first polynom(0 <= x <= Pi/4)
    y        = simd::Coscof_p0;
    __m128 z = _mm_mul_ps(x, x);

    y          = _mm_mul_ps(y, z);
    y          = _mm_add_ps(y, simd::Coscof_p1);
    y          = _mm_mul_ps(y, z);
    y          = _mm_add_ps(y, simd::Coscof_p2);
    y          = _mm_mul_ps(y, z);
    y          = _mm_mul_ps(y, z);
    __m128 tmp = _mm_mul_ps(z, simd::Half);
    y          = _mm_sub_ps(y, tmp);
    y          = _mm_add_ps(y, simd::One);

    // Evaluate the second polynom(Pi/4 <= x <= 0)

    __m128 y2 = simd::Sincof_p0;
    y2        = _mm_mul_ps(y2, z);
    y2        = _mm_add_ps(y2, simd::Sincof_p1);
    y2        = _mm_mul_ps(y2, z);
    y2        = _mm_add_ps(y2, simd::Sincof_p2);
    y2        = _mm_mul_ps(y2, z);
    y2        = _mm_mul_ps(y2, x);
    y2        = _mm_add_ps(y2, x);

    // select the correct result from the two polynoms
    xmm3 = poly_mask;
    y2   = _mm_and_ps(xmm3, y2);
    y    = _mm_andnot_ps(xmm3, y);
    y    = _mm_add_ps(y, y2);
    // update the sign
    y = _mm_xor_ps(y, sign_bit);
    return y;
}

static inline __m128 cos(__m128 x) {
    __m128 xmm1, xmm2, xmm3, y;

    __m128i emm0, emm2;

    // take the absolute value
    x = _mm_and_ps(x, simd::Inverse_sign_mask);

    // scale by 4/Pi
    y = _mm_mul_ps(x, simd::Cephes_FOPI);

    // store the integer part of y in mm0
    emm2 = _mm_cvttps_epi32(y);
    // j=(j+1) & (~1) (see the cephes sources)
    emm2 = _mm_add_epi32(emm2, simd::One_i);
    emm2 = _mm_and_si128(emm2, simd::Inverse_one_i);
    y    = _mm_cvtepi32_ps(emm2);

    emm2 = _mm_sub_epi32(emm2, simd::Two_i);

    // get the swap sign flag
    emm0 = _mm_andnot_si128(emm2, simd::Four_i);
    emm0 = _mm_slli_epi32(emm0, 29);
    // get the polynom selection mask
    emm2 = _mm_and_si128(emm2, simd::Two_i);
    emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());

    __m128 sign_bit  = _mm_castsi128_ps(emm0);
    __m128 poly_mask = _mm_castsi128_ps(emm2);

    // The magic pass: "Extended precision modular arithmetic"
    // x = ((x - y * DP1) - y * DP2) - y * DP3;
    xmm1 = simd::Minus_cephes_DP1;
    xmm2 = simd::Minus_cephes_DP2;
    xmm3 = simd::Minus_cephes_DP3;
    xmm1 = _mm_mul_ps(y, xmm1);
    xmm2 = _mm_mul_ps(y, xmm2);
    xmm3 = _mm_mul_ps(y, xmm3);
    x    = _mm_add_ps(x, xmm1);
    x    = _mm_add_ps(x, xmm2);
    x    = _mm_add_ps(x, xmm3);

    // Evaluate the first polynom(0 <= x <= Pi/4)
    y        = simd::Coscof_p0;
    __m128 z = _mm_mul_ps(x, x);

    y          = _mm_mul_ps(y, z);
    y          = _mm_add_ps(y, simd::Coscof_p1);
    y          = _mm_mul_ps(y, z);
    y          = _mm_add_ps(y, simd::Coscof_p2);
    y          = _mm_mul_ps(y, z);
    y          = _mm_mul_ps(y, z);
    __m128 tmp = _mm_mul_ps(z, simd::Half);
    y          = _mm_sub_ps(y, tmp);
    y          = _mm_add_ps(y, simd::One);

    // Evaluate the second polynom(Pi/4 <= x <= 0)

    __m128 y2 = simd::Sincof_p0;
    y2        = _mm_mul_ps(y2, z);
    y2        = _mm_add_ps(y2, simd::Sincof_p1);
    y2        = _mm_mul_ps(y2, z);
    y2        = _mm_add_ps(y2, simd::Sincof_p2);
    y2        = _mm_mul_ps(y2, z);
    y2        = _mm_mul_ps(y2, x);
    y2        = _mm_add_ps(y2, x);

    // select the correct result from the two polynoms
    xmm3 = poly_mask;
    y2   = _mm_and_ps(xmm3, y2);  //, xmm3);
    y    = _mm_andnot_ps(xmm3, y);
    y    = _mm_add_ps(y, y2);
    // update the sign
    y = _mm_xor_ps(y, sign_bit);

    return y;
}

static inline void sincos(Simdf_p x, Simdf& s, Simdf& c) {
    __m128 xmm1, xmm2, xmm3, sign_bit_sin, y;

    __m128i emm0, emm2, emm4;

    sign_bit_sin = x.v;
    // take the absolute value
    __m128 xa = _mm_and_ps(x.v, simd::Inverse_sign_mask);
    // extract the sign bit (upper one)
    sign_bit_sin = _mm_and_ps(sign_bit_sin, simd::Sign_mask);

    // scale by 4/Pi
    y = _mm_mul_ps(xa, simd::Cephes_FOPI);

    // store the integer part of y in emm2
    emm2 = _mm_cvttps_epi32(y);

    // j=(j+1) & (~1) (see the cephes sources)
    emm2 = _mm_add_epi32(emm2, simd::One_i);
    emm2 = _mm_and_si128(emm2, simd::Inverse_one_i);
    y    = _mm_cvtepi32_ps(emm2);

    emm4 = emm2;

    // get the swap sign flag for the sine
    emm0                     = _mm_and_si128(emm2, simd::Four_i);
    emm0                     = _mm_slli_epi32(emm0, 29);
    __m128 swap_sign_bit_sin = _mm_castsi128_ps(emm0);

    // get the polynom selection mask for the sine*/
    emm2             = _mm_and_si128(emm2, simd::Two_i);
    emm2             = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());
    __m128 poly_mask = _mm_castsi128_ps(emm2);

    // The magic pass: "Extended precision modular arithmetic"
    // x = ((x - y * DP1) - y * DP2) - y * DP3;
    xmm1 = simd::Minus_cephes_DP1;
    xmm2 = simd::Minus_cephes_DP2;
    xmm3 = simd::Minus_cephes_DP3;
    xmm1 = _mm_mul_ps(y, xmm1);
    xmm2 = _mm_mul_ps(y, xmm2);
    xmm3 = _mm_mul_ps(y, xmm3);
    xa   = _mm_add_ps(xa, xmm1);
    xa   = _mm_add_ps(xa, xmm2);
    xa   = _mm_add_ps(xa, xmm3);

    emm4                = _mm_sub_epi32(emm4, simd::Two_i);
    emm4                = _mm_andnot_si128(emm4, simd::Four_i);
    emm4                = _mm_slli_epi32(emm4, 29);
    __m128 sign_bit_cos = _mm_castsi128_ps(emm4);

    sign_bit_sin = _mm_xor_ps(sign_bit_sin, swap_sign_bit_sin);

    // Evaluate the first polynom(0 <= x <= Pi/4)
    __m128 z = _mm_mul_ps(xa, xa);
    y        = simd::Coscof_p0;

    y          = _mm_mul_ps(y, z);
    y          = _mm_add_ps(y, simd::Coscof_p1);
    y          = _mm_mul_ps(y, z);
    y          = _mm_add_ps(y, simd::Coscof_p2);
    y          = _mm_mul_ps(y, z);
    y          = _mm_mul_ps(y, z);
    __m128 tmp = _mm_mul_ps(z, simd::Half);
    y          = _mm_sub_ps(y, tmp);
    y          = _mm_add_ps(y, simd::One);

    // Evaluate the second polynom(Pi/4 <= x <= 0)

    __m128 y2 = simd::Sincof_p0;
    y2        = _mm_mul_ps(y2, z);
    y2        = _mm_add_ps(y2, simd::Sincof_p1);
    y2        = _mm_mul_ps(y2, z);
    y2        = _mm_add_ps(y2, simd::Sincof_p2);
    y2        = _mm_mul_ps(y2, z);
    y2        = _mm_mul_ps(y2, xa);
    y2        = _mm_add_ps(y2, xa);

    // select the correct result from the two polynoms
    xmm3         = poly_mask;
    __m128 ysin2 = _mm_and_ps(xmm3, y2);
    __m128 ysin1 = _mm_andnot_ps(xmm3, y);
    y2           = _mm_sub_ps(y2, ysin2);
    y            = _mm_sub_ps(y, ysin1);

    xmm1 = _mm_add_ps(ysin1, ysin2);
    xmm2 = _mm_add_ps(y, y2);

    // update the sign
    s = _mm_xor_ps(xmm1, sign_bit_sin);
    c = _mm_xor_ps(xmm2, sign_bit_cos);
}

struct Sincos {
    float sin;
    float cos;
};

static inline Sincos sincos(float xf) {
    __m128 x = _mm_load_ss(&xf);

    __m128 xmm1, xmm2, xmm3, sign_bit_sin, y;

    __m128i emm0, emm2, emm4;

    sign_bit_sin = x;
    // take the absolute value
    x = _mm_and_ps(x, simd::Inverse_sign_mask);
    // extract the sign bit (upper one)
    sign_bit_sin = _mm_and_ps(sign_bit_sin, simd::Sign_mask);

    // scale by 4/Pi
    y = _mm_mul_ss(x, simd::Cephes_FOPI);

    // store the integer part of y in emm2
    emm2 = _mm_cvttps_epi32(y);

    // j=(j+1) & (~1) (see the cephes sources)
    emm2 = _mm_add_epi32(emm2, simd::One_i);
    emm2 = _mm_and_si128(emm2, simd::Inverse_one_i);

    y = _mm_cvtepi32_ps(emm2);

    emm4 = emm2;

    // get the swap sign flag for the sine
    emm0 = _mm_and_si128(emm2, simd::Four_i);
    emm0 = _mm_slli_epi32(emm0, 29);

    __m128 swap_sign_bit_sin = _mm_castsi128_ps(emm0);

    // get the polynom selection mask for the sine*/
    emm2 = _mm_and_si128(emm2, simd::Two_i);
    emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());

    __m128 poly_mask = _mm_castsi128_ps(emm2);

    // The magic pass: "Extended precision modular arithmetic"
    // x = ((x - y * DP1) - y * DP2) - y * DP3;
    xmm1 = simd::Minus_cephes_DP1;
    xmm2 = simd::Minus_cephes_DP2;
    xmm3 = simd::Minus_cephes_DP3;
    xmm1 = _mm_mul_ss(y, xmm1);
    xmm2 = _mm_mul_ss(y, xmm2);
    xmm3 = _mm_mul_ss(y, xmm3);

    x = _mm_add_ss(x, xmm1);
    x = _mm_add_ss(x, xmm2);
    x = _mm_add_ss(x, xmm3);

    emm4 = _mm_sub_epi32(emm4, simd::Two_i);
    emm4 = _mm_andnot_si128(emm4, simd::Four_i);
    emm4 = _mm_slli_epi32(emm4, 29);

    __m128 sign_bit_cos = _mm_castsi128_ps(emm4);

    sign_bit_sin = _mm_xor_ps(sign_bit_sin, swap_sign_bit_sin);

    // Evaluate the first polynom(0 <= x <= Pi/4)
    __m128 z = _mm_mul_ss(x, x);

    y = simd::Coscof_p0;
    y = _mm_mul_ss(y, z);
    y = _mm_add_ss(y, simd::Coscof_p1);
    y = _mm_mul_ss(y, z);
    y = _mm_add_ss(y, simd::Coscof_p2);
    y = _mm_mul_ss(y, z);
    y = _mm_mul_ss(y, z);

    __m128 tmp = _mm_mul_ss(z, simd::Half);

    y = _mm_sub_ss(y, tmp);
    y = _mm_add_ss(y, simd::One);

    // Evaluate the second polynom(Pi/4 <= x <= 0)

    __m128 y2 = simd::Sincof_p0;
    y2        = _mm_mul_ss(y2, z);
    y2        = _mm_add_ss(y2, simd::Sincof_p1);
    y2        = _mm_mul_ss(y2, z);
    y2        = _mm_add_ss(y2, simd::Sincof_p2);
    y2        = _mm_mul_ss(y2, z);
    y2        = _mm_mul_ss(y2, x);
    y2        = _mm_add_ss(y2, x);

    // select the correct result from the two polynoms
    xmm3 = poly_mask;

    __m128 ysin2 = _mm_and_ps(xmm3, y2);
    __m128 ysin1 = _mm_andnot_ps(xmm3, y);

    y2 = _mm_sub_ss(y2, ysin2);
    y  = _mm_sub_ss(y, ysin1);

    xmm1 = _mm_add_ss(ysin1, ysin2);
    xmm2 = _mm_add_ss(y, y2);

    // update the sign
    xmm1 = _mm_xor_ps(xmm1, sign_bit_sin);
    xmm2 = _mm_xor_ps(xmm2, sign_bit_cos);

    //    float s;
    //    float c;

    //    _mm_store_ss(&s, xmm1);
    //    _mm_store_ss(&c, xmm2);

    //    return {s, c};
    return {_mm_cvtss_f32(xmm1), _mm_cvtss_f32(xmm2)};
}

}  // namespace math

#endif
