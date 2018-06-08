#ifndef SU_CORE_SCENE_PROP_BVH_WRAPPER_HPP
#define SU_CORE_SCENE_PROP_BVH_WRAPPER_HPP

#include "scene/bvh/scene_bvh_tree.hpp"
#include "scene/material/sampler_settings.hpp"

namespace scene {

namespace shape {
class Node_stack;
}

class Worker;
struct Ray;

namespace prop {

class Prop;
struct Intersection;

class BVH_wrapper {
 public:
  using Sampler_filter = material::Sampler_settings::Filter;

  bvh::Tree<Prop>& tree();

  void set_infinite_props(std::vector<Prop*> const& infite_props);

  math::AABB const& aabb() const;

  bool intersect(Ray& ray, shape::Node_stack& node_stack, Intersection& intersection) const;

  bool intersect_fast(Ray& ray, shape::Node_stack& node_stack, Intersection& intersection) const;

  bool intersect(Ray& ray, shape::Node_stack& node_stack, float& epsilon) const;

  bool intersect_p(Ray const& ray, shape::Node_stack& node_stack) const;

  float opacity(Ray const& ray, Sampler_filter filter, Worker const& worker) const;

  float3 thin_absorption(Ray const& ray, Sampler_filter filter, Worker const& worker) const;

 private:
  bvh::Tree<Prop> tree_;

  uint32_t num_infinite_props_;
  Prop* const* infinite_props_;
};

}  // namespace prop
}  // namespace scene

#endif
