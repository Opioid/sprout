#pragma once

#include "plane.hpp"
#include "vector3.inl"
#include "vector4.inl"

namespace math { namespace plane {

static inline Plane create(const Vector3f_a& normal, float d) {
	return Plane(normal, d);
}

static inline Plane create(const Vector3f_a& normal, const Vector3f_a& point) {
	return Plane(normal[0], normal[1], normal[2], -dot(normal, point));
}

static inline Plane create(const Vector3f_a& v0, const Vector3f_a& v1, const Vector3f_a& v2) {
	Vector3f_a n = normalized(cross(v2 - v1, v0 - v1));

	return create(n, v0);
}

static inline float dot(const Vector4f_a& p, const Vector3f_a& v) {
	return (p[0] * v[0] + p[1] * v[1]) + (p[2] * v[2] + p[3]);
}

static inline bool behind(const Vector4f_a& p, const Vector3f_a& point) {
	return dot(p, point) < 0.f;
}

}}
