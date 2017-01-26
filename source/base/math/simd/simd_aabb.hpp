#pragma once

#include "simd_vector.hpp"
#include "math/aabb.hpp"

namespace math { namespace simd {

struct AABB {
	AABB(const math::AABB& box);

	void merge_assign(const AABB& other);

	Vector min;
	Vector max;
};

}}
