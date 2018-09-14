#include "tracking_multi.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/entity/composed_transformation.inl"
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

Tracking_multi::Tracking_multi(rnd::Generator& rng, take::Settings const& take_settings) noexcept
    : Integrator(rng, take_settings) {}

void Tracking_multi::prepare(Scene const& /*scene*/, uint32_t /*num_samples_per_pixel*/) noexcept {}

void Tracking_multi::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) noexcept {}
/*
static inline void max_probabilities(float mt,
                                                                         float3 const& mu_a,
                                                                         float3 const& mu_s,
                                                                         float3 const& mu_n,
                                                                         float& pa, float& ps,
float& pn, float3& wa, float3& ws, float3& wn) { float const ma = math::max_component(mu_a); float
const ms = math::max_component(mu_s); float const mn = math::max_component(mu_n); float const c
= 1.f / (ma + ms + mn);

        pa = ma * c;
        ps = ms * c;
        pn = mn * c;

        wa = (mu_a / (mt * pa));
        ws = (mu_s / (mt * ps));
        wn = (mu_n / (mt * pn));
}

static inline void max_history_probabilities(float mt,
                                                                                         float3
const& mu_a, float3 const& mu_s, float3 const& mu_n, float3 const& w, float& pa, float& ps, float&
pn, float3& wa, float3& ws, float3& wn) { float const ma = math::max_component(mu_a * w); float
const ms = math::max_component(mu_s * w); float const mn = math::max_component(mu_n * w); float
const c = 1.f / (ma + ms + mn);

        pa = ma * c;
        ps = ms * c;
        pn = mn * c;

        wa = (mu_a / (mt * pa));
        ws = (mu_s / (mt * ps));
        wn = (mu_n / (mt * pn));
}

static inline void max_history_probabilities(float mt,
                                                                                         float3
const& mu_a, float3 const& mu_s, float3 const& mu_n, float3 const& w, float& pn, float3& wn) { float
const ma = math::max_component(mu_a * w); float const ms = math::max_component(mu_s * w); float
const mn = math::max_component(mu_n * w); float const c = 1.f / (ma + ms + mn);

        pn = mn * c;

        wn = (mu_n / (mt * pn));
}

static inline void avg_probabilities(float mt,
                                                                         float3 const& mu_a,
                                                                         float3 const& mu_s,
                                                                         float3 const& mu_n,
                                                                         float& pa, float& ps,
float& pn, float3& wa, float3& ws, float3& wn) { float const ma = math::average(mu_a); float const
ms = math::average(mu_s); float const mn = math::average(mu_n); float const c = 1.f / (ma + ms +
mn);

        pa = ma * c;
        ps = ms * c;
        pn = mn * c;

        wa = (mu_a / (mt * pa));
        ws = (mu_s / (mt * ps));
        wn = (mu_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt,
                                                                                         float3
const& mu_a, float3 const& mu_s, float3 const& mu_n, float3 const& w, float& pa, float& ps, float&
pn, float3& wa, float3& ws, float3& wn) { float const ma = 0.f;//math::average(mu_a * w); float
const ms = math::average(mu_s * w); float const mn = math::average(mu_n * w); float const c = 1.f /
(ma + ms + mn);

        pa = ma * c;
        ps = ms * c;
        pn = mn * c;

        wa = (mu_a / (mt * pa));
        ws = (mu_s / (mt * ps));
        wn = (mu_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt,
                                                                                         float3
const& mu_s, float3 const& mu_n, float3 const& w, float& ps, float& pn, float3& ws, float3& wn) {
        float const ms = math::average(mu_s * w);
        float const mn = math::average(mu_n * w);
        float const c = 1.f / (ms + mn);

        ps = ms * c;
        pn = mn * c;

        ws = (mu_s / (mt * ps));
        wn = (mu_n / (mt * pn));
}
*/
bool Tracking_multi::transmittance(Ray const& ray, Worker& worker, float3& transmittance) noexcept {
    return Tracking::transmittance(ray, rng_, worker, transmittance);
}

bool Tracking_multi::integrate(Ray& ray, Intersection& intersection, Sampler_filter filter,
                               Worker& worker, float3& li, float3& transmittance) noexcept {
    if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
        li            = float3(0.f);
        transmittance = float3(1.f);
        return false;
    }

    float const d     = ray.max_t;
    float const range = d - ray.min_t;

    if (range < Tracking::Ray_epsilon) {
        li            = float3(0.f);
        transmittance = float3(1.f);
        return true;
    }

    SOFT_ASSERT(!worker.interface_stack().empty());

    auto const interface = worker.interface_stack().top();

    auto const& material = *interface->material();

    if (!material.is_scattering_volume()) {
        // Basically the "glass" case
        float3 const mu_a = material.absorption_coefficient(interface->uv, filter, worker);

        li            = float3(0.f);
        transmittance = attenuation(range, mu_a);
        return true;
    }

    if (material.is_heterogeneous_volume()) {
        Transformation temp;
        auto const&    transformation = interface->prop->transformation_at(ray.time, temp);

        float3 const local_origin = transformation.world_to_object_point(ray.origin);
        float3 const local_dir    = transformation.world_to_object_vector(ray.direction);

        auto const   shape  = interface->prop->shape();
        float3 const origin = shape->object_to_texture_point(local_origin);
        float3 const dir    = shape->object_to_texture_vector(local_dir);

        math::Ray local_ray(origin, dir, ray.min_t, ray.max_t);

        const float ray_offset = Tracking::Ray_epsilon / math::length(dir);

        auto const& tree = *material.volume_tree();

        float3 w(1.f);
        for (; local_ray.min_t < d;) {
            if (Tracking::CM data; tree.intersect(local_ray, data)) {
                if (float t;
                    Tracking::tracking(local_ray, data, material, filter, rng_, worker, t, w)) {
                    intersection.prop       = interface->prop;
                    intersection.geo.p      = ray.point(t);
                    intersection.geo.uv     = interface->uv;
                    intersection.geo.part   = interface->part;
                    intersection.subsurface = true;

                    li            = float3(0.f);
                    transmittance = w;
                    return math::all_greater_equal(w, Tracking::Abort_epsilon);
                }
            }

            SOFT_ASSERT(local_ray.max_t + ray_offset > local_ray.min_t);

            local_ray.min_t = local_ray.max_t + ray_offset;
            local_ray.max_t = d;
        }

        li            = float3(0.f);
        transmittance = w;
        return math::all_greater_equal(w, Tracking::Abort_epsilon);
    } else if (material.is_textured_volume()) {
        auto const mu = material.collision_coefficients(interface->uv, filter, worker);

        float3 const mu_t = mu.a + mu.s;

        float const mt  = math::max_component(mu_t);
        float const imt = 1.f / mt;

        float3 const mu_n = float3(mt) - mu_t;

        float3 w(1.f);

        for (float t = ray.min_t;;) {
            float const r0 = rng_.random_float();
            t -= std::log(1.f - r0) * imt;
            if (t > d) {
                li            = float3(0.f);
                transmittance = w;
                return true;
            }

            //		float ps, pn;
            //		float3 ws, wn;
            //		avg_history_probabilities(mt, mu_s, mu_n, w, ps, pn, ws, wn);

            float const ms = math::average(mu.s * w);
            float const mn = math::average(mu_n * w);

            float const mc = ms + mn;
            if (mc < 1e-10f) {
                li            = float3(0.f);
                transmittance = w;
                return true;
            }

            float const c = 1.f / mc;

            float const ps = ms * c;
            float const pn = mn * c;

            float const r1 = rng_.random_float();
            if (r1 <= 1.f - pn && ps > 0.f) {
                intersection.prop       = interface->prop;
                intersection.geo.p      = ray.point(t);
                intersection.geo.uv     = interface->uv;
                intersection.geo.part   = interface->part;
                intersection.subsurface = true;

                float3 const ws = mu.s / (mt * ps);

                li = float3(0.f);
                transmittance = w * ws;
                return true;
            } else {
                float3 const wn = mu_n / (mt * pn);

                SOFT_ASSERT(math::all_finite(wn));

                w *= wn;
            }
        }
    } else {
        /*
        static bool constexpr achromtatic = true;

        if (achromtatic) {
            auto const mu = material.collision_coefficients(interface->uv, filter, worker);

            const float3 sigma_a = mu.a;

                const float3 sigma_s = mu.s;

            const float3 extinction = sigma_a + sigma_s;

            float const minorant_mu_t = math::average(extinction);

                const float3 scattering_albedo = sigma_s / extinction;

                const float r = rng_.random_float();
                const float t = ray.min_t -std::log(1.f - r) / minorant_mu_t;

                if (t < d) {
                    intersection.prop           = interface->prop;
                    intersection.geo.p          = ray.point(t);
                    intersection.geo.uv         = interface->uv;
                    intersection.geo.part       = interface->part;
                    intersection.subsurface = true;

                    li = float3(0.f);
                    transmittance = scattering_albedo;
                    return true;
                }

                li = float3(0.f);
                transmittance = float3(1.f);
                return true;
        }
    */

        static bool constexpr decomposition = false;

        if (decomposition) {
            /*
            auto const cm = material.control_medium();

            float const rc  = rng_.random_float();
            float const t_c = ray.min_t - std::log(1.f - rc) / cm.minorant_mu_t;

            if (t_c < d) {
                intersection.prop           = interface->prop;
                intersection.geo.p          = ray.point(t_c);
                intersection.geo.uv         = interface->uv;
                intersection.geo.part       = interface->part;
                intersection.subsurface = true;

                li            = float3(0.f);
                transmittance = float3(cm.minorant_mu_s / cm.minorant_mu_t);
                return true;
            }

            float const mt = cm.majorant_mu_t - cm.minorant_mu_t;

            auto mu = material.collision_coefficients();
            mu.a -= cm.minorant_mu_a;
            mu.s -= cm.minorant_mu_s;

            float const rr  = rng_.random_float();
            float const t_r = ray.min_t - std::log(1.f - rr) / mt;

            if (t_r < d) {
                intersection.prop           = interface->prop;
                intersection.geo.p          = ray.point(t_r);
                intersection.geo.uv         = interface->uv;
                intersection.geo.part       = interface->part;
                intersection.subsurface = true;

                li            = float3(0.f);
                transmittance = float3(mu.s / mt);
                return true;
            }

            li            = float3(0.f);
            transmittance = float3(1.f);
            return true;
*/

            auto const cm = material.control_medium();

            float const rc  = rng_.random_float();
            float const t_c = ray.min_t - std::log(1.f - rc) / cm.minorant_mu_t;

            if (t_c > ray.max_t) {
                li            = float3(0.f);
                transmittance = float3(1.f);
                return true;
            }

            float const mt = cm.majorant_mu_t - cm.minorant_mu_t;

            auto mu = material.collision_coefficients();
            mu.a -= cm.minorant_mu_a;
            mu.s -= cm.minorant_mu_s;

            float const mu_n = cm.majorant_mu_t - cm.minorant_mu_t - mt;

            float const imt = 1.f / mt;

            for (float t = ray.min_t;;) {
                float const r0 = rng_.random_float();
                t -= std::log(1.f - r0) * imt;

                if (t > t_c) {
                    float const r1 = rng_.random_float();

                    if (r1 < cm.minorant_mu_a / cm.minorant_mu_t) {
                        li            = float3(0.f);
                        transmittance = float3(0.f);
                        return true;
                    } else {
                        intersection.prop       = interface->prop;
                        intersection.geo.p      = ray.point(t_c);
                        intersection.geo.uv     = interface->uv;
                        intersection.geo.part   = interface->part;
                        intersection.subsurface = true;

                        li            = float3(0.f);
                        transmittance = float3(1.f);
                        return true;
                    }
                } else {
                    float const r1 = rng_.random_float();

                    if (r1 < mu.a[0] / mt) {
                        li            = float3(0.f);
                        transmittance = float3(0.f);
                        return true;
                    } else if (r1 < 1.f - mu_n / mt) {
                        intersection.prop       = interface->prop;
                        intersection.geo.p      = ray.point(t);
                        intersection.geo.uv     = interface->uv;
                        intersection.geo.part   = interface->part;
                        intersection.subsurface = true;

                        li            = float3(0.f);
                        transmittance = float3(1.f);
                        return true;
                    }
                }
            }

        } else {
            auto const mu = material.collision_coefficients();

            float3 const mu_t = mu.a + mu.s;

            float const mt  = math::max_component(mu_t);
            float const imt = 1.f / mt;

            float3 const mu_n = float3(mt) - mu_t;

            float3 w(1.f);

            for (float t = ray.min_t;;) {
                float const r0 = rng_.random_float();
                t -= std::log(1.f - r0) * imt;
                if (t > d) {
                    li            = float3(0.f);
                    transmittance = w;
                    return true;
                }

                //		float ps, pn;
                //		float3 ws, wn;
                //		avg_history_probabilities(mt, mu_s, mu_n, w, ps, pn, ws, wn);

                float const ms = math::average(mu.s * w);
                float const mn = math::average(mu_n * w);
                float const c  = 1.f / (ms + mn);

                float const ps = ms * c;
                float const pn = mn * c;

                float const r1 = rng_.random_float();
                if (r1 <= 1.f - pn && ps > 0.f) {
                    intersection.prop       = interface->prop;
                    intersection.geo.p      = ray.point(t);
                    intersection.geo.uv     = interface->uv;
                    intersection.geo.part   = interface->part;
                    intersection.subsurface = true;

                    float3 const ws = mu.s / (mt * ps);

                    li = float3(0.f);
                    transmittance = w * ws;
                    return true;
                } else {
                    float3 const wn = mu_n / (mt * pn);

                    SOFT_ASSERT(math::all_finite(wn));

                    w *= wn;
                }
            }
        }
    }
}

size_t Tracking_multi::num_bytes() const noexcept {
    return sizeof(*this);
}

Tracking_multi_factory::Tracking_multi_factory(take::Settings const& take_settings,
                                               uint32_t              num_integrators) noexcept
    : Factory(take_settings, num_integrators),
      integrators_(memory::allocate_aligned<Tracking_multi>(num_integrators)) {}

Tracking_multi_factory::~Tracking_multi_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* Tracking_multi_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Tracking_multi(rng, take_settings_);
}

}  // namespace rendering::integrator::volume
