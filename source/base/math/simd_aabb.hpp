#pragma once

#include "math/aabb.hpp"
#include "simd/simd.hpp"

namespace math {

struct Simd_AABB {
	Simd_AABB() = default;
	Simd_AABB(const AABB& box);
	Simd_AABB(const float* min, const float* max);
	Simd_AABB(FVector min, FVector max);

	void merge_assign(const Simd_AABB& other);
	void merge_assign(FVector other_min, FVector other_max);

	Vector min;
	Vector max;
};

}
