#pragma once

#include "memory/const.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace math {

SU_GLOBALCONST(float) Pi = 3.14159265358979323846f;
SU_GLOBALCONST(float) Pi_inv = 0.318309886183790671538f;

inline float degrees_to_radians(float degrees) {
	return degrees * (Pi / 180.f);
}

inline float radians_to_degrees(float radians) {
	return radians * (180.f / Pi);
}

template <typename T>
T cot(T x) {
	return std::tan((Pi / 2.f) - x);
}

template <typename T>
int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

template<typename T>
T frac(T x) {
	return x - std::floor(x);
}

template<typename T>
T round(T x) {
	return std::floor(x >= T(0) ? x + T(0.5) : x - T(0.5));
}

template<typename T>
T clamp(T a, T mi, T ma) {
	return std::min(std::max(a, mi), ma);
}

template<typename T>
T saturate(T a) {
	return std::min(std::max(a, T(0)), T(1));
}

template<typename T>
T lerp(T a, T b, T t) {
	T u = T(1) - t;
	return u * a + t * b;
}

template<typename T>
T exp2(T x) {
	return std::pow(T(2), x);
}

template<typename T>
T pow4(T x) {
	T x2 = x * x;
	return x2 * x2;
}

template<typename T>
T pow5(T x) {
	return pow4(x) * x;
}

template<typename T>
T inverse_sqrt(T x) {
	return T(1) / std::sqrt(x);
}

template<typename T>
T mod(T k, T n) {
	// Works for negative k; apparently % is the reminder, not really modulo
	return (k %= n) < T(0) ? k + n : k;
}

}
