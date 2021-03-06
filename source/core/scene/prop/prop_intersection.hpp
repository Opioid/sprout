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

namespace shape {
class Shape;
}

namespace prop {

class Prop;

struct Intersection {
    using Filter  = material::Sampler_settings::Filter;
    using Sample  = material::Sample;
    using Sampler = sampler::Sampler;

    material::Material const* material(Worker const& worker) const;

    shape::Shape const* shape(Worker const& worker) const;

    uint32_t light_id(Worker const& worker) const;

    bool visible_in_camera(Worker const& worker) const;

    float opacity(Filter filter, Worker const& worker) const;

    Sample const& sample(float3_p wo, Ray const& ray, Filter filter, float alpha,
                         bool avoid_caustics, Sampler& sampler, Worker& worker) const;

    bool evaluate_radiance(float3_p wo, Filter filter, Worker& worker, float3& radiance,
                           bool& pure_emissive) const;

    bool same_hemisphere(float3_p v) const;

    float3 offset_p(float3_p v) const;
    float3 offset_p(float3_p geo_n, bool translucent) const;

    shape::Intersection geo;

    uint32_t prop;

    bool subsurface;
};

}  // namespace prop
}  // namespace scene

#endif
