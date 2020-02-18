#include "tracking_multi_pre.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/volumetric/volumetric_octree.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape.hpp"
#include "tracking.hpp"

#include "base/debug/assert.hpp"

namespace rendering::integrator::volume {

Tracking_multi_pre::Tracking_multi_pre(rnd::Generator& rng) : Integrator(rng) {}

void Tracking_multi_pre::prepare(Scene const& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Tracking_multi_pre::start_pixel() {}

bool Tracking_multi_pre::transmittance(Ray const& ray, Worker& worker, float3& transmittance) {
    return Tracking::transmittance(ray, rng_, worker, transmittance);
}

Event Tracking_multi_pre::integrate(Ray& ray, Intersection& intersection, Filter filter,
                                    Worker& worker, float3& li, float3& transmittance) {
    if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
        li            = float3(0.f);
        transmittance = float3(1.f);
        return Event::Abort;
    }

    float const d = ray.max_t;

    // Not sure wether the first test still makes sense.
    // The second test avoids falsely reporting very long volume sections,
    // when in fact a very short intersection was missed.
    // However, this might cause problems if we ever want to support "infinite" volumes.

    if (scene::offset_f(ray.min_t) >= d || scene::Almost_ray_max_t <= d) {
        li            = float3(0.f);
        transmittance = float3(1.f);
        return Event::Pass;
    }

    SOFT_ASSERT(!worker.interface_stack().empty());

    auto const interface = worker.interface_stack().top();

    auto const& material = *interface->material(worker);

    if (!material.is_scattering_volume()) {
        // Basically the "glass" case
        float3 const mu_a = material.absorption_coefficient(interface->uv, filter, worker);

        li            = float3(0.f);
        transmittance = attenuation(d - ray.min_t, mu_a);
        return Event::Pass;
    }

    if (material.is_heterogeneous_volume()) {
        Transformation temp;
        auto const&    transformation = worker.scene().prop_transformation_at(interface->prop,
                                                                           ray.time, temp);

        float3 const local_origin = transformation.world_to_object_point(ray.origin);
        float3 const local_dir    = transformation.world_to_object_vector(ray.direction);

        auto const shape = worker.scene().prop_shape(interface->prop);

        float3 const origin = shape->object_to_texture_point(local_origin);
        float3 const dir    = shape->object_to_texture_vector(local_dir);

        math::ray local_ray(origin, dir, ray.min_t, d);

        auto const& tree = *material.volume_tree();

        float const srs = material.similarity_relation_scale(ray.depth);

        float3 w(1.f);

        li = float3(0.f);

        Event event = Event::Pass;

        if (material.is_emissive(worker.scene())) {
            for (; local_ray.min_t < d;) {
                if (Tracking::CM cm; tree.intersect(local_ray, cm)) {
                    cm.minorant_mu_s *= srs;
                    cm.majorant_mu_s *= srs;

                    float      t;
                    auto const result = Tracking::tracking(local_ray, cm, material, srs, filter,
                                                           rng_, worker, t, w, li);

                    if (Event::Scatter == result) {
                        set_scattering(intersection, interface, ray.point(t));
                        event = Event::Scatter;
                        break;
                    }

                    if (Event::Absorb == result) {
                        transmittance      = w;
                        ray.max_t          = t;
                        intersection.geo.p = local_ray.point(t);
                        return Event::Absorb;
                    }
                }

                SOFT_ASSERT(scene::offset_f(local_ray.max_t) > local_ray.min_t);

                local_ray.min_t = scene::offset_f(local_ray.max_t);
                local_ray.max_t = d;
            }
        } else {
            for (; local_ray.min_t < d;) {
                if (Tracking::CM cm; tree.intersect(local_ray, cm)) {
                    cm.minorant_mu_s *= srs;
                    cm.majorant_mu_s *= srs;

                    if (float t; Tracking::tracking(local_ray, cm, material, srs, filter, rng_,
                                                    worker, t, w)) {
                        set_scattering(intersection, interface, ray.point(t));
                        event = Event::Scatter;
                        break;
                    }
                }

                SOFT_ASSERT(scene::offset_f(local_ray.max_t) > local_ray.min_t);

                local_ray.min_t = scene::offset_f(local_ray.max_t);
                local_ray.max_t = d;
            }
        }

        transmittance = w;
        return any_greater_equal(w, Tracking::Abort_epsilon) ? event : Event::Abort;
    }

    if (material.is_textured_volume()) {
        auto const mu = material.collision_coefficients(interface->uv, filter, worker);

        float3 w;
        Event  event = Event::Pass;

        if (float t; Tracking::tracking(ray, mu, rng_, t, w)) {
            set_scattering(intersection, interface, ray.point(t));
            event = Event::Scatter;
        }

        li            = float3(0.f);
        transmittance = w;
        return event;
    }

    static bool constexpr decomposition = false;

    if (decomposition) {
        auto const cm = material.control_medium();

        float const minorant_mu_t = cm.minorant_mu_t();

        float const rc  = rng_.random_float();
        float const t_c = ray.min_t - std::log(1.f - rc) / minorant_mu_t;

        if (t_c > ray.max_t) {
            set_scattering(intersection, interface, ray.point(t_c));

            li            = float3(0.f);
            transmittance = float3(cm.minorant_mu_s / minorant_mu_t);

            return Event::Scatter;
        }

        float const mt = cm.majorant_mu_t() - minorant_mu_t;

        auto mu = material.collision_coefficients();
        mu.a -= cm.minorant_mu_a;
        mu.s -= cm.minorant_mu_s;

        float3 const mu_t = mu.a + mu.s;

        float3 const mu_n = float3(mt) - mu_t;

        float const imt = 1.f / mt;

        float3 w(1.f);

        for (float t = ray.min_t;;) {
            float const r0 = rng_.random_float();
            t -= std::log(1.f - r0) * imt;
            if (t > d) {
                li            = float3(0.f);
                transmittance = w;
                return Event::Pass;
            }

            float const ms = average(mu.s * w);
            float const mn = average(mu_n * w);
            float const c  = 1.f / (ms + mn);

            float const ps = ms * c;
            float const pn = mn * c;

            float const r1 = rng_.random_float();
            if (r1 <= 1.f - pn && ps > 0.f) {
                set_scattering(intersection, interface, ray.point(t));

                float3 const ws = mu.s / (mt * ps);

                li            = float3(0.f);
                transmittance = w * ws;

                return Event::Scatter;
            }

            float3 const wn = mu_n / (mt * pn);

            SOFT_ASSERT(all_finite(wn));

            w *= wn;
        }
    } else {
        float3 w;

        Event event = Event::Pass;

        if (material.is_emissive(worker.scene())) {
            auto const cce = material.collision_coefficients_emission();

            float      t;
            auto const result = Tracking::tracking(ray, cce, rng_, t, w, li);

            transmittance = w;

            if (Event::Scatter == result) {
                set_scattering(intersection, interface, ray.point(t));
                event = Event::Scatter;
            } else if (Event::Absorb == result) {
                ray.max_t = t;
                return Event::Absorb;
            }

            return any_greater_equal(w, Tracking::Abort_epsilon) ? event : Event::Abort;
        }

        auto const mu = material.collision_coefficients();

        if (float t; Tracking::tracking(ray, mu, rng_, t, w)) {
            set_scattering(intersection, interface, ray.point(t));
            event = Event::Scatter;
        }

        li            = float3(0.f);
        transmittance = w;
        return any_greater_equal(w, Tracking::Abort_epsilon) ? event : Event::Abort;
    }
}

void Tracking_multi_pre::set_scattering(Intersection& intersection, Interface const* interface,
                                        float3 const& p) {
    intersection.prop       = interface->prop;
    intersection.geo.p      = p;
    intersection.geo.uv     = interface->uv;
    intersection.geo.part   = interface->part;
    intersection.subsurface = true;
}

Tracking_multi_pre_pool::Tracking_multi_pre_pool(uint32_t num_integrators)
    : Typed_pool<Tracking_multi_pre>(num_integrators) {}

Integrator* Tracking_multi_pre_pool::get(uint32_t id, rnd::Generator& rng) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Tracking_multi_pre(rng);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::volume
