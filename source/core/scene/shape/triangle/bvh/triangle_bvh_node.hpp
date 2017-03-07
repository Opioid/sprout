#pragma once

#include "base/math/vector3.hpp"
//#include "base/math/ray.hpp"
#include "base/math/simd/simd.hpp"

namespace scene { namespace shape { namespace triangle { namespace bvh {

class Node {

public:

	Node() = default;

	float3 min() const;
	float3 max() const;

	uint32_t next() const;

	uint8_t axis() const;
	uint8_t num_primitives() const;

	uint32_t indices_start() const;
	uint32_t indices_end() const;

	void set_aabb(const float* min, const float* max);

	void set_split_node(uint32_t next_node, uint8_t axis);
	void set_leaf_node(uint32_t start_primitive, uint8_t num_primitives);

//	bool intersect_p(const math::Ray& ray) const;

	bool intersect_p(math::simd::FVector origin,
					 math::simd::FVector inv_direction,
					 math::simd::FVector min_t,
					 math::simd::FVector max_t) const;

private:

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

	Min min_;
	Max max_;
};

}}}}
