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
    using Sampler_filter = material::Sampler_settings::Filter;

    bool hit() const;

    material::Material const* material() const;

    bool is_light() const;

    uint32_t light_id() const;

    float area() const;

    float opacity(float time, Sampler_filter filter, Worker const& worker) const;

    float3 thin_absorption(float3 const& wo, float const time, Sampler_filter filter,
                           Worker const& worker) const;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Sampler_filter filter,
                                   bool avoid_caustics, sampler::Sampler& sampler,
                                   Worker const& worker, uint32_t depth = 0) const;

    bool same_hemisphere(float3 const& v) const;

    Prop const* prop;

    shape::Intersection geo;

    bool subsurface;
};

}  // namespace prop
}  // namespace scene

#endif
