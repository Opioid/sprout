#include "tracking_multi.hpp"
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
#include "tracking.inl"

#include "base/debug/assert.hpp"

#include <iostream>

namespace rendering::integrator::volume {

using namespace scene::prop;

static inline void set_scattering(Intersection& intersection, Interface const* interface,
                                  float3 const& p) noexcept {
    intersection.prop       = interface->prop;
    intersection.geo.p      = p;
    intersection.geo.uv     = interface->uv;
    intersection.geo.part   = interface->part;
    intersection.subsurface = true;
}

Tracking_multi::Tracking_multi(rnd::Generator& rng) noexcept : Integrator(rng) {}

void Tracking_multi::prepare(Scene const& /*scene*/, uint32_t /*num_samples_per_pixel*/) noexcept {}

void Tracking_multi::start_pixel() noexcept {}

bool Tracking_multi::transmittance(Ray const& ray, Worker& worker, float3& tr) noexcept {
    return Tracking::transmittance(ray, rng_, worker, tr);
}

Event Tracking_multi::integrate(Ray& ray, Intersection& intersection, Filter filter, Worker& worker,
                                float3& li, float3& tr) noexcept {
    if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
        li = float3(0.f);
        tr = float3(1.f);
        return Event::Abort;
    }

    float const d = ray.max_t;

    // Not sure wether the first test still makes sense.
    // The second test avoids falsely reporting very long volume sections,
    // when in fact a very short intersection was missed.
    // However, this might cause problems if we ever want to support "infinite" volumes.

    if (scene::offset_f(ray.min_t) >= d || scene::Almost_ray_max_t <= d) {
        li = float3(0.f);
        tr = float3(1.f);
        return Event::Pass;
    }

    SOFT_ASSERT(!worker.interface_stack().empty());

    auto const interface = worker.interface_stack().top();

    auto const& material = *interface->material(worker);

    if (!material.is_scattering_volume()) {
        // Basically the "glass" case
        float3 const mu_a = material.absorption_coefficient(interface->uv, filter, worker);

        li = float3(0.f);
        tr = attenuation(d - ray.min_t, mu_a);
        return Event::Pass;
    }

    if (material.is_heterogeneous_volume()) {
        math::ray local_ray = texture_space_ray(ray, interface->prop, worker);

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
                    } else if (Event::Absorb == result) {
                        tr                 = w;
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

        tr = w;
        return any_greater_equal(w, Tracking::Abort_epsilon) ? event : Event::Abort;
    } else if (material.is_textured_volume()) {
        auto const mu = material.collision_coefficients(interface->uv, filter, worker);

        float3 w;
        Event  event = Event::Pass;

        if (float t; Tracking::tracking(ray, mu, rng_, t, w)) {
            set_scattering(intersection, interface, ray.point(t));
            event = Event::Scatter;
        }

        li = float3(0.f);
        tr = w;
        return event;
    } else {
        static bool constexpr decomposition = false;

        if (decomposition) {
            auto const cm = material.control_medium();

            float const minorant_mu_t = cm.minorant_mu_t();

            float const rc  = rng_.random_float();
            float const t_c = ray.min_t - std::log(1.f - rc) / minorant_mu_t;

            if (t_c > ray.max_t) {
                set_scattering(intersection, interface, ray.point(t_c));

                li = float3(0.f);
                tr = float3(cm.minorant_mu_s / minorant_mu_t);
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
                    li = float3(0.f);
                    tr = w;
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

                    li = float3(0.f);
                    tr = w * ws;
                    return Event::Scatter;
                } else {
                    float3 const wn = mu_n / (mt * pn);

                    SOFT_ASSERT(all_finite(wn));

                    w *= wn;
                }
            }
        } else {
            float3 w;

            Event event = Event::Pass;

            if (material.is_emissive(worker.scene())) {
                auto const cce = material.collision_coefficients_emission();

                float      t;
                auto const result = Tracking::tracking(ray, cce, rng_, t, w, li);

                tr = w;

                if (Event::Scatter == result) {
                    set_scattering(intersection, interface, ray.point(t));
                    event = Event::Scatter;
                } else if (Event::Absorb == result) {
                    ray.max_t = t;
                    return Event::Absorb;
                }

                return any_greater_equal(w, Tracking::Abort_epsilon) ? event : Event::Abort;
            } else {
                //                auto const mu = material.collision_coefficients();

                //                if (float t; Tracking::tracking(ray, mu, rng_, t, w)) {
                //                    set_scattering(intersection, interface, ray.point(t));
                //                    event = Event::Scatter;
                //                }

                //                li = float3(0.f);
                //                tr = w;
                //                return any_greater_equal(w, Tracking::Abort_epsilon) ? event :
                //                Event::Abort;

                // Tracking as reference
                /*
                auto const mu = material.collision_coefficients();

                float3 const extinction = mu.a + mu.s;

                float3 const scattering_albedo = mu.s / extinction;

                float const r = rng_.random_float();
                float const t = -std::log(1.f - r) / average(extinction);

                float const td = d - ray.min_t;

                if (t < td) {
                    float3 const p = ray.point(ray.min_t + t);

                    set_scattering(intersection, interface, p);

                    event = Event::Scatter;

                    float3 const w = exp(-(t)*extinction);

                    float3 const pdf = extinction * w;

                    float3 const weight = scattering_albedo * extinction / pdf;

                    tr = w * weight; // This should cancel out to 1.f
                    li = float3(0.f);
                } else {
                    tr    = float3(1.f);  // exp(-(td) * extinction);
                    li    = float3(0.f);
                    event = Event::Pass;
                }

                return event;
                */

                // Decomposition tracking

                li = float3(0.f);

                auto const                mu = material.collision_coefficients();
                scene::material::CM const ccm(mu);

                scene::material::CC const cm{float3(ccm.minorant_mu_a), float3(ccm.minorant_mu_s)};

                scene::material::CC const rm{mu.a - cm.a, mu.s - cm.s};

                float3 const mu_t = mu.a + mu.s;

                float const mt  = max_component(mu_t);
                float const imt = 1.f / mt;

                float3 const mu_n0 = float3(mt) - mu_t;

                float3 lw(1.f);

                for (float t = ray.min_t;;) {
                    float const r0 = rng_.random_float();
                    t -= std::log(1.f - r0) * imt;
                    if (t > d) {
                        tr = lw;

                        return Event::Pass;
                    }

                    float const pc_a = cm.a[0] * imt;
                    float const pc_s = cm.s[0] * imt;

                    float const cm_t   = cm.a[0] + cm.s[0];
                    float const rm_t   = max_component(lw * (rm.a + rm.s));
                    float const factor = 1.f - cm_t * imt;

                    float const mu_n = std::max(mt - cm_t - rm_t, 0.f);

                    float const ma = average(rm.a * lw);
                    float const ms = average(rm.s * lw);
                    float const mn = average(mu_n0 * lw);

                    //                float const pr_a = factor * (rm.a[0] / (rm.a[0] + rm.s[0] +
                    //                mu_n)); float const pr_s = factor * (rm.s[0] / (rm.a[0] +
                    //                rm.s[0] + mu_n)); float const p_n  = factor * (mu_n / (rm.a[0]
                    //                + rm.s[0] + mu_n));

                    //                    float const pr_a = factor * (ma / (ma + ms + mn));
                    //                    float const pr_s = factor * (ms / (ma + ms + mn));
                    //                    float const p_n  = factor * (mn / (ma + ms + mn));

                    float const div = ms + mn;

                    float const pr_s = div > 0.f ? factor * (ms / div) : 0.f;
                    float const p_n  = div > 0.f ? factor * (mn / div) : 0.f;

                    //     float const p_n = 1.f - (pc_s + pr_s);

                    //                    float const mc = ma + ms + mn;
                    //                    if (mc < 1e-10f) {
                    //                        tr  = lw;
                    //                        li = float3(0.f);
                    //                        return Event::Pass;
                    //                    }

                    //      float const c = 1.f / mc;

                    //                    float const pa = ma * c;
                    //                    float const ps = ms * c;
                    //                    float const pn = mn * c;

                    float f = 0.f;

                    float const r1 = rng_.random_float();

                    //                   if (r1 < (f += pc_a)) {
                    //                        return Event::Absorb;
                    //                    } else

                    float const tp = pc_s + pr_s + p_n;

                    if (r1 < (f += pc_s)) {
                        lw *= cm.s[0] / (mt * (pc_s));
                        tr = lw;

                        float3 const p = ray.point(t);
                        set_scattering(intersection, interface, p);

                        return Event::Scatter;
                        //                    } else if (r1 < (f += pr_a)) {
                        //                        return Event::Absorb;
                    } else if (r1 < (f += pr_s)) {
                        lw *= rm.s / (mt * pr_s);
                        tr = lw;

                        float3 const p = ray.point(t);
                        set_scattering(intersection, interface, p);

                        return Event::Scatter;
                    } else if (p_n > 0.f) {
                        lw *= mu_n0 / (mt * p_n);
                    }
                }

                /*
                                li = float3(0.f);

                                auto const mu = material.collision_coefficients();

                                float3 const mu_t = mu.a + mu.s;

                                float const mt  = max_component(mu_t);
                                float const imt = 1.f / mt;

                                float3 const mu_n = float3(mt) - mu_t;

                                float3 lw(1.f);

                                for (float t = ray.min_t, d = ray.max_t;;) {
                                    float const r0 = rng_.random_float();
                                    t -= std::log(1.f - r0) * imt;
                                    if (t > d) {
                                        tr = lw;
                                        return Event::Pass;
                                    }

                                    float const ms = average(mu.s * lw);
                                    float const mn = average(mu_n * lw);

                                    float const mc = ms + mn;
                                    if (mc < 1e-10f) {
                                        tr = lw;
                                        return Event::Pass;
                                    }

                                    float const c = 1.f / mc;

                                    float const ps = ms * c;
                                    float const pn = mn * c;

                                    if (float const r1 = rng_.random_float(); (r1 <= 1.f - pn) & (ps
                   > 0.f)) { float3 const ws = mu.s / (mt * ps);


                                        tr     = lw * ws;


                                        float3 const p = ray.point(t);
                                        set_scattering(intersection, interface, p);

                                        return Event::Scatter;

                                    } else {
                                        float3 const wn = mu_n / (mt * pn);

                                        SOFT_ASSERT(all_finite(wn));

                                        lw *= wn;
                                    }
                                }
                */
            }
        }
    }
}

Tracking_multi_factory::Tracking_multi_factory(uint32_t num_integrators) noexcept
    : integrators_(memory::allocate_aligned<Tracking_multi>(num_integrators)) {}

Tracking_multi_factory::~Tracking_multi_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* Tracking_multi_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Tracking_multi(rng);
}

}  // namespace rendering::integrator::volume
