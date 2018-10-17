#ifndef SU_CORE_SCENE_PROP_INTERSECTION_INL
#define SU_CORE_SCENE_PROP_INTERSECTION_INL

#include "base/math/vector3.inl"
#include "prop.hpp"
#include "prop_intersection.hpp"
#include "scene/material/material.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"

namespace scene::prop {

inline bool Intersection::hit() const noexcept {
    return nullptr != prop;
}

inline material::Material const* Intersection::material() const noexcept {
    return prop->material(geo.part);
}

inline uint32_t Intersection::light_id() const noexcept {
    return prop->light_id(geo.part);
}

inline float Intersection::area() const noexcept {
    return prop->area(geo.part);
}

inline float Intersection::opacity(uint64_t time, Sampler_filter filter, Worker const& worker) const
    noexcept {
    return material()->opacity(geo.uv, time, filter, worker);
}

inline float3 Intersection::thin_absorption(float3 const& wo, uint64_t time, Sampler_filter filter,
                                            Worker const& worker) const noexcept {
    return material()->thin_absorption(wo, geo.geo_n, geo.uv, time, filter, worker);
}

inline material::Sample const& Intersection::sample(float3 const& wo, Ray const& ray,
                                                    Sampler_filter filter, bool avoid_caustics,
                                                    sampler::Sampler& sampler, Worker const& worker,
                                                    uint32_t depth) const noexcept {
    material::Material const* material = Intersection::material();

    Renderstate rs;
    rs.p = geo.p;
    rs.t = geo.t;
    rs.b = geo.b;

    if (material->is_two_sided() && !same_hemisphere(wo)) {
        rs.n     = -geo.n;
        rs.geo_n = -geo.geo_n;
    } else {
        rs.n     = geo.n;
        rs.geo_n = geo.geo_n;
    }

    rs.time = ray.time;

    rs.uv             = geo.uv;
    rs.area           = area();
    rs.ior            = worker.ior_outside(wo, *this);
    rs.wavelength     = ray.wavelength;
    rs.subsurface     = subsurface;
    rs.avoid_caustics = avoid_caustics;

    return material->sample(wo, rs, filter, sampler, worker, depth);
}

inline bool Intersection::same_hemisphere(float3 const& v) const noexcept {
    return math::dot(geo.geo_n, v) > 0.f;
}

}  // namespace scene::prop

#endif
