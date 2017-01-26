#pragma once

#include "simd_aabb.hpp"
#include "simd_vector.inl"

namespace math { namespace simd {

inline AABB::AABB(const math::AABB& box) :
	min(load_float3(box.min())),
	max(load_float3(box.max())) {}

inline void AABB::merge_assign(const AABB& other) {
	min = min3(min, other.min);
	max = max3(max, other.max);
}

}}
