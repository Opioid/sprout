#ifndef SU_CORE_SCENE_PROP_INTERSECTION_INL
#define SU_CORE_SCENE_PROP_INTERSECTION_INL

#include "base/math/vector3.inl"
#include "prop.hpp"
#include "prop_intersection.hpp"
#include "scene/material/material.inl"
#include "scene/ray_offset.inl"
#include "scene/scene.inl"
#include "scene/scene_ray.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::prop {

inline material::Material const* Intersection::material(Worker const& worker) const {
    return worker.scene().prop_material(prop, geo.part);
}

inline shape::Shape const* Intersection::shape(Worker const& worker) const {
    return worker.scene().prop_shape(prop);
}

inline uint32_t Intersection::light_id(Worker const& worker) const {
    return worker.scene().prop_light_id(prop, geo.part);
}

inline bool Intersection::visible_in_camera(Worker const& worker) const {
    return worker.scene().prop(prop)->visible_in_camera();
}

inline float Intersection::opacity(Filter filter, Worker const& worker) const {
    return material(worker)->opacity(geo.uv, filter, worker);
}

inline float3 Intersection::thin_absorption(float3_p wo, Filter filter,
                                            Worker const& worker) const {
    return material(worker)->thin_absorption(wo, geo.geo_n, geo.uv, filter, worker);
}

inline material::Sample const& Intersection::sample(float3_p wo, Ray const& ray, Filter filter,
                                                    float alpha, bool avoid_caustics,
                                                    Sampler& sampler, Worker& worker) const {
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

    rs.uv = geo.uv;

    rs.ior   = worker.ior_outside(wo, *this);
    rs.alpha = alpha;

    rs.prop      = prop;
    rs.part      = geo.part;
    rs.primitive = geo.primitive;

    rs.filter = filter;

    rs.subsurface     = subsurface;
    rs.avoid_caustics = avoid_caustics;

    return material->sample(wo, ray, rs, sampler, worker);
}

inline bool Intersection::evaluate_radiance(float3_p wo, Filter filter, Worker& worker,
                                            float3& radiance, bool& pure_emissive) const {
    material::Material const* material = Intersection::material(worker);

    pure_emissive = material->is_pure_emissive();

    if (!material->is_two_sided() && !same_hemisphere(wo)) {
        return false;
    }

    float const extent = worker.scene().light_area(prop, geo.part);

    radiance = material->evaluate_radiance(wo, geo.geo_n, float3(geo.uv), extent, filter, worker);

    return true;
}

inline bool Intersection::same_hemisphere(float3_p v) const {
    return dot(geo.geo_n, v) > 0.f;
}

inline float3 Intersection::offset_p(bool translucent) const {
    float3 const p = geo.p;

    if (subsurface) {
        return float3(p[0], p[1], p[2], 0.f);
    }

    if (translucent) {
        float const t = max_component(abs(p * geo.geo_n));
        float const d = offset_f(t) - t;

        return float3(p[0], p[1], p[2], d);
    }

    return offset_ray(p, geo.geo_n);
}

inline float3 Intersection::offset_p(float3_p wi) const {
    float3 const p = geo.p;

    if (subsurface) {
        return float3(p[0], p[1], p[2], 0.f);
    }

    return offset_ray(p, same_hemisphere(wi) ? geo.geo_n : -geo.geo_n);
}

}  // namespace scene::prop

#endif
