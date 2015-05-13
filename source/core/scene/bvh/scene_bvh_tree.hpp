#pragma once

#include "scene/prop/prop_intersection.hpp"
#include "scene/shape/node_stack.hpp"
#include "base/math/bounding/aabb.hpp"
#include <vector>

namespace image { namespace sampler {

class Sampler_2D;

}}

namespace scene {

class Prop;

namespace bvh {

struct Build_node {
	Build_node();
	~Build_node();

	bool intersect(math::Oray& ray, const std::vector<Prop*>& props, Node_stack& node_stack, Intersection& intersection) const;

	bool intersect_p(const math::Oray& ray, const std::vector<Prop*>& props, Node_stack& node_stack) const;

	float opacity(const math::Oray& ray, const std::vector<Prop*>& props, Node_stack& node_stack, const image::sampler::Sampler_2D& sampler) const;

	math::aabb aabb;

	uint8_t axis;

	uint32_t offset;
	uint32_t props_end;

	Build_node* children[2];
};

class Tree {
public:

	const math::aabb& aabb() const;

	bool intersect(math::Oray& ray, Node_stack& node_stack, Intersection& intersection) const;

	bool intersect_p(const math::Oray& ray, Node_stack& node_stack) const;

	float opacity(const math::Oray& ray, Node_stack& node_stack, const image::sampler::Sampler_2D& sampler) const;

private:

	Build_node root_;

	uint32_t infinite_props_start_;
	uint32_t infinite_props_end_;

	std::vector<Prop*> props_;

	friend class Builder;
};


}}
