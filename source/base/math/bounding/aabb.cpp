#include "aabb.hpp"
#include "math/vector.inl"
#include <limits>

namespace math {

AABB::AABB() {}

AABB::AABB(const float3& min, const float3& max) {
	bounds_[0] = min;
	bounds_[1] = max;
}

const float3& AABB::min() const {
	return bounds_[0];
}

const float3& AABB::max() const {
	return bounds_[1];
}

float3 AABB::position() const {
	return 0.5f * (bounds_[0] + bounds_[1]);
}

float3 AABB::halfsize() const {
	return 0.5f * (bounds_[1] - bounds_[0]);
}

bool AABB::intersect_p(const math::Oray& ray) const {
	float min_t = (bounds_[    ray.sign[0]].x - ray.origin.x) * ray.reciprocal_direction.x;
	float max_t = (bounds_[1 - ray.sign[0]].x - ray.origin.x) * ray.reciprocal_direction.x;

	float min_ty = (bounds_[    ray.sign[1]].y - ray.origin.y) * ray.reciprocal_direction.y;
	float max_ty = (bounds_[1 - ray.sign[1]].y - ray.origin.y) * ray.reciprocal_direction.y;

	if (min_t > max_ty || min_ty > max_t) {
		return false;
	}

	if (min_ty > min_t) {
		min_t = min_ty;
	}

	if (max_ty < max_t) {
		max_t = max_ty;
	}

	float min_tz = (bounds_[    ray.sign[2]].z - ray.origin.z) * ray.reciprocal_direction.z;
	float max_tz = (bounds_[1 - ray.sign[2]].z - ray.origin.z) * ray.reciprocal_direction.z;

	if (min_t > max_tz || min_tz > max_t) {
		return false;
	}

	if (min_tz > min_t) {
		min_t = min_tz;
	}

	if (max_tz < max_t) {
		max_t = max_tz;
	}

	return min_t < ray.max_t && max_t > ray.min_t;
	/*
	tmin := (b.Bounds[    ray.Sign[0]].X - ray.Origin.X) * ray.ReciprocalDirection.X
	tmax := (b.Bounds[1 - ray.Sign[0]].X - ray.Origin.X) * ray.ReciprocalDirection.X

	tymin := (b.Bounds[    ray.Sign[1]].Y - ray.Origin.Y) * ray.ReciprocalDirection.Y
	tymax := (b.Bounds[1 - ray.Sign[1]].Y - ray.Origin.Y) * ray.ReciprocalDirection.Y

	if tmin > tymax || tymin > tmax {
		return false
	}

	if tymin > tmin {
		tmin = tymin
	}

	if tymax < tmax {
		tmax = tymax
	}

	tzmin := (b.Bounds[    ray.Sign[2]].Z - ray.Origin.Z) * ray.ReciprocalDirection.Z
	tzmax := (b.Bounds[1 - ray.Sign[2]].Z - ray.Origin.Z) * ray.ReciprocalDirection.Z

	if tmin > tzmax || tzmin > tmax {
		return false
	}

	if tzmin > tmin {
		tmin = tzmin
	}

	if tzmax < tmax {
		tmax = tzmax
	}

	return tmin < ray.MaxT && tmax > ray.MinT */
}


void AABB::set_min_max(const float3& min, const float3& max) {
	bounds_[0] = min;
	bounds_[1] = max;
}

void AABB::transform(const float4x4& m, AABB& other) const {
	float3 xa = bounds_[0].x * m.x.xyz;
	float3 xb = bounds_[1].x * m.x.xyz;

	float3 ya = bounds_[0].y * m.y.xyz;
	float3 yb = bounds_[1].y * m.y.xyz;

	float3 za = bounds_[0].z * m.z.xyz;
	float3 zb = bounds_[1].z * m.z.xyz;

	other.bounds_[0] = math::min(xa, xb) + math::min(ya, yb) + math::min(za, zb) + m.w.xyz;
	other.bounds_[1] = math::max(xa, xb) + math::max(ya, yb) + math::max(za, zb) + m.w.xyz;
}

AABB AABB::merge(const AABB& other) const {
	return AABB(math::min(bounds_[0], other.bounds_[0]), math::max(bounds_[1], other.bounds_[1]));
}

AABB AABB::empty() {
	float max = std::numeric_limits<float>::max();
	return AABB(float3(max, max, max), float3(-max, -max, -max));
}

}
