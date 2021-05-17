#ifndef SU_CORE_SCENE_PROP_BVH_WRAPPER_HPP
#define SU_CORE_SCENE_PROP_BVH_WRAPPER_HPP

#include "scene/bvh/scene_bvh_tree.hpp"
#include "scene/material/sampler_settings.hpp"

#include <vector>

namespace scene {

namespace shape {
enum class Interpolation;
class Node_stack;
}  // namespace shape

class Worker;
struct Ray;

struct Result1 {
    bool  valid;
    float value;
};

namespace prop {

class Prop;
struct Intersection;

class BVH_wrapper {
  public:
    using Interpolation = shape::Interpolation;
    using Filter = material::Sampler_settings::Filter;

    bvh::Tree& tree();

    void set_props(std::vector<uint32_t> const& infinite_props, std::vector<Prop> const& props);

    AABB aabb() const;

    bool intersect(Ray& ray, Worker& worker, Interpolation ipo, Intersection& isec) const;

    bool intersect_shadow(Ray& ray, Worker& worker, Intersection& isec) const;

    bool intersect_p(Ray const& ray, Worker& worker) const;

    Result1 visibility(Ray const& ray, Filter filter, Worker& worker) const;

    bool thin_absorption(Ray const& ray, Filter filter, Worker& worker, float3& ta) const;

  private:
    bvh::Tree tree_;

    uint32_t num_infinite_props_;

    uint32_t const* infinite_props_;

    Prop const* props_;
};

}  // namespace prop
}  // namespace scene

#endif
