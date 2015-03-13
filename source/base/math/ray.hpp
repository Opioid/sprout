#pragma once

#include <cstdint>

namespace math {

template<typename T> struct Vector3;

template<typename T>
struct Ray {
	Vector3<T> origin, direction;
	T min_t, max_t;
};

template<typename T>
struct Optimized_ray : public Ray<T> {
	Vector3<T> reciprocal_direction;
	uint8_t sign[3];
	T time;
	uint32_t depth;
};

typedef Optimized_ray<float> Oray;

}
