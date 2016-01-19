#pragma once

#include "differential.hpp"
#include "base/math/vector.inl"

namespace scene { namespace shape {

inline math::float3 Differential::tangent_to_world(const math::float3& v) const{
	return math::float3(
		v.x * t.x + v.y * b.x + v.z * n.x,
		v.x * t.y + v.y * b.y + v.z * n.y,
		v.x * t.z + v.y * b.z + v.z * n.z);
}

inline math::float3 Differential::tangent_to_world(math::float2 v) const{
	return math::float3(
		v.x * t.x + v.y * b.x,
		v.x * t.y + v.y * b.y,
		v.x * t.z + v.y * b.z);
}

inline bool Differential::same_hemisphere(const math::float3& v) const {
	return math::dot(geo_n, v) > 0.f;
}

inline void Differential::revert_direction() {
	n = -n;
	geo_n = -geo_n;
}

}}
