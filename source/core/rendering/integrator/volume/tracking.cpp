#include "tracking.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/volumetric/volumetric_octree.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape.hpp"

#include "base/debug/assert.hpp"

#ifdef SU_DEBUG
#include "base/math/print.hpp"
#endif

namespace rendering::integrator::volume {

#ifdef SU_DEBUG
bool check(float3 const& majorant_mt, float mt);
#endif

// Code for hetereogeneous transmittance inspired by:
// https://github.com/DaWelter/ToyTrace/blob/master/src/atmosphere.cxx

static inline bool residual_ratio_tracking_transmitted(float3& transmitted, ray const& ray,
                                                       float minorant_mu_t, float majorant_mu_t,
                                                       Tracking::Material const& material,
                                                       float srs, Tracking::Filter filter,
                                                       rnd::Generator& rng,
                                                       Worker&         worker) noexcept {
    // Transmittance of the control medium
    transmitted *= attenuation(ray.max_t - ray.min_t, minorant_mu_t);

    if (all_less(transmitted, Tracking::Abort_epsilon)) {
        return false;
    }

    float const mt = majorant_mu_t - minorant_mu_t;

    if (mt < Tracking::Min_mt) {
        return true;
    }

    // Transmittance of the residual medium
    float const imt = 1.f / mt;

    SOFT_ASSERT(std::isfinite(imt));

    for (float t = ray.min_t, d = ray.max_t;;) {
        float const r0 = rng.random_float();
        t -= std::log(1.f - r0) * imt;
        if (t > d) {
            return true;
        }

        float3 const uvw = ray.point(t);

        auto mu = material.collision_coefficients(uvw, filter, worker);

        mu.s *= srs;

        float3 const mu_t = (mu.a + mu.s) - minorant_mu_t;

        SOFT_ASSERT(check(mu_t, mt));

        float3 const mu_n = float3(mt) - mu_t;

        transmitted *= imt * mu_n;

        if (all_less(transmitted, Tracking::Abort_epsilon)) {
            return false;
        }

        SOFT_ASSERT(all_finite(transmitted));
    }
}

static inline bool tracking_transmitted(float3& transmitted, ray const& ray, Tracking::CM const& cm,
                                        Tracking::Material const& material, float srs,
                                        Tracking::Filter filter, rnd::Generator& rng,
                                        Worker& worker) noexcept {
    float const mt = cm.majorant_mu_t();

    if (mt < Tracking::Min_mt) {
        return true;
    }

    if (float minorant_mu_t = cm.minorant_mu_t(); minorant_mu_t > 0.f) {
        return residual_ratio_tracking_transmitted(transmitted, ray, minorant_mu_t, mt, material,
                                                   srs, filter, rng, worker);
    }

    float const imt = 1.f / mt;

    SOFT_ASSERT(std::isfinite(imt));

    for (float t = ray.min_t, d = ray.max_t;;) {
        float const r0 = rng.random_float();
        t -= std::log(1.f - r0) * imt;
        if (t > d) {
            return true;
        }

        float3 const uvw = ray.point(t);

        auto mu = material.collision_coefficients(uvw, filter, worker);

        mu.s *= srs;

        float3 const mu_t = mu.a + mu.s;

        SOFT_ASSERT(check(mu_t, mt));

        float3 const mu_n = float3(mt) - mu_t;

        transmitted *= imt * mu_n;

        // TODO: Consider employing russian roulette instead of just aborting
        if (all_less(transmitted, Tracking::Abort_epsilon)) {
            return false;
        }

        //        if (all_less(transmitted, 0.01f)) {
        //            static float constexpr q = 0.1f;
        //            if (rendering::russian_roulette(transmitted, q, rng.random_float())) {
        //                return false;
        //            }
        //        }

        SOFT_ASSERT(all_finite(transmitted));
    }
}

bool Tracking::transmittance(Ray const& ray, rnd::Generator& rng, Worker& worker,
                             float3& tr) noexcept {
    SOFT_ASSERT(!worker.interface_stack().empty());

    auto const interface = worker.interface_stack().top();

    auto const& material = *interface->material(worker);

    float const d = ray.max_t;

    if (scene::offset_f(ray.min_t) >= d) {
        tr = float3(1.f);
        return true;
    }

    if (material.is_heterogeneous_volume()) {
        math::ray local_ray = texture_space_ray(ray, interface->prop, worker);

        auto const& tree = *material.volume_tree();

        float const srs = material.similarity_relation_scale(ray.depth);

        float3 w(1.f);
        for (; local_ray.min_t < d;) {
            if (CM cm; tree.intersect(local_ray, cm)) {
                cm.minorant_mu_s *= srs;
                cm.majorant_mu_s *= srs;

                if (!tracking_transmitted(w, local_ray, cm, material, srs, Filter::Nearest, rng,
                                          worker)) {
                    return false;
                }
            }

            SOFT_ASSERT(scene::offset_f(local_ray.max_t) > local_ray.min_t);

            local_ray.min_t = scene::offset_f(local_ray.max_t);
            local_ray.max_t = d;
        }

        tr = w;
        return true;
    }

    if (material.is_textured_volume()) {
        auto const mu = material.collision_coefficients(interface->uv, Filter::Nearest, worker);

        float3 const mu_t = mu.a + mu.s;

        tr = attenuation(d - ray.min_t, mu_t);
        return true;
    }

    auto const mu = material.collision_coefficients();

    float3 const mu_t = mu.a + mu.s;

    tr = attenuation(d - ray.min_t, mu_t);
    return true;
}

static inline bool decomposition_tracking(ray const& ray, Tracking::CM const& cm,
                                          Tracking::Material const& material, float srs,
                                          Tracking::Filter filter, rnd::Generator& rng,
                                          Worker& worker, float& t_out, float3& w) noexcept {
    float const d = ray.max_t;

    float const mt  = cm.majorant_mu_t();
    float const imt = 1.f / mt;

    //      float3 const mu_n = float3(mt) - mu_t;

    float lw = w[0];

    float const pc_a = cm.minorant_mu_a * imt;
    float const pc_s = cm.minorant_mu_s * imt;

    float const cm_t = cm.minorant_mu_t();

    float const factor = 1.f - cm_t * imt;

    for (float t = ray.min_t;;) {
        float const r0 = rng.random_float();
        t -= std::log(1.f - r0) * imt;
        if (t > d) {
            w     = float3(lw);
            t_out = t;
            return false;
        }

        float f = 0.f;

        float const r1 = rng.random_float();

        if (r1 < (f += pc_a)) {
            w     = float3(lw);
            t_out = t;
            return false;
        }

        if (r1 < (f += pc_s)) {
            lw *= cm.minorant_mu_s / (mt * pc_s);
            w = float3(lw);

            t_out = t;
            return true;
        }

        float3 const uvw = ray.point(t);

        auto mu = material.collision_coefficients(uvw, filter, worker);

        scene::material::CC const rm{float3(mu.a - cm.minorant_mu_a),
                                     float3((srs * mu.s) - cm.minorant_mu_s)};

        float const rm_t = rm.a[0] + rm.s[0];
        float const mu_n = std::max(mt - cm_t - rm_t, 0.f);

        float const divisor = factor / (rm.a[0] + rm.s[0] + mu_n);

        float const pr_a = divisor * rm.a[0];
        float const pr_s = divisor * rm.s[0];
        float const p_n  = divisor * mu_n;

        if (r1 < (f += pr_a)) {
            w     = float3(lw);
            t_out = t;
            return false;
        } else if (r1 < (f += pr_s)) {
            lw *= rm.s[0] / (mt * pr_s);
            w = float3(lw);

            t_out = t;

            return true;
        } else {
            lw *= mu_n / (mt * p_n);
        }
    }
}

bool Tracking::tracking(ray const& ray, CM const& cm, Material const& material, float srs,
                        Filter filter, rnd::Generator& rng, Worker& worker, float& t_out,
                        float3& w) noexcept {
    float const mt = cm.majorant_mu_t();

    if (mt < Min_mt) {
        return false;
    }

    static bool constexpr decomposition = true;

    if (decomposition /*&& cm.minorant_mu_t() > 0.f*/) {
        return decomposition_tracking(ray, cm, material, srs, filter, rng, worker, t_out, w);
    }

    float3 lw = w;

    SOFT_ASSERT(all_finite(lw));

    float const imt = 1.f / mt;

    for (float t = ray.min_t, d = ray.max_t;;) {
        float const r0 = rng.random_float();
        t -= std::log(1.f - r0) * imt;
        if (t > d) {
            w = lw;
            return false;
        }

        float3 const uvw = ray.point(t);

        auto mu = material.collision_coefficients(uvw, filter, worker);

        mu.s *= srs;

        float3 const mu_t = mu.a + mu.s;

        float3 const mu_n = float3(mt) - mu_t;

        float const ms = average(mu.s * lw);
        float const mn = average(mu_n * lw);
        float const c  = 1.f / (ms + mn);

        float const ps = ms * c;
        float const pn = mn * c;

        if (float const r1 = rng.random_float(); (r1 <= 1.f - pn) & (ps > 0.f)) {
            float3 const ws = mu.s / (mt * ps);

            SOFT_ASSERT(all_finite(ws));

            t_out = t;
            w     = lw * ws;
            return true;
        } else {
            float3 const wn = mu_n / (mt * pn);

            SOFT_ASSERT(all_finite(wn));

            lw *= wn;
        }
    }
}

Event Tracking::tracking(ray const& ray, CM const& cm, Material const& material, float srs,
                         Filter filter, rnd::Generator& rng, Worker& worker, float& t_out,
                         float3& w, float3& li) noexcept {
    float const mt = cm.majorant_mu_t();

    if (mt < Min_mt) {
        return Event::Pass;
    }

    float3 lw = w;

    SOFT_ASSERT(all_finite(lw));

    float const imt = 1.f / mt;

    for (float t = ray.min_t, d = ray.max_t;;) {
        float const r0 = rng.random_float();
        t -= std::log(1.f - r0) * imt;
        if (t > d) {
            w  = lw;
            li = float3(0.);
            return Event::Pass;
        }

        float3 const uvw = ray.point(t);

        auto cce = material.collision_coefficients_emission(uvw, filter, worker);

        CC& mu = cce.cc;
        mu.s *= srs;

        float3 const mu_t = mu.a + mu.s;

        float3 const mu_n = float3(mt) - mu_t;

        float const ma = average(mu.a * lw);
        float const ms = average(mu.s * lw);
        float const mn = average(mu_n * lw);
        float const c  = 1.f / (ma + ms + mn);

        float const pa = ma * c;
        float const ps = ms * c;
        float const pn = mn * c;

        float const r1 = rng.random_float();
        if (r1 < pa) {
            float3 const wa = mu.a / (mt * pa);

            t_out = t;
            w     = float3(0.f);
            li    = lw * wa * cce.e;
            return Event::Absorb;
        }

        if ((r1 <= 1.f - pn) & (ps > 0.f)) {
            float3 const ws = mu.s / (mt * ps);

            SOFT_ASSERT(all_finite(ws));

            t_out = t;
            w     = lw * ws;
            li    = float3(0.);
            return Event::Scatter;
        }

        float3 const wn = mu_n / (mt * pn);

        SOFT_ASSERT(all_finite(wn));

        lw *= wn;
    }
}

bool Tracking::tracking(ray const& ray, CC const& mu, rnd::Generator& rng, float& t_out,
                        float3& w) noexcept {
    float3 const mu_t = mu.a + mu.s;

    float const mt  = max_component(mu_t);
    float const imt = 1.f / mt;

    float3 const mu_n = float3(mt) - mu_t;

    float3 lw(1.f);

    for (float t = ray.min_t, d = ray.max_t;;) {
        float const r0 = rng.random_float();
        t -= std::log(1.f - r0) * imt;
        if (t > d) {
            w = lw;
            return false;
        }

        float const ms = average(mu.s * lw);
        float const mn = average(mu_n * lw);

        float const mc = ms + mn;
        if (mc < 1e-10f) {
            w = lw;
            return false;
        }

        float const c = 1.f / mc;

        float const ps = ms * c;
        float const pn = mn * c;

        if (float const r1 = rng.random_float(); (r1 <= 1.f - pn) & (ps > 0.f)) {
            float3 const ws = mu.s / (mt * ps);

            t_out = t;
            w     = lw * ws;
            return true;
        } else {
            float3 const wn = mu_n / (mt * pn);

            SOFT_ASSERT(all_finite(wn));

            lw *= wn;
        }
    }
}

Event Tracking::tracking(ray const& ray, CCE const& cce, rnd::Generator& rng, float& t_out,
                         float3& w, float3& li) noexcept {
    CC const& mu = cce.cc;

    float3 const mu_t = mu.a + mu.s;

    float const mt  = max_component(mu_t);
    float const imt = 1.f / mt;

    float3 const mu_n = float3(mt) - mu_t;

    float3 lw(1.f);

    for (float t = ray.min_t, d = ray.max_t;;) {
        float const r0 = rng.random_float();
        t -= std::log(1.f - r0) * imt;
        if (t > d) {
            w  = lw;
            li = float3(0.f);
            return Event::Pass;
        }

        float const ma = average(mu.a * lw);
        float const ms = average(mu.s * lw);
        float const mn = average(mu_n * lw);

        float const mc = ma + ms + mn;
        if (mc < 1e-10f) {
            w  = lw;
            li = float3(0.f);
            return Event::Pass;
        }

        float const c = 1.f / mc;

        float const pa = ma * c;
        float const ps = ms * c;
        float const pn = mn * c;

        float const r1 = rng.random_float();
        if (r1 < pa) {
            float3 const wa = mu.a / (mt * pa);

            t_out = t;
            w     = float3(0.f);
            li    = lw * wa * cce.e;
            return Event::Absorb;
        } else if ((r1 <= 1.f - pn) & (ps > 0.f)) {
            float3 const ws = mu.s / (mt * ps);

            t_out = t;
            w     = lw * ws;
            li    = float3(0.f);
            return Event::Scatter;
        } else {
            float3 const wn = mu_n / (mt * pn);

            SOFT_ASSERT(all_finite(wn));

            lw *= wn;
        }
    }
}

#ifdef SU_DEBUG
bool check(float3 const& majorant_mt, float mt) {
    if (mt < max_component(majorant_mt)) {
        std::cout << "mu_t: " << majorant_mt << " mt: " << mt << std::endl;
        return false;
    }

    return true;
}
#endif

}  // namespace rendering::integrator::volume
