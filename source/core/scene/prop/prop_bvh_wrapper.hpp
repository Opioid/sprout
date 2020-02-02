#ifndef SU_CORE_SCENE_PROP_BVH_WRAPPER_HPP
#define SU_CORE_SCENE_PROP_BVH_WRAPPER_HPP

#include "scene/bvh/scene_bvh_tree.hpp"
#include "scene/material/sampler_settings.hpp"

#include <vector>

namespace scene {

namespace shape {
struct Normals;
class Node_stack;
}  // namespace shape

class Worker;
struct Ray;

namespace prop {

class Prop;
struct Intersection;

class BVH_wrapper {
  public:
    using Filter = material::Sampler_settings::Filter;

    bvh::Tree& tree();

    void set_props(std::vector<uint32_t> const& infinite_props, std::vector<Prop> const& props);

    AABB const& aabb() const;

    bool intersect(Ray& ray, Worker const& worker, Intersection& intersection) const;

    bool intersect_nsf(Ray& ray, Worker const& worker, Intersection& intersection) const;

    bool intersect(Ray& ray, Worker const& worker, shape::Normals& normals) const;

    bool intersect_p(Ray const& ray, Worker const& worker) const;

    bool visibility(Ray const& ray, Filter filter, Worker const& worker, float& v) const;

    bool thin_absorption(Ray const& ray, Filter filter, Worker const& worker, float3& ta) const;

  private:
    bvh::Tree tree_;

    uint32_t num_infinite_props_;

    uint32_t const* infinite_props_;

    Prop const* props_;
};

}  // namespace prop
}  // namespace scene

#endif
