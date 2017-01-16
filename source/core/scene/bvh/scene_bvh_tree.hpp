#pragma once

#include "scene/material/sampler_settings.hpp"
#include "scene/scene_intersection.hpp"
#include "scene/shape/node_stack.hpp"
#include "base/math/bounding/aabb.hpp"
#include <vector>

namespace image { namespace texture { namespace sampler {

class Sampler_2D;

}}}

namespace scene {

class Prop;
class Worker;
struct Ray;

namespace bvh {

struct Build_node {
	using Sampler_filter = material::Sampler_settings::Filter;

	Build_node();
	~Build_node();

	bool intersect(scene::Ray& ray, const std::vector<Prop*>& props,
				   shape::Node_stack& node_stack, Intersection& intersection) const;

	bool intersect_p(const scene::Ray& ray, const std::vector<Prop*>& props,
					 shape::Node_stack& node_stack) const;

	float opacity(const scene::Ray& ray, const std::vector<Prop*>& props,
				  Worker& worker, Sampler_filter filter) const;

	float3 thin_absorption(const scene::Ray& ray, const std::vector<Prop*>& props,
						   Worker& worker, Sampler_filter filter) const;

	math::aabb aabb;

	uint8_t axis;

	uint32_t offset;
	uint32_t props_end;

	Build_node* children[2];
};

class Tree {

public:

	void clear();

	const math::aabb& aabb() const;

	bool intersect(scene::Ray& ray, shape::Node_stack& node_stack,
				   Intersection& intersection) const;

	bool intersect_p(const scene::Ray& ray, shape::Node_stack& node_stack) const;

	float opacity(const scene::Ray& ray, Worker& worker,
				  material::Sampler_settings::Filter filter) const;

	float3 absorption(const scene::Ray& ray, Worker& worker,
					  material::Sampler_settings::Filter filter) const;

private:

	Build_node root_;

	uint32_t infinite_props_start_;
	uint32_t infinite_props_end_;

	std::vector<Prop*> props_;

	friend class Builder;
};


}}
