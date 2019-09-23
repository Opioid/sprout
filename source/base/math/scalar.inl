#ifndef SU_BASE_MATH_SCALAR_INL
#define SU_BASE_MATH_SCALAR_INL

#include "scalar.hpp"
#include "simd/simd.inl"
#include "vector3.hpp"

namespace math {

//==============================================================================
// SIMD float scalar
//==============================================================================

inline Simd1f::Simd1f() noexcept = default;

inline Simd1f::Simd1f(__m128 m) noexcept : v(m) {}

inline Simd1f::Simd1f(float s) noexcept : v(_mm_load_ss(&s)) {}

inline float Simd1f::x() const noexcept {
    return _mm_cvtss_f32(v);
}

static inline Simd1f operator+(float a, Simd1f const& b) noexcept {
    __m128 const s = _mm_load_ss(&a);

    return _mm_add_ss(s, b.v);
}

static inline Simd1f operator+(Simd1f const& a, Simd1f const& b) noexcept {
    return _mm_add_ss(a.v, b.v);
}

static inline Simd1f operator-(Simd1f const& a, Simd1f const& b) noexcept {
    return _mm_sub_ss(a.v, b.v);
}

static inline Simd1f operator-(__m128 a, Simd1f const& b) noexcept {
    return _mm_sub_ss(a, b.v);
}

static inline Simd1f operator*(float a, Simd1f const& b) noexcept {
    __m128 const s = _mm_load_ss(&a);

    return _mm_mul_ss(s, b.v);
}

static inline Simd1f operator*(Simd1f const& a, Simd1f const& b) noexcept {
    return _mm_mul_ss(a.v, b.v);
}

static inline Simd1f operator/(Simd1f const& a, Simd1f const& b) noexcept {
    return _mm_div_ss(a.v, b.v);
}

static inline Simd1f sqrt(Simd1f const& x) noexcept {
    __m128 const res  = _mm_rsqrt_ss(x.v);
    __m128 const muls = _mm_mul_ss(_mm_mul_ss(x.v, res), res);
    return _mm_mul_ss(x.v, _mm_mul_ss(_mm_mul_ss(simd::Half, res), _mm_sub_ss(simd::Three, muls)));
}

static inline Simd1f rsqrt(Simd1f const& x) noexcept {
    __m128 const res  = _mm_rsqrt_ss(x.v);
    __m128 const muls = _mm_mul_ss(_mm_mul_ss(x.v, res), res);
    return _mm_mul_ss(_mm_mul_ss(simd::Half, res), _mm_sub_ss(simd::Three, muls));
}

static inline Simd1f reciprocal(Simd1f const x) noexcept {
    __m128 rcp  = _mm_rcp_ss(x.v);
    __m128 muls = _mm_mul_ss(_mm_mul_ss(rcp, rcp), x.v);
    return _mm_sub_ss(_mm_add_ss(rcp, rcp), muls);
}

static inline Simd1f min(Simd1f const& a, Simd1f const& b) noexcept {
    return _mm_min_ss(a.v, b.v);
}

static inline Simd1f max(Simd1f const& a, Simd1f const& b) noexcept {
    return _mm_max_ss(a.v, b.v);
}

}  // namespace math

#endif
