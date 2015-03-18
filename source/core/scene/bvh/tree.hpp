#pragma once

#include "base/math/bounding/aabb.hpp"

namespace scene { namespace bvh {

struct Build_node {
	math::AABB aabb;

	uint32_t axis;

	uint32_t offset;
	uint32_t props_end;

	Build_node* children[2];
};

class Tree {
public:

	bool intersect_p() const;

private:

	Build_node root_;

	uint32_t infinite_props_start_;
	uint32_t infinite_props_end_;
};


}}
