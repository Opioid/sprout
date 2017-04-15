#pragma once

#include "simd_aabb.hpp"
#include "math/vector.inl"

namespace math {

inline Simd_AABB::Simd_AABB(const AABB& box) :
	min(simd::load_float3(box.min().v)),
	max(simd::load_float3(box.max().v)) {}

inline Simd_AABB::Simd_AABB(const float* min, const float* max) :
	min(simd::load_float3(min)),
	max(simd::load_float3(max)) {}

inline Simd_AABB::Simd_AABB(VVector min, VVector max) : min(min), max(max) {}

inline void Simd_AABB::merge_assign(const Simd_AABB& other) {
	min = math::min(min, other.min);
	max = math::max(max, other.max);
}

inline void Simd_AABB::merge_assign(VVector other_min, VVector other_max) {
	min = math::min(min, other_min);
	max = math::max(max, other_max);
}

}
