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
    using Filter  = material::Sampler_settings::Filter;
    using Sample  = material::Sample;
    using Sampler = sampler::Sampler;

    material::Material const* material(Worker const& worker) const;

    uint32_t light_id(Worker const& worker) const;

    bool visible_in_camera(Worker const& worker) const;

    float opacity(uint64_t time, Filter filter, Worker const& worker) const;

    float3 thin_absorption(float3 const& wo, uint64_t time, Filter filter,
                           Worker const& worker) const;

    Sample const& sample(float3 const& wo, Ray const& ray, Filter filter, float alpha,
                         bool avoid_caustics, Sampler& sampler, Worker& worker) const;

    bool same_hemisphere(float3 const& v) const;

    shape::Intersection geo;

    uint32_t prop;

    bool subsurface;
};

}  // namespace prop
}  // namespace scene

#endif
