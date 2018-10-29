#ifndef SU_CORE_SCENE_PROP_INTERSECTION_HPP
#define SU_CORE_SCENE_PROP_INTERSECTION_HPP

#include "scene/material/sampler_settings.hpp"
#include "scene/shape/shape_intersection.hpp"

namespace sampler {
class Sampler;
}

namespace scene {

struct Ray;
class Worker;

namespace material {

class Sample;
class Material;

}  // namespace material

namespace prop {

class Prop;

struct Intersection {
    using Filter = material::Sampler_settings::Filter;

    bool hit() const noexcept;

    material::Material const* material() const noexcept;

    bool is_light() const noexcept;

    uint32_t light_id() const noexcept;

    float area() const noexcept;

    float opacity(uint64_t time, Filter filter, Worker const& worker) const noexcept;

    float3 thin_absorption(float3 const& wo, uint64_t time, Filter filter,
                           Worker const& worker) const noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Filter filter,
                                   bool avoid_caustics, sampler::Sampler& sampler,
                                   Worker const& worker, uint32_t depth = 0) const noexcept;

    bool same_hemisphere(float3 const& v) const noexcept;

    Prop const* prop;

    shape::Intersection geo;

    bool subsurface;
};

}  // namespace prop
}  // namespace scene

#endif
