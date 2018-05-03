#ifndef SU_BASE_SIMD_AABB_INL
#define SU_BASE_SIMD_AABB_INL

#include "simd_aabb.hpp"
#include "aabb.hpp"
#include "simd_vector.inl"
#include "simd/simd.inl"

namespace math {

inline Simd_AABB::Simd_AABB(const AABB& box) :
	min(simd::load_float3(box.min().v)),
	max(simd::load_float3(box.max().v)) {}

inline Simd_AABB::Simd_AABB(float const* min, float const* max) :
	min(simd::load_float3(min)),
	max(simd::load_float3(max)) {}

inline Simd_AABB::Simd_AABB(FVector min, FVector max) : min(min), max(max) {}

inline void Simd_AABB::merge_assign(const Simd_AABB& other) {
	min = math::min(min, other.min);
	max = math::max(max, other.max);
}

inline void Simd_AABB::merge_assign(FVector other_min, FVector other_max) {
	min = math::min(min, other_min);
	max = math::max(max, other_max);
}

}

#endif
