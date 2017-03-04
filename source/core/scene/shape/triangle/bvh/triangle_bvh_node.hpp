#pragma once

#include "base/math/aabb.hpp"
#include "base/math/vector.hpp"
#include "base/math/ray.hpp"
#include "base/math/simd/simd.hpp"

namespace scene { namespace shape { namespace triangle { namespace bvh {

struct Node {
	Node() = default;

	void set_aabb(const math::aabb& aabb);

	uint32_t next() const;

	uint32_t indices_start() const;
	uint32_t indices_end() const;

//	bool intersect_p(const math::Ray& ray) const;

	bool intersect_p(math::simd::FVector origin,
					 math::simd::FVector inv_direction,
					 math::simd::FVector min_t,
					 math::simd::FVector max_t) const;

	struct alignas(16) Min {
		float v[3];
		uint32_t next_or_data;
	};

	struct alignas(16) Max {
		float v[3];
		uint8_t axis;
		uint8_t num_primitives;
		uint8_t pad[2];
	};

	Min min;
	Max max;
};

}}}}
