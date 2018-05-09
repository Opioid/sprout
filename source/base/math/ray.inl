#ifndef SU_BASE_MATH_RAY_INL
#define SU_BASE_MATH_RAY_INL

#include "ray.hpp"
#include "vector3.inl"

namespace math {

inline Ray::Ray(f_float3 origin, f_float3 direction, float min_t, float max_t) :
	origin(origin),
	direction(direction),
	inv_direction(reciprocal(direction)),
	min_t(min_t),
	max_t(max_t),
	signs {
		inv_direction[0] < 0.f ? uint8_t(1u) : uint8_t(0u),
		inv_direction[1] < 0.f ? uint8_t(1u) : uint8_t(0u),
		inv_direction[2] < 0.f ? uint8_t(1u) : uint8_t(0u)}
	{}

inline Ray::Ray(f_float3 origin, f_float3 direction,
				float min_t, float max_t, uint32_t depth) :
	origin(origin),
	direction(direction),
	inv_direction(reciprocal(direction)),
	min_t(min_t),
	max_t(max_t),
	signs {
		inv_direction[0] < 0.f ? uint8_t(1u) : uint8_t(0u),
		inv_direction[1] < 0.f ? uint8_t(1u) : uint8_t(0u),
		inv_direction[2] < 0.f ? uint8_t(1u) : uint8_t(0u)},
	depth(depth) {}

inline Ray::Ray(f_float3 origin, f_float3 direction, f_float3 inv_direction,
				float min_t, float max_t, uint8_t sign_x, uint8_t sign_y, uint8_t sign_z) :
	origin(origin),
	direction(direction),
	inv_direction(inv_direction),
	min_t(min_t),
	max_t(max_t),
	signs{sign_x, sign_y, sign_z} {}

inline void Ray::set_direction(f_float3 v) {
	direction = v;
	inv_direction = reciprocal(v);

	signs[0] = inv_direction[0] < 0.f ? 1u : 0u;
	signs[1] = inv_direction[1] < 0.f ? 1u : 0u;
	signs[2] = inv_direction[2] < 0.f ? 1u : 0u;
}

inline float3 Ray::point(float t) const {
	return origin + t * direction;
}

inline float Ray::length() const {
	return math::length((min_t - max_t) * direction);
}

inline Ray Ray::normalized() const {
	float const length = math::length(direction);
	return Ray(origin, direction / length, length * inv_direction,
			   min_t * length, max_t * length, signs[0], signs[1], signs[2]);
}

}

#endif
