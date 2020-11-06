#ifndef SU_BASE_MATH_MATH_HPP
#define SU_BASE_MATH_MATH_HPP

#include "simd/simd_const.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace math {

template <typename T>
inline T constexpr Pi_t = T(3.14159265358979323846);

template <typename T>
inline T constexpr Pi_inv_t = T(0.318309886183790671538);

inline float constexpr Pi     = Pi_t<float>;
inline float constexpr Pi_inv = Pi_inv_t<float>;

template <typename T>
static T constexpr degrees_to_radians(T degrees) {
    return degrees * (Pi_t<T> / T(180));
}

template <typename T>
static T constexpr radians_to_degrees(T radians) {
    return radians * (T(180) / Pi_t<T>);
}

template <typename T>
static T cot(T x) {
    return std::tan((Pi_t<T> / T(2)) - x);
}

template <typename T>
static int constexpr sign(T val) {
    return (T(0) < val) - (val < T(0));
}

template <typename T>
static T frac(T x) {
    return x - std::floor(x);
}

template <typename T>
static T saturate(T a) {
    return std::min(std::max(a, T(0)), T(1));
}

template <typename T>
static T clamp(T a, T mi, T ma) {
    return std::min(std::max(a, mi), ma);
}

template <typename T>
static T constexpr lerp(T a, T b, T t) {
    T u = T(1) - t;
    return u * a + t * b;
}

template <typename T>
static T exp2(T x) {
    return T(std::pow(T(2), x));
}

template <typename T>
static T constexpr pow2(T x) {
    return x * x;
}

template <typename T>
static T constexpr pow3(T x) {
    return (x * x) * x;
}

template <typename T>
static T constexpr pow4(T x) {
    T const x2 = x * x;
    return x2 * x2;
}

template <typename T>
static T constexpr pow5(T x) {
    return pow4(x) * x;
}

template <typename T>
static T constexpr pow6(T x) {
    T const x2 = x * x;
    return x2 * x2 * x2;
}

template <typename T>
static T constexpr mod(T k, T n) {
    // Works for negative k: % is the remainder, not modulo
    return (k %= n) < T(0) ? k + n : k;
}

static inline float sqrt(float x) {
    __m128 xs    = _mm_load_ss(&x);
    __m128 res   = _mm_rsqrt_ss(xs);
    __m128 muls  = _mm_mul_ss(_mm_mul_ss(xs, res), res);
    __m128 sqrtx = _mm_mul_ss(
        xs, _mm_mul_ss(_mm_mul_ss(simd::Half, res), _mm_sub_ss(simd::Three, muls)));
    return _mm_cvtss_f32(sqrtx);
}

static inline float rsqrt(float x) {
    __m128 xs   = _mm_load_ss(&x);
    __m128 res  = _mm_rsqrt_ss(xs);
    __m128 muls = _mm_mul_ss(_mm_mul_ss(xs, res), res);
    return _mm_cvtss_f32(_mm_mul_ss(_mm_mul_ss(simd::Half, res), _mm_sub_ss(simd::Three, muls)));
}

static inline float rcp(float x) {
    __m128 xs   = _mm_load_ss(&x);
    __m128 rcp  = _mm_rcp_ss(xs);
    __m128 muls = _mm_mul_ss(_mm_mul_ss(rcp, rcp), xs);
    return _mm_cvtss_f32(_mm_sub_ss(_mm_add_ss(rcp, rcp), muls));
}

// https://twitter.com/ian_mallett/status/910006486453043203
// copysign1(x) == std::copysign(1.f, x)
static inline float copysign1(float x) {
    __m128 const result = _mm_and_ps(simd::Sign_mask, _mm_set1_ps(x));
    return _mm_cvtss_f32(_mm_or_ps(result, simd::One));
}

template <typename T>
static inline T bilinear(T c00, T c10, T c01, T c11, float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c00 + s * c10) + t * (_s * c01 + s * c11);
}

template <typename T>
static inline T round_up(T x, T m) {
    return ((x + m - 1) / m) * m;
}

}  // namespace math

#endif
