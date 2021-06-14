#ifndef SU_BASE_MATH_SIMD_INL
#define SU_BASE_MATH_SIMD_INL

#include "scalar.hpp"
#include "simd.hpp"
#include "simd_const.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

namespace simd {

static inline __m128 set_float4(float x) {
    return _mm_set1_ps(x);
}

static inline __m128 load_float(float const* x) {
    return _mm_load_ss(x);
}

static inline __m128 load_float3(float const* source) {
    // Reads an extra float which is zero'd
    __m128 v = _mm_load_ps(source);
    return _mm_and_ps(v, simd::Mask3);
}

static inline __m128 load_float4(float const* source) {
    return _mm_load_ps(source);
}

static inline __m128 load_unaligned_float4(float const* source) {
    return _mm_loadu_ps(source);
}

static inline float get_x(__m128 v) {
    return _mm_cvtss_f32(v);
}

static inline void store_float4(float* destination, __m128 v) {
    _mm_store_ps(destination, v);
}

}  // namespace simd

namespace math {

inline Simdf::Simdf(__m128 m) : v(m) {}

inline Simdf::Simdf(float s) : v(_mm_set1_ps(s)) {}

inline Simdf::Simdf(Scalar_p s) : v(SU_PERMUTE_PS(s.v, _MM_SHUFFLE(0, 0, 0, 0))) {}

inline Simdf::Simdf(float sx, float sy) {
    __m128 x = _mm_load_ss(&sx);
    __m128 y = _mm_load_ss(&sy);
    v        = _mm_unpacklo_ps(x, y);
}

inline Simdf::Simdf(Vector3f_a_p o) : v(_mm_load_ps(o.v)) {}

inline Simdf::Simdf(Vector4f_a_p o) : v(_mm_load_ps(o.v)) {}

inline Simdf::Simdf(float const* a) : v(_mm_load_ps(a)) {}

inline Simdf Simdf::create_from_3(float const* f) {
    // Reads an extra float which is zero'd
    __m128 const v = _mm_load_ps(f);
    return _mm_and_ps(v, simd::Mask3);
}

inline Simdf Simdf::create_from_3_unaligned(float const* f) {
    // Reads an extra float which is zero'd
    __m128 const v = _mm_loadu_ps(f);
    return _mm_and_ps(v, simd::Mask3);
}

inline float Simdf::x() const {
    return _mm_cvtss_f32(v);
}

inline float Simdf::y() const {
    __m128 const t = SU_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
    return _mm_cvtss_f32(t);
}

inline float Simdf::z() const {
    __m128 const t = SU_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
    return _mm_cvtss_f32(t);
}

inline float Simdf::w() const {
    __m128 const t = SU_PERMUTE_PS(v, _MM_SHUFFLE(3, 3, 3, 3));
    return _mm_cvtss_f32(t);
}

inline Simdf Simdf::splat_x() const {
    return SU_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
}

inline Simdf Simdf::splat_w() const {
    return SU_PERMUTE_PS(v, _MM_SHUFFLE(3, 3, 3, 3));
}

static inline Simdf operator+(float a, Simdf_p b) {
    __m128 const s = _mm_set1_ps(a);

    return _mm_add_ps(s, b.v);
}

static inline Simdf operator+(Simdf_p a, Simdf_p b) {
    return _mm_add_ps(a.v, b.v);
}

static inline Simdf& operator+=(Simdf& a, Simdf_p b) {
    a = _mm_add_ps(a.v, b.v);
    return a;
}

static inline Simdf operator-(float a, Simdf_p b) {
    __m128 const s = _mm_set1_ps(a);

    return _mm_sub_ps(s, b.v);
}

static inline Simdf operator-(Simdf_p a, Simdf_p b) {
    return _mm_sub_ps(a.v, b.v);
}

static inline Simdf operator-(__m128 a, Simdf_p b) {
    return _mm_sub_ps(a, b.v);
}

static inline Simdf operator*(float a, Simdf_p b) {
    __m128 const s = _mm_set1_ps(a);

    return _mm_mul_ps(s, b.v);
}

static inline Simdf operator*(Simdf_p a, Simdf_p b) {
    return _mm_mul_ps(a.v, b.v);
}

static inline Simdf operator/(Simdf_p a, Simdf_p b) {
    return _mm_div_ps(a.v, b.v);
}

static inline Simdf& operator*=(Simdf& a, Simdf_p b) {
    a = _mm_mul_ps(a.v, b.v);
    return a;
}

static inline Simdf operator-(Simdf_p v) {
    return _mm_sub_ps(_mm_set1_ps(0.f), v.v);
}

static inline Simdf dot3(Simdf_p a, Simdf_p b) {
    __m128 mul  = _mm_mul_ps(a.v, b.v);
    __m128 shuf = _mm_movehdup_ps(mul);
    __m128 sums = _mm_add_ss(mul, shuf);
    shuf        = _mm_movehl_ps(shuf, sums);
    __m128 d    = _mm_add_ss(sums, shuf);
    // Splat x
    return SU_PERMUTE_PS(d, _MM_SHUFFLE(0, 0, 0, 0));
}

static inline Scalar dot3_scalar(Simdf_p a, Simdf_p b) {
    __m128 mul  = _mm_mul_ps(a.v, b.v);
    __m128 shuf = _mm_movehdup_ps(mul);
    __m128 sums = _mm_add_ss(mul, shuf);
    shuf        = _mm_movehl_ps(shuf, sums);
    return _mm_add_ss(sums, shuf);
}

static inline Simdf cross3(Simdf_p a, Simdf_p b) {
    __m128 tmp0 = _mm_shuffle_ps(b.v, b.v, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 tmp1 = _mm_shuffle_ps(a.v, a.v, _MM_SHUFFLE(3, 0, 2, 1));

#if defined(__AVX2__)
    tmp1 = _mm_mul_ps(tmp1, b.v);

    __m128 tmp2 = _mm_fmsub_ps(tmp0, a.v, tmp1);
#else
    tmp0 = _mm_mul_ps(tmp0, a.v);
    tmp1 = _mm_mul_ps(tmp1, b.v);

    __m128 tmp2 = _mm_sub_ps(tmp0, tmp1);
#endif
    return SU_PERMUTE_PS(tmp2, _MM_SHUFFLE(3, 0, 2, 1));
}

static inline Simdf sqrt(Simdf_p x) {
    __m128 const res  = _mm_rsqrt_ps(x.v);
    __m128 const muls = _mm_mul_ps(_mm_mul_ps(x.v, res), res);
    return _mm_mul_ps(x.v, _mm_mul_ps(_mm_mul_ps(simd::Half, res), _mm_sub_ps(simd::Three, muls)));
}

static inline Simdf rsqrt(Simdf_p x) {
    __m128 const res  = _mm_rsqrt_ps(x.v);
    __m128 const muls = _mm_mul_ps(_mm_mul_ps(x.v, res), res);
    return _mm_mul_ps(_mm_mul_ps(simd::Half, res), _mm_sub_ps(simd::Three, muls));
}

static inline Simdf normalize3(Simdf_p v) {
    return rsqrt(dot3(v, v)) * v;
}

static inline Simdf reciprocal3(Simdf_p v) {
    __m128 rcp = _mm_rcp_ps(v.v);
    rcp        = _mm_and_ps(rcp, simd::Mask3);
    __m128 mul = _mm_mul_ps(v.v, _mm_mul_ps(rcp, rcp));

    return _mm_sub_ps(_mm_add_ps(rcp, rcp), mul);
}

static inline Simdf min(Simdf_p a, Simdf_p b) {
    return _mm_min_ps(a.v, b.v);
}

static inline Simdf max(Simdf_p a, Simdf_p b) {
    return _mm_max_ps(a.v, b.v);
}

static inline Simdf min_scalar(Simdf_p a, Simdf_p b) {
    return _mm_min_ss(a.v, b.v);
}

static inline Simdf max_scalar(Simdf_p a, Simdf_p b) {
    return _mm_max_ss(a.v, b.v);
}

static inline void sign(Simdf_p v, uint32_t s[4]) {
    __m128 const sm = _mm_cmplt_ps(v.v, simd::Zero);

    __m128i const smi = _mm_and_si128(_mm_castps_si128(sm), simd::Bool_mask);

    _mm_store_si128(reinterpret_cast<__m128i*>(s), smi);
}

static inline Simdf lerp(Simdf_p a, Simdf_p b, Simdf_p t) {
    Simdf const u = 1.f - t;
    return u * a + t * b;
}

}  // namespace math

#endif
