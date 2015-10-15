#pragma once

#include "base/math/vector.hpp"
#include "base/math/plane.hpp"
#include <cstdint>
#include <vector>

namespace scene { namespace shape {

struct Vertex;

namespace triangle {

struct Index_triangle;
struct Triangle;

namespace bvh {

class Split_candidate {
public:

	Split_candidate(const math::plane& plane, uint8_t axis);
	Split_candidate(uint8_t bb_axis, uint8_t split_axis, const math::float3& p,
					const std::vector<uint32_t>& primitive_indices,
					const std::vector<Index_triangle>& triangles,
					const std::vector<Vertex>& vertices);

	uint64_t key() const;

	const math::plane& plane() const;

	uint8_t axis() const;

private:

	uint64_t key_;

	math::plane plane_;

	uint8_t axis_;
};

}}}}
