#ifndef SU_CORE_SCENE_BVH_TREE_HPP
#define SU_CORE_SCENE_BVH_TREE_HPP

#include "scene/material/sampler_settings.hpp"
#include "scene/shape/node_stack.hpp"
#include "base/math/aabb.hpp"
#include <vector>

namespace scene {

namespace prop {
	class Prop;
	struct Intersection;
}

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
				   prop::Intersection& intersection) const;

	bool intersect_p(const scene::Ray& ray, shape::Node_stack& node_stack) const;

	float opacity(const scene::Ray& ray, Sampler_filter filter, const Worker& worker) const;

	float3 thin_absorption(const scene::Ray& ray, Sampler_filter filter,
						   const Worker& worker) const;

private:

	uint32_t num_nodes_ = 0;
	Node*	 nodes_		= nullptr;

	std::vector<prop::Prop*> props_;

	uint32_t num_infinite_props_;
	prop::Prop* const* infinite_props_;

	math::AABB aabb_;

	friend class Builder;
};

}}

#endif
