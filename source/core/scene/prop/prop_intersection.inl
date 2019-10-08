#ifndef SU_CORE_SCENE_PROP_INTERSECTION_INL
#define SU_CORE_SCENE_PROP_INTERSECTION_INL

#include "base/math/vector3.inl"
#include "prop.hpp"
#include "prop_intersection.hpp"
#include "scene/material/material.hpp"
#include "scene/scene.inl"
#include "scene/scene_ray.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::prop {

inline material::Material const* Intersection::material(Worker const& worker) const noexcept {
    return worker.scene().prop_material(prop, geo.part);
}

inline uint32_t Intersection::light_id(Worker const& worker) const noexcept {
    return worker.scene().prop_light_id(prop, geo.part);
}

inline float Intersection::area(Worker const& worker) const noexcept {
    return worker.scene().prop_area(prop, geo.part);
}

inline float Intersection::opacity(uint64_t time, Filter filter, Worker const& worker) const
    noexcept {
    return material(worker)->opacity(geo.uv, time, filter, worker);
}

inline float3 Intersection::thin_absorption(float3 const& wo, uint64_t time, Filter filter,
                                            Worker const& worker) const noexcept {
    return material(worker)->thin_absorption(wo, geo.geo_n, geo.uv, time, filter, worker);
}

inline material::Sample const& Intersection::sample(float3 const& wo, Ray const& ray, Filter filter,
                                                    bool avoid_caustics, sampler::Sampler& sampler,
                                                    Worker const& worker) const noexcept {
    material::Material const* material = Intersection::material(worker);

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

    rs.uv   = geo.uv;
    rs.area = area(worker);
    rs.ior  = worker.ior_outside(wo, *this);

    rs.subsurface     = subsurface;
    rs.avoid_caustics = avoid_caustics;

    return material->sample(wo, ray, rs, filter, sampler, worker);
}

inline bool Intersection::same_hemisphere(float3 const& v) const noexcept {
    return dot(geo.geo_n, v) > 0.f;
}

}  // namespace scene::prop

#endif
