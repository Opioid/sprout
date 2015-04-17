#pragma once

#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace math {

extern const float Pi;
extern const float Pi_div_2;
extern const float Pi_div_180;
extern const float _180_div_pi;
extern const float Pi_inv;

inline float degrees_to_radians(float degrees) {
	return degrees * Pi_div_180;
}

inline float radians_to_degrees(float radians) {
	return radians * _180_div_pi;
}

template <typename T>
inline T cot(T x) {
	return tan(Pi_div_2 - x);
}

template <typename T>
inline int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

template<typename T>
inline T frac(T x) {
	return x - std::floor(x);
}

template<typename T>
inline T round(T x) {
	return std::floor(x >= T(0) ? x + T(0.5) : x - T(0.5));
}

template<typename T>
inline T saturate(T a) {
	return std::min(std::max(a, T(0)), T(1));
}

template<typename T>
inline T lerp(T a, T b, T t) {
	T u = T(1) - t;
	return u * a + t * b;
}

template<typename T>
inline T exp2(T x) {
	return pow(T(2), x);
}

template<typename T>
inline T inverse_sqrt(T x) {
	return T(1) / std::sqrt(x);
}

}
