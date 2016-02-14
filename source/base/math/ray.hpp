#pragma once

#include <cstdint>

namespace math {

template<typename T> struct Vector3;

template<typename T>
struct Ray {
	Ray();
	Ray(const Vector3<T>& origin, const Vector3<T>& direction, T min_t = T(0), T max_t = T(1));

	Vector3<T> point(T t) const;

	T length() const;

	Vector3<T> origin, direction;
	T min_t, max_t;
};

template<typename T>
struct Optimized_ray : public Ray<T> {
	Optimized_ray();
	Optimized_ray(const Vector3<T>& origin, const Vector3<T>& direction, T min_t = T(0), T max_t = T(1));

	void set_direction(const Vector3<T>& v);

	Vector3<T> inv_direction;
	int8_t sign[3];
	int8_t pad;
};

typedef Optimized_ray<float> Oray;

}
