#pragma once

#include "ray.hpp"
#include "math.hpp"
#include "vector3.inl"

namespace math {

template<typename T>
Ray<T>::Ray() {}

template<typename T>
Ray<T>::Ray(const Vector3<T>& origin, const Vector3<T>& direction, T min_t, T max_t) :
	origin(origin), direction(direction), min_t(min_t), max_t(max_t)
{}

template<typename T>
Vector3<T> Ray<T>::point(T t) const {
	return origin + t * direction;
}

template<typename T>
T Ray<T>::length() const {
	return distance(point(min_t), point(max_t));
}

template<typename T>
Optimized_ray<T>::Optimized_ray() {}

template<typename T>
Optimized_ray<T>::Optimized_ray(const Vector3<T>& origin, const Vector3<T>& direction, T min_t, T max_t) :
	Ray<T>(origin, direction, min_t, max_t),
	inv_direction(T(1) / direction.x, T(1) / direction.y, T(1) / direction.z) {
	sign[0] = inv_direction.x < T(0) ? 1 : 0;
	sign[1] = inv_direction.y < T(0) ? 1 : 0;
	sign[2] = inv_direction.z < T(0) ? 1 : 0;
}

template<typename T>
void Optimized_ray<T>::set_direction(const Vector3<T>& v) {
	this->direction = v;
	inv_direction = Vector3<T>(T(1) / v.x, T(1) / v.y, T(1) / v.z);

	sign[0] = inv_direction.x < T(0) ? 1 : 0;
	sign[1] = inv_direction.y < T(0) ? 1 : 0;
	sign[2] = inv_direction.z < T(0) ? 1 : 0;
}

}
