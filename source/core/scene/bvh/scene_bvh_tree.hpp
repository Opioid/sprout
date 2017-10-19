#pragma once

#include "scene/material/sampler_settings.hpp"
#include "scene/scene_intersection.hpp"
#include "scene/shape/node_stack.hpp"
#include "base/math/aabb.hpp"
#include <vector>

namespace scene {

class Prop;
class Worker;
struct Ray;

namespace bvh {

class Node;

class Tree {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	Tree() = default;
	~Tree();

	void clear();

	using Node = bvh::Node;

	Node* allocate_nodes(uint32_t num_nodes);

	const math::AABB& aabb() const;

	bool intersect(scene::Ray& ray, shape::Node_stack& node_stack,
				   Intersection& intersection) const;

	bool intersect_p(const scene::Ray& ray, shape::Node_stack& node_stack) const;

	float opacity(const scene::Ray& ray, Sampler_filter filter, Worker& worker) const;

	float3 thin_absorption(const scene::Ray& ray, Sampler_filter filter, Worker& worker) const;

private:

	uint32_t num_nodes_ = 0;
	Node*	 nodes_		= nullptr;

	uint32_t infinite_props_start_;
	uint32_t infinite_props_end_;

	std::vector<Prop*> props_;

	math::AABB aabb_;

	friend class Builder;
};


}}
