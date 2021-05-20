#ifndef SU_BASE_MATH_SCALAR_INL
#define SU_BASE_MATH_SCALAR_INL

#include "scalar.hpp"
#include "simd.inl"

namespace math {

inline Scalar::Scalar() = default;

inline Scalar::Scalar(__m128 m) : v(m) {}

inline Scalar::Scalar(float s) : v(_mm_load_ss(&s)) {}

inline float Scalar::x() const {
    return _mm_cvtss_f32(v);
}

static inline Scalar operator+(float a, Scalar_p b) {
    __m128 const s = _mm_load_ss(&a);

    return _mm_add_ss(s, b.v);
}

static inline Scalar operator+(Scalar_p a, Scalar_p b) {
    return _mm_add_ss(a.v, b.v);
}

static inline Scalar operator-(Scalar_p a, Scalar_p b) {
    return _mm_sub_ss(a.v, b.v);
}

static inline Scalar operator-(__m128 a, Scalar_p b) {
    return _mm_sub_ss(a, b.v);
}

static inline Scalar operator*(float a, Scalar_p b) {
    __m128 const s = _mm_load_ss(&a);

    return _mm_mul_ss(s, b.v);
}

static inline Scalar operator*(Scalar_p a, Scalar_p b) {
    return _mm_mul_ss(a.v, b.v);
}

static inline Scalar operator/(Scalar_p a, Scalar_p b) {
    return _mm_div_ss(a.v, b.v);
}

static inline Scalar operator-(Scalar_p s) {
    return _mm_sub_ss(simd::Zero, s.v);
}

static inline Scalar sqrt(Scalar_p x) {
    __m128 const res  = _mm_rsqrt_ss(x.v);
    __m128 const muls = _mm_mul_ss(_mm_mul_ss(x.v, res), res);
    return _mm_mul_ss(x.v, _mm_mul_ss(_mm_mul_ss(simd::Half, res), _mm_sub_ss(simd::Three, muls)));
}

static inline Scalar rsqrt(Scalar_p x) {
    __m128 const res  = _mm_rsqrt_ss(x.v);
    __m128 const muls = _mm_mul_ss(_mm_mul_ss(x.v, res), res);
    return _mm_mul_ss(_mm_mul_ss(simd::Half, res), _mm_sub_ss(simd::Three, muls));
}

static inline Scalar reciprocal(Scalar_p x) {
    __m128 rcp  = _mm_rcp_ss(x.v);
    __m128 muls = _mm_mul_ss(_mm_mul_ss(rcp, rcp), x.v);
    return _mm_sub_ss(_mm_add_ss(rcp, rcp), muls);
}

static inline Scalar min(Scalar_p a, Scalar_p b) {
    return _mm_min_ss(a.v, b.v);
}

static inline Scalar max(Scalar_p a, Scalar_p b) {
    return _mm_max_ss(a.v, b.v);
}

}  // namespace math

#endif
