#include "tracking_multi.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/integrator/integrator.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material.inl"
#include "scene/material/volumetric/volumetric_octree.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/ray_offset.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape.hpp"
#include "tracking.inl"

#include "base/debug/assert.hpp"

namespace rendering::integrator::volume {

using namespace scene::prop;

static inline void set_scattering(Intersection& isec, Interface const* interface, float3_p p) {
    isec.prop       = interface->prop;
    isec.geo.p      = p;
    isec.geo.uv     = interface->uv;
    isec.geo.part   = interface->part;
    isec.subsurface = true;
}

Tracking_multi::Tracking_multi() = default;

void Tracking_multi::start_pixel(rnd::Generator& /*rng*/) {}

bool Tracking_multi::transmittance(Ray const& ray, Filter filter, Worker& worker, float3& tr) {
    return Tracking::transmittance(ray, filter, worker, tr);
}

Event Tracking_multi::integrate(Ray& ray, Intersection& isec, Filter filter, Worker& worker,
                                float3& li, float3& tr) {
    if (!worker.intersect_and_resolve_mask(ray, isec, filter)) {
        li = float3(0.f);
        tr = float3(1.f);
        return Event::Abort;
    }

    SOFT_ASSERT(!worker.interface_stack().empty());

    auto stack = worker.interface_stack();

    auto const interface = worker.interface_stack().top();

    float const d = ray.max_t();

    // This test is intended to catch corner cases where we actually left the scattering medium,
    // but the intersection point was too close to detect.
    bool missed = false;

    if (scene::Almost_ray_max_t <= d) {
        missed = true;
    } else if (!interface->matches(isec) || !isec.same_hemisphere(ray.direction)) {
        float3 const v = -ray.direction;

        Ray tray(isec.offset_p(v), v, 0.f, scene::Ray_max_t, 0, 0.f, ray.time);
        if (Normals normals; worker.intersect(interface->prop, tray, normals)) {
            if (dot(normals.geo_n, v) <= 0.f) {
                missed = true;
            }
        }
    }

    if (missed) {
        stack.pop();
        li = float3(0.f);
        tr = float3(1.f);
        return Event::Pass;
    }

    auto const& material = *interface->material(worker);

    auto& rng = worker.rng();

    if (!material.is_scattering_volume()) {
        // Basically the "glass" case
        float3 const mu_a = material.collision_coefficients(interface->uv, filter, worker).a;

        li = float3(0.f);
        tr = attenuation(d - ray.min_t(), mu_a);
        return Event::Pass;
    }

    if (material.is_heterogeneous_volume()) {
        math::ray local_ray = texture_space_ray(ray, interface->prop, worker);

        auto const& tree = *material.volume_tree();

        float const srs = material.similarity_relation_scale(ray.depth);

        float3 w(1.f);

        li = float3(0.f);

        Event event = Event::Pass;

        if (material.is_emissive()) {
            for (; local_ray.min_t() < d;) {
                if (Tracking::CM cm; tree.intersect(local_ray, cm)) {
                    cm.minorant_mu_s *= srs;
                    cm.majorant_mu_s *= srs;

                    float      t;
                    auto const result = Tracking::tracking(local_ray, cm, material, srs, filter,
                                                           worker, t, w, li);

                    if (Event::Scatter == result) {
                        set_scattering(isec, interface, ray.point(t));
                        event = Event::Scatter;
                        break;
                    }

                    if (Event::Absorb == result) {
                        tr          = w;
                        ray.max_t() = t;
                        // This is in local space on purpose!
                        isec.geo.p = local_ray.point(t);
                        return Event::Absorb;
                    }
                }

                SOFT_ASSERT(scene::offset_f(local_ray.max_t()) > local_ray.min_t());

                local_ray.min_t() = scene::offset_f(local_ray.max_t());
                local_ray.max_t() = d;
            }
        } else {
            for (; local_ray.min_t() < d;) {
                if (Tracking::CM cm; tree.intersect(local_ray, cm)) {
                    cm.minorant_mu_s *= srs;
                    cm.majorant_mu_s *= srs;

                    if (float t;
                        Tracking::tracking(local_ray, cm, material, srs, filter, worker, t, w)) {
                        set_scattering(isec, interface, ray.point(t));
                        event = Event::Scatter;
                        break;
                    }
                }

                SOFT_ASSERT(scene::offset_f(local_ray.max_t()) > local_ray.min_t());

                local_ray.min_t() = scene::offset_f(local_ray.max_t());
                local_ray.max_t() = d;
            }
        }

        tr = w;
        return any_greater_equal(w, Tracking::Abort_epsilon) ? event : Event::Abort;
    }

    float3 w;
    Event  event = Event::Pass;

    if (material.is_emissive()) {
        auto const cce = material.collision_coefficients_emission();

        float      t;
        auto const result = Tracking::tracking(ray, cce, rng, t, w, li);

        tr = w;

        if (Event::Scatter == result) {
            set_scattering(isec, interface, ray.point(t));
            event = Event::Scatter;
        } else if (Event::Absorb == result) {
            ray.max_t() = t;
            return Event::Absorb;
        }

        return any_greater_equal(w, Tracking::Abort_epsilon) ? event : Event::Abort;
    }

    auto const mu = material.is_textured_volume()
                        ? material.collision_coefficients(interface->uv, filter, worker)
                        : material.collision_coefficients();

    if (float t; Tracking::tracking(ray, mu, rng, t, w)) {
        set_scattering(isec, interface, ray.point(t));
        event = Event::Scatter;
    }

    li = float3(0.f);
    tr = w;
    return any_greater_equal(w, Tracking::Abort_epsilon) ? event : Event::Abort;

    // Tracking as reference
    /*
    auto const mu = material.collision_coefficients();

    float3 const attenuation = mu.a + mu.s;

    float3 const scattering_albedo = mu.s / attenuation;

    float const r = rng_.random_float();
    float const t = -std::log(1.f - r) / average(attenuation);

    float const td = d - ray.min_t;

    if (t < td) {
        float3 const p = ray.point(ray.min_t + t);

        set_scattering(isec, interface, p);

        event = Event::Scatter;

        float3 const w = exp(-(t)*attenuation);

        float3 const pdf = attenuation * w;

        float3 const weight = scattering_albedo * attenuation / pdf;

        tr = w * weight; // This should cancel out to 1.f
        li = float3(0.f);
    } else {
        tr    = float3(1.f);  // exp(-(td) * attenuation);
        li    = float3(0.f);
        event = Event::Pass;
    }

    return event;
    */
}

Tracking_multi_pool::Tracking_multi_pool(uint32_t num_integrators)
    : Typed_pool<Tracking_multi, Integrator>(num_integrators) {}

Integrator* Tracking_multi_pool::create(uint32_t id, uint32_t /*max_samples_per_pixel*/) const {
    return new (&integrators_[id]) Tracking_multi();
}

}  // namespace rendering::integrator::volume
