#pragma once

#include "base/math/vector.hpp"
#include "base/math/ray.hpp"
#include "base/math/bounding/aabb.hpp"

namespace scene { namespace shape { namespace triangle { namespace bvh {

/*
struct Node {
	void set_aabb(const math::aabb& aabb);

	uint32_t primitive_end() const;

	math::aabb aabb;

	union {
		uint32_t second_child_index;
		uint32_t primitive_offset;
	};

	uint8_t axis;
	uint8_t num_primitives;
	uint8_t pad[2];
};
*/

struct Node {
	Node();

	void set_aabb(const math::aabb& aabb);

	uint32_t primitive_end() const;

	bool intersect_p(const math::Oray& ray) const;
	bool intersect_p(const math::Oray& ray, float& min_t, float& max_t) const;

	union {
		float3 bounds[2];

		struct {
			float pad0[3];

			union {
				uint32_t second_child_index;
				uint32_t primitive_offset;
			};

			float pad1[3];

			uint8_t axis;
			uint8_t num_primitives;
			uint8_t pad[2];
		};
	};
};

}}}}
