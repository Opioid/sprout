#pragma once

#include "simd_aabb.hpp"
#include "simd/vector.inl"

namespace math {

inline Simd_AABB::Simd_AABB(const AABB& box) :
	min(load_float3(box.min())),
	max(load_float3(box.max())) {}

inline Simd_AABB::Simd_AABB(const float* min, const float* max) :
	min(load_float3(min)),
	max(load_float3(max)) {}

inline Simd_AABB::Simd_AABB(FVector min, FVector max) : min(min), max(max) {}

inline void Simd_AABB::merge_assign(const Simd_AABB& other) {
	min = ::min(min, other.min);
	max = ::max(max, other.max);
}

inline void Simd_AABB::merge_assign(FVector other_min, FVector other_max) {
	min = ::min(min, other_min);
	max = ::max(max, other_max);
}

}
