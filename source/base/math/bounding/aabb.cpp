#include "aabb.hpp"
#include "math/vector.inl"
#include <limits>

namespace math {

AABB::AABB() {}

AABB::AABB(const float3& min, const float3& max) {
	bounds_[0] = min;
	bounds_[1] = max;
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

	other.bounds_[0] = min(xa, xb) + min(ya, yb) + min(za, zb) + m.w.xyz;
	other.bounds_[1] = max(xa, xb) + max(ya, yb) + max(za, zb) + m.w.xyz;
}

AABB AABB::merge(const AABB& other) const {
	return AABB(min(bounds_[0], other.bounds_[0]), max(bounds_[1], other.bounds_[1]));
}

AABB AABB::empty() {
	float max = std::numeric_limits<float>::max();
	return AABB(float3(max, max, max), float3(-max, -max, -max));
}

}
