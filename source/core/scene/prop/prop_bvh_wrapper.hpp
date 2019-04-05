#ifndef SU_CORE_SCENE_PROP_BVH_WRAPPER_HPP
#define SU_CORE_SCENE_PROP_BVH_WRAPPER_HPP

#include "scene/bvh/scene_bvh_tree.hpp"
#include "scene/material/sampler_settings.hpp"

namespace scene {

namespace shape {
struct Normals;
class Node_stack;
enum class Visibility;

}  // namespace shape

class Worker;
struct Ray;

namespace prop {

class Prop;
struct Intersection;

class BVH_wrapper {
  public:
    using Filter     = material::Sampler_settings::Filter;
    using Visibility = shape::Visibility;

    bvh::Tree<Prop>& tree() noexcept;

    void set_infinite_props(std::vector<Prop*> const& infite_props) noexcept;

    AABB const& aabb() const noexcept;

    bool intersect(Ray& ray, shape::Node_stack& node_stack, Intersection& intersection) const
        noexcept;

    bool intersect_fast(Ray& ray, shape::Node_stack& node_stack, Intersection& intersection) const
        noexcept;

    bool intersect(Ray& ray, shape::Node_stack& node_stack, shape::Normals& normals) const noexcept;

    bool intersect_p(Ray const& ray, shape::Node_stack& node_stack) const noexcept;

    Visibility visibility(Ray const& ray, Filter filter, Worker const& worker, float& v) const
        noexcept;

    Visibility thin_absorption(Ray const& ray, Filter filter, Worker const& worker,
                               float3& ta) const noexcept;

  private:
    bvh::Tree<Prop> tree_;

    uint32_t     num_infinite_props_;
    Prop* const* infinite_props_;
};

}  // namespace prop
}  // namespace scene

#endif
