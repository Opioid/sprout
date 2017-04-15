#pragma once

#include "math/aabb.hpp"
#include "simd/simd.hpp"

namespace math {

struct Simd_AABB {
	Simd_AABB() = default;
	Simd_AABB(const AABB& box);
	Simd_AABB(const float* min, const float* max);
	Simd_AABB(VVector min, VVector max);

	void merge_assign(const Simd_AABB& other);
	void merge_assign(VVector other_min, VVector other_max);

	Vector min;
	Vector max;
};

}
