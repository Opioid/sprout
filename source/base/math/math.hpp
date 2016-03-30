#pragma once

#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace math {

// The purpose of the following global constants is to prevent redundant
// reloading of the constants when they are referenced by more than one
// separate inline math routine called within the same function.  Declaring
// a constant locally within a routine is sufficient to prevent redundant
// reloads of that constant when that single routine is called multiple
// times in a function, but if the constant is used (and declared) in a
// separate math routine it would be reloaded.

#ifndef SU_GLOBALCONST
#	ifdef __GNUG__
#		define SU_GLOBALCONST(X) extern const X __attribute__((weak))
#	elif defined(_MSC_VER)
#		define SU_GLOBALCONST(X) extern const __declspec(selectany) X
#	endif
#endif

SU_GLOBALCONST(float) Pi = 3.14159265358979323846f;
SU_GLOBALCONST(float) Pi_div_2 = 1.57079632679489661923f;
SU_GLOBALCONST(float) Pi_div_180 = Pi / 180.f;
SU_GLOBALCONST(float) _180_div_pi = 180.f / Pi;
SU_GLOBALCONST(float) Pi_inv = 0.318309886183790671538f;

inline float degrees_to_radians(float degrees) {
	return degrees * Pi_div_180;
}

inline float radians_to_degrees(float radians) {
	return radians * _180_div_pi;
}

template <typename T>
T cot(T x) {
	return std::tan(Pi_div_2 - x);
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
T inverse_sqrt(T x) {
	return T(1) / std::sqrt(x);
}

}
