#pragma once

#include "base/math/vector3.hpp"
//#include "base/math/ray.hpp"
#include "base/simd/simd.hpp"

namespace scene { namespace bvh {

class Node {

public:

	Node() = default;

	float3 min() const{
		return float3(min_.v);
	}

	float3 max() const {
		return float3(max_.v);
	}

	uint32_t next() const {
		return min_.next_or_data;
	}

	uint8_t axis() const {
		return max_.axis;
	}

	uint8_t num_primitives() const {
		return max_.num_primitives;
	}

	uint32_t indices_start() const {
		return min_.next_or_data;
	}

	uint32_t indices_end() const {
		return min_.next_or_data + static_cast<uint32_t>(max_.num_primitives);
	}

	void set_aabb(const float* min, const float* max);

	void set_split_node(uint32_t next_node, uint8_t axis);
	void set_leaf_node(uint32_t start_primitive, uint8_t num_primitives);

//	bool intersect_p(const math::Ray& ray) const;

	bool intersect_p(VVector origin, VVector inv_direction,
					 VVector min_t, VVector max_t) const;

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

}}
