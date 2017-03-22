#pragma once

#include "simd.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace math {

template<typename T> constexpr T Pi_t	  = T(3.14159265358979323846);
template<typename T> constexpr T Pi_inv_t = T(0.318309886183790671538);

constexpr float Pi	   = Pi_t<float>;
constexpr float Pi_inv = Pi_inv_t<float>;

template <typename T>
static T degrees_to_radians(T degrees) {
	return degrees * (Pi_t<T> / T(180));
}

template <typename T>
static T radians_to_degrees(T radians) {
	return radians * (T(180) / Pi_t<T>);
}

template <typename T>
static T cot(T x) {
	return std::tan((Pi_t<T> / T(2)) - x);
}

template <typename T>
static int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

template<typename T>
static T frac(T x) {
	return x - std::floor(x);
}

template<typename T>
static T round(T x) {
	return std::floor(x >= T(0) ? x + T(0.5) : x - T(0.5));
}

template<typename T>
static T clamp(T a, T mi, T ma) {
	return std::min(std::max(a, mi), ma);
}

template<typename T>
static T saturate(T a) {
	return std::min(std::max(a, T(0)), T(1));
}

template<typename T>
T lerp(T a, T b, T t) {
	T u = T(1) - t;
	return u * a + t * b;
}

template<typename T>
static T exp2(T x) {
	return std::pow(T(2), x);
}

template<typename T>
static T pow4(T x) {
	T x2 = x * x;
	return x2 * x2;
}

template<typename T>
static T pow5(T x) {
	return pow4(x) * x;
}

template<typename T>
static T inverse_sqrt(T x) {
	return T(1) / std::sqrt(x);
}

template<typename T>
static T mod(T k, T n) {
	// Works for negative k; apparently % is the reminder, not really modulo
	return (k %= n) < T(0) ? k + n : k;
}

static inline float sqrt(float x) {
	Vector three = _mm_set1_ps(3.f);
	Vector half  = _mm_set1_ps(0.5f);
	Vector xs	 = _mm_load_ss(&x);
	Vector res   = _mm_rsqrt_ss(xs);
	Vector muls  = _mm_mul_ss(_mm_mul_ss(xs, res), res);
	Vector sqrtx = _mm_mul_ss(xs, _mm_mul_ss(_mm_mul_ss(half, res), _mm_sub_ss(three, muls)));
	_mm_store_ss(&x, sqrtx);
	return x;
}

static inline float rsqrt(float x) {
	Vector three = _mm_set1_ps(3.f);
	Vector half  = _mm_set1_ps(0.5f);
	Vector xs	 = _mm_load_ss(&x);
	Vector res   = _mm_rsqrt_ss(xs);
	Vector muls  = _mm_mul_ss(_mm_mul_ss(xs, res), res);
	_mm_store_ss(&x, _mm_mul_ss(_mm_mul_ss(half, res), _mm_sub_ss(three, muls)));
	return x;
}

static inline float rcp(float x) {
	Vector xs	= _mm_load_ss(&x);
	Vector rcp  = _mm_rcp_ss(xs);
	Vector muls = _mm_mul_ss(_mm_mul_ss(rcp, rcp), xs);
	_mm_store_ss(&x, _mm_sub_ss(_mm_add_ss(rcp, rcp), muls));
	return x;
}

}
