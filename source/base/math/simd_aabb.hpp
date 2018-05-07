#ifndef SU_BASE_MATH_SIMD_AABB_HPP
#define SU_BASE_MATH_SIMD_AABB_HPP

#include "simd/simd.hpp"

namespace math {

class AABB;

struct Simd_AABB {
	Simd_AABB() = default;
	Simd_AABB(AABB const& box);
	Simd_AABB(float const* min, float const* max);
	Simd_AABB(FVector min, FVector max);

	void merge_assign(const Simd_AABB& other);
	void merge_assign(FVector other_min, FVector other_max);

	Vector min;
	Vector max;
};

}

#endif
