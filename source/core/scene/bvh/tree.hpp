#pragma once

#include "scene/prop/prop_intersection.hpp"
#include "base/math/bounding/aabb.hpp"
#include <vector>

namespace scene {

class Prop;

namespace bvh {

struct Build_node {
	Build_node();
	~Build_node();

	bool intersect(math::Oray& ray, const std::vector<Prop*>& props, Intersection& intersection) const;
	bool intersect_p(const math::Oray& ray, const std::vector<Prop*>& props) const;

	math::AABB aabb;

	uint32_t axis;

	uint32_t offset;
	uint32_t props_end;

	Build_node* children[2];
};

class Tree {
public:

	bool intersect(math::Oray& ray, Intersection& intersection) const;

	bool intersect_p(const math::Oray& ray) const;

private:

	Build_node root_;

	uint32_t infinite_props_start_;
	uint32_t infinite_props_end_;

	std::vector<Prop*> props_;

	friend class Builder;
};


}}
