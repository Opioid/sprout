#ifndef SU_CORE_SCENE_PROP_BVH_WRAPPER_HPP
#define SU_CORE_SCENE_PROP_BVH_WRAPPER_HPP

#include "scene/bvh/scene_bvh_tree.hpp"
#include "scene/material/sampler_settings.hpp"
#include "scene/shape/node_stack.hpp"

namespace scene {

class Worker;
struct Ray;

namespace prop {

class Prop;
struct Intersection;

class BVH_wrapper {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	bvh::Tree<Prop>& tree();

	void set_infinite_props(const std::vector<Prop*>& infite_props);

	const math::AABB& aabb() const;

	bool intersect(scene::Ray& ray, shape::Node_stack& node_stack,
				   Intersection& intersection) const;

	bool intersect_p(const scene::Ray& ray, shape::Node_stack& node_stack) const;

	float opacity(const scene::Ray& ray, Sampler_filter filter, const Worker& worker) const;

	float3 thin_absorption(const scene::Ray& ray, Sampler_filter filter,
						   const Worker& worker) const;

private:

	bvh::Tree<Prop> tree_;

	uint32_t num_infinite_props_;
	Prop* const* infinite_props_;
};

}}

#endif
