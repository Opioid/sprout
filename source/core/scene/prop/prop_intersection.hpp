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

    material::Material const* material(Worker const& worker) const;

    uint32_t light_id(Worker const& worker) const;

    float opacity(uint64_t time, Filter filter, Worker const& worker) const;

    float3 thin_absorption(float3 const& wo, uint64_t time, Filter filter,
                           Worker const& worker) const;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Filter filter,
                                   bool avoid_caustics, sampler::Sampler& sampler,
                                   Worker const& worker) const;

    bool same_hemisphere(float3 const& v) const;

    shape::Intersection geo;

    uint32_t prop;

    bool subsurface;
};

}  // namespace prop
}  // namespace scene

#endif
