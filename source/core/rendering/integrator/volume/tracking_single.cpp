#include "tracking_single.hpp"
#include "base/math/aabb.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/random/generator.inl"
#include "rendering/integrator/integrator.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "scene/composed_transformation.inl"
#include "scene/light/light.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/volumetric/volumetric_octree.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape.inl"
#include "scene/shape/shape_sample.hpp"
#include "tracking.inl"

#include "base/debug/assert.hpp"

namespace rendering::integrator::volume {

using namespace scene;

Tracking_single::Tracking_single(uint32_t max_samples_per_pixel, bool progressive) {
    if (progressive) {
        sampler_pool_ = new sampler::Random_pool(2 * Num_dedicated_samplers);
    } else {
        sampler_pool_ = new sampler::Golden_ratio_pool(2 * Num_dedicated_samplers);
    }

    static uint32_t constexpr Max_lights = light::Tree::Max_lights;

    for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
        sampler_pool_->create(2 * i + 0, 1, Max_lights + 1, max_samples_per_pixel);
        sampler_pool_->create(2 * i + 1, Max_lights, Max_lights + 1, max_samples_per_pixel);
    }
}

Tracking_single::~Tracking_single() {
    delete sampler_pool_;
}

void Tracking_single::start_pixel(RNG& rng) {
    sampler_.start_pixel(rng);

    for (uint32_t i = 0; i < 2 * Num_dedicated_samplers; ++i) {
        sampler_pool_->get(i).start_pixel(rng);
    }
}
/*
static inline void max_probabilities(float mt,
                                                                         float3_p mu_a,
                                                                         float3_p mu_s,
                                                                         float3_p mu_n,
                                                                         float& pa, float& ps,
float& pn, float3& wa, float3& ws, float3& wn) { float const ma = max_component(mu_a); float
const ms = max_component(mu_s); float const mn = max_component(mu_n); float const c
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
const& mu_a, float3_p mu_s, float3_p mu_n, float3_p w, float& pa, float& ps, float&
pn, float3& wa, float3& ws, float3& wn) { float const ma = max_component(mu_a * w); float
const ms = max_component(mu_s * w); float const mn = max_component(mu_n * w); float
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
const& mu_a, float3_p mu_s, float3_p mu_n, float3_p w, float& pn, float3& wn) { float
const ma = max_component(mu_a * w); float const ms = max_component(mu_s * w); float
const mn = max_component(mu_n * w); float const c = 1.f / (ma + ms + mn);

        pn = mn * c;

        wn = (mu_n / (mt * pn));
}

static inline void avg_probabilities(float mt,
                                                                         float3_p mu_a,
                                                                         float3_p mu_s,
                                                                         float3_p mu_n,
                                                                         float& pa, float& ps,
float& pn, float3& wa, float3& ws, float3& wn) { float const ma = average(mu_a); float const
ms = average(mu_s); float const mn = average(mu_n); float const c = 1.f / (ma + ms +
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
const& mu_a, float3_p mu_s, float3_p mu_n, float3_p w, float& pa, float& ps, float&
pn, float3& wa, float3& ws, float3& wn) { float const ma = 0.f;//average(mu_a * w); float
const ms = average(mu_s * w); float const mn = average(mu_n * w); float const c = 1.f /
(ma + ms + mn);

        pa = ma * c;
        ps = ms * c;
        pn = mn * c;

        wa = (mu_a / (mt * pa));
        ws = (mu_s / (mt * ps));
        wn = (mu_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt, float3_p mu_s, float3_p mu_n, float3_p w,
                                             float& ps, float& pn, float3& ws, float3& wn) {
    float const ms = average(mu_s * w);
    float const mn = average(mu_n * w);
    float const c  = 1.f / (ms + mn);

    ps = ms * c;
    pn = mn * c;

    ws = (mu_s / (mt * ps));
    wn = (mu_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt,
                                                                                         float3
const& mu_s, float3_p mu_n, float3_p w, float& pn, float3& wn) { float const ms =
average(mu_s * w); float const mn = average(mu_n * w); float const c = 1.f / (ms + mn);

        pn = mn * c;

        wn = (mu_n / (mt * pn));
}
*/
bool Tracking_single::transmittance(Ray const& ray, Filter filter, Worker& worker, float3& tr) {
    return Tracking::transmittance(ray, filter, worker, tr);
}

Event Tracking_single::integrate(Ray& ray, Intersection& isec, Filter filter, Worker& worker,
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
        if (shape::Intersection nisec;
            worker.intersect(interface->prop, tray, shape::Interpolation::Normal, nisec)) {
            if (dot(nisec.geo_n, v) <= 0.f) {
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

        auto const& scene = worker.scene();

        float3 w(1.f);
        for (; local_ray.min_t() < d;) {
            if (Tracking::CM data; tree.intersect(local_ray, data)) {
                if (float t;
                    Tracking::tracking(local_ray, data, material, 1.f, filter, worker, t, w)) {
                    float3 const p = ray.point(t);

                    float const select = light_sampler(ray.depth).sample_1D(rng, 1);

                    auto const  light     = scene.random_light(select);
                    auto const& light_ref = scene.light(light.offset);

                    li = w * direct_light(light_ref, light.pdf, ray, p, 0, isec, material, worker);
                    tr = float3(0.f);
                    return Event::Pass;
                }
            }

            local_ray.min_t() = scene::offset_f(local_ray.max_t());
            local_ray.max_t() = d;
        }

        li = float3(0.f);
        tr = w;
        return Event::Pass;
    }

    if (material.is_textured_volume()) {
        auto const mu = material.collision_coefficients(float2(0.f), filter, worker);

        float3 const attenuation = mu.a + mu.s;

        float3 const scattering_albedo = mu.s / attenuation;

        tr = exp(-(d - ray.min_t()) * attenuation);

        float const r = rng.random_float();
        float const t = -std::log(1.f - r * (1.f - average(tr))) / average(attenuation);

        float3 const p = ray.point(ray.min_t() + t);

        float const select = light_sampler(ray.depth).sample_1D(rng, 1);

        auto const  light     = worker.scene().random_light(select);
        auto const& light_ref = worker.scene().light(light.offset);

        float3 const l = direct_light(light_ref, light.pdf, ray, p, 0, isec, material, worker);

        li = l * (1.f - tr) * scattering_albedo;
    } else {
        auto const mu = material.collision_coefficients();

        float3 const attenuation = mu.a + mu.s;

        float3 const scattering_albedo = mu.s / attenuation;

        tr = exp(-(d - ray.min_t()) * attenuation);

        float3 const scattering_tr = (1.f - tr) * scattering_albedo;

        float const select = light_sampler(ray.depth).sample_1D(rng, light::Tree::Max_lights);

        bool const split = ray.depth < Num_dedicated_samplers;

        auto& lights = worker.lights();

        worker.scene().random_light(ray.point(ray.min_t()), ray.point(d), select, split, lights);

        float3 lli(one_bounce(tr, scattering_tr, ray, material, split, worker));

        for (uint32_t il = 0, len = lights.size(); il < len; ++il) {
            auto const  light     = lights[il];
            auto const& light_ref = worker.scene().light(light.offset);

            if (light_ref.is_finite(worker.scene())) {
                // Equi-angular sampling
                float3 const position = worker.scene().light_aabb(light.offset).position();

                float const delta = dot(position - ray.origin, ray.direction);

                float3 const closest_point = ray.point(delta);

                float const D = distance(closest_point, position);

                float const theta_a = std::atan2(ray.min_t() - delta, D);
                float const theta_b = std::atan2(d - delta, D);

                float const r = material_sampler(ray.depth).sample_1D(rng, il);
                float const t = D * std::tan(lerp(theta_a, theta_b, r));

                float const ea_pdf = D / ((theta_b - theta_a) * (D * D + t * t));

                // PDF for distance sampling
                float const sample_t = delta + t;

                float const avg_att       = average(attenuation);
                float const dinstance_pdf = avg_att /
                                            (math::exp((sample_t - ray.min_t()) * avg_att) -
                                             math::exp((sample_t - d) * avg_att));

                float3 const p = ray.point(sample_t);

                float3 const l = direct_light(light_ref, light.pdf, ea_pdf, dinstance_pdf, ray, p,
                                              il, isec, material, worker);

                float3 const w = exp(-(sample_t - ray.min_t()) * attenuation);

                lli += (l * attenuation) * (scattering_albedo * w);
            } else {
                // Distance sampling
                float const r = material_sampler(ray.depth).sample_1D(rng, il);
                float const t = -std::log(1.f - r * (1.f - average(tr))) / average(attenuation);

                float3 const p = ray.point(ray.min_t() + t);

                float3 const l = direct_light(light_ref, light.pdf, ray, p, il, isec, material,
                                              worker);

                // Short version
                lli += l * scattering_tr;

                // Instructive version
                //            {
                //                float3 const ltr = exp(-t * attenuation);

                //                float3 const weight = (1.f - tr) / (ltr * attenuation);

                //                li = l * attenuation * scattering_albedo * ltr * weight;
                //            }
            }
        }

        li = lli;
    }

    return Event::Pass;
}

float3 Tracking_single::direct_light(Light const& light, float light_pdf, Ray const& ray,
                                     float3_p position, uint32_t sampler_d,
                                     Intersection const& isec, Material const& material,
                                     Worker& worker) {
    shape::Sample_to light_sample;
    if (!light.sample(position, float3(0.f), true, ray.time, light_sampler(ray.depth), sampler_d,
                      worker, light_sample)) {
        return float3(0.f);
    }

    Ray shadow_ray(position, light_sample.wi, 0.f, light_sample.t(), ray.depth, ray.wavelength,
                   ray.time);

    float3 tr;
    if (!worker.transmitted(shadow_ray, float3(0.f), isec, Filter::Nearest, tr)) {
        return float3(0.f);
    }

    SOFT_ASSERT(all_finite(tr));

    float const phase = material.phase(-ray.direction, light_sample.wi);

    float3 const radiance = light.evaluate(light_sample, Filter::Nearest, worker);

    return (phase / (light_sample.pdf() * light_pdf)) * (tr * radiance);
}

float3 Tracking_single::direct_light(Light const& light, float light_pdf, float ea_pdf,
                                     float distance_pdf, Ray const& ray, float3_p position,
                                     uint32_t sampler_d, Intersection const& isec,
                                     Material const& material, Worker& worker) {
    shape::Sample_to light_sample;
    if (!light.sample(position, float3(0.f), ray.time, true, light_sampler(ray.depth), sampler_d,
                      worker, light_sample)) {
        return float3(0.f);
    }

    Ray shadow_ray(position, light_sample.wi, 0.f, light_sample.t(), ray.depth, ray.wavelength,
                   ray.time);

    float3 tr;
    if (!worker.transmitted(shadow_ray, float3(0.f), isec, Filter::Nearest, tr)) {
        return float3(0.f);
    }

    SOFT_ASSERT(all_finite(tr));

    float const phase = material.phase(-ray.direction, light_sample.wi);

    float3 const radiance = light.evaluate(light_sample, Filter::Nearest, worker);

    float const weight = predivided_power_heuristic(light_sample.pdf() * light_pdf * ea_pdf,
                                                    phase * distance_pdf);

    return (weight * phase) * (tr * radiance);
}

float3 Tracking_single::one_bounce(float3_p tr, float3_p scattering_tr, Ray const& ray,
                                   Material const& material, bool split, Worker& worker) {
    auto const mu = material.collision_coefficients();

    float3 const attenuation = mu.a + mu.s;

    float const d = ray.max_t();

    auto& rng = worker.rng();

    auto& sampler = material_sampler(ray.depth);

    float const avg_att = average(attenuation);

    float const r = sampler.sample_1D(rng, light::Tree::Max_lights);
    float const t = -std::log(1.f - r * (1.f - average(tr))) / avg_att;

    float const sample_t = ray.min_t() + t;

    float3 const p = ray.point(sample_t);

    float2 const uv(sampler.sample_2D(rng));

    float4 const sp = material.sample_phase(-ray.direction, uv);

    float3 const wi    = sp.xyz();
    float const  phase = sp[3];

    Ray bounce_ray(p, wi, 0.f, scene::Ray_max_t, ray.depth, ray.wavelength, ray.time);

    Intersection isec;

    if (!worker.intersect_and_resolve_mask(bounce_ray, isec, Filter::Undefined)) {
        return float3(0.f);
    }

    uint32_t const light_id = isec.light_id(worker);
    if (!Light::is_area_light(light_id)) {
        return float3(0.f);
    }

    float3 const wo = -wi;

    float3 ls_energy;
    bool   pure_emissive;
    if (!isec.evaluate_radiance(wo, Filter::Undefined, worker, ls_energy, pure_emissive)) {
        return float3(0.f);
    }

    auto const& scene = worker.scene();
    auto const  light = scene.light(light_id, ray.point(ray.min_t()), ray.point(d), split);

    if (!light.ref.is_finite(scene)) {
        return float3(0.f);
    }

    float const ls_pdf    = light.ref.pdf(bounce_ray, float3(0.f), isec, true, worker);
    float const light_pdf = ls_pdf * light.pdf;

    // PDF for equi-angular sampling
    float3 const position = scene.light_aabb(light.offset).position();

    float const delta = dot(position - ray.origin, ray.direction);

    float3 const closest_point = ray.point(delta);

    float const D = distance(closest_point, position);

    float const theta_a = std::atan2(ray.min_t() - delta, D);
    float const theta_b = std::atan2(d - delta, D);

    float const ea_t   = sample_t - delta;
    float const ea_pdf = D / ((theta_b - theta_a) * (D * D + ea_t * ea_t));

    // PDF for distance sampling
    float const range        = d - ray.min_t();
    float const distance_pdf = avg_att /
                               (math::exp(t * avg_att) - math::exp((t - range) * avg_att));

    float const mis_weight = power_heuristic(distance_pdf * phase, ea_pdf * light_pdf);

    bounce_ray.max_t() = std::max(scene::offset_b(bounce_ray.max_t()), 0.f);

    float3 trans;
    if (!worker.transmitted(bounce_ray, float3(0.f), isec, Filter::Nearest, trans)) {
        return float3(0.f);
    }

    return mis_weight * (trans * ls_energy * scattering_tr);
}

sampler::Sampler& Tracking_single::material_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return sampler_pool_->get(2 * bounce + 0);
    }

    return sampler_;
}

sampler::Sampler& Tracking_single::light_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return sampler_pool_->get(2 * bounce + 1);
    }

    return sampler_;
}

Tracking_single_pool::Tracking_single_pool(uint32_t num_integrators, bool progressive)
    : Typed_pool<Tracking_single, Integrator>(num_integrators), progressive_(progressive) {}

Integrator* Tracking_single_pool::create(uint32_t id, uint32_t max_samples_per_pixel) const {
    return new (&integrators_[id]) Tracking_single(max_samples_per_pixel, progressive_);
}

}  // namespace rendering::integrator::volume
