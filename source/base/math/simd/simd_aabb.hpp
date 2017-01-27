#pragma once

#include "simd_vector.hpp"
#include "math/aabb.hpp"

namespace math { namespace simd {

struct AABB {
	AABB(const math::AABB& box);
	AABB(FVector min, FVector max);

	void merge_assign(const AABB& other);
	void merge_assign(FVector other_min, FVector other_max);

	Vector min;
	Vector max;
};

}}
