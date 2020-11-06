#include "tracking_single.hpp"
#include "base/math/aabb.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/memory/array.inl"
#include "base/random/generator.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/volume/volume_integrator.inl"
#include "rendering/rendering_worker.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "scene/entity/composed_transformation.inl"
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

Tracking_single::Tracking_single(bool progressive)
    : sampler_pool_(progressive ? nullptr
                                : new sampler::Golden_ratio_pool(2 * Num_dedicated_samplers)) {
    if (sampler_pool_) {
        for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
            material_samplers_[i] = sampler_pool_->get(2 * i + 0);
            light_samplers_[i]    = sampler_pool_->get(2 * i + 1);
        }
    } else {
        for (auto& s : material_samplers_) {
            s = &sampler_;
        }

        for (auto& s : light_samplers_) {
            s = &sampler_;
        }
    }

    lights_.reserve(scene::light::Tree::Max_lights);
}

Tracking_single::~Tracking_single() {
    delete sampler_pool_;
}

void Tracking_single::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);

    static uint32_t constexpr Max_lights = light::Tree::Max_lights;

    for (auto s : material_samplers_) {
        s->resize(num_samples_per_pixel, 1, 0, Max_lights);
    }

    for (auto s : light_samplers_) {
        s->resize(num_samples_per_pixel, 1, Max_lights, Max_lights + 1);
    }
}

void Tracking_single::start_pixel(RNG& rng) {
    sampler_.start_pixel(rng);

    for (auto s : material_samplers_) {
        s->start_pixel(rng);
    }

    for (auto s : light_samplers_) {
        s->start_pixel(rng);
    }
}
/*
static inline void max_probabilities(float mt,
                                                                         float3 const& mu_a,
                                                                         float3 const& mu_s,
                                                                         float3 const& mu_n,
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
const& mu_a, float3 const& mu_s, float3 const& mu_n, float3 const& w, float& pa, float& ps, float&
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
const& mu_a, float3 const& mu_s, float3 const& mu_n, float3 const& w, float& pn, float3& wn) { float
const ma = max_component(mu_a * w); float const ms = max_component(mu_s * w); float
const mn = max_component(mu_n * w); float const c = 1.f / (ma + ms + mn);

        pn = mn * c;

        wn = (mu_n / (mt * pn));
}

static inline void avg_probabilities(float mt,
                                                                         float3 const& mu_a,
                                                                         float3 const& mu_s,
                                                                         float3 const& mu_n,
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
const& mu_a, float3 const& mu_s, float3 const& mu_n, float3 const& w, float& pa, float& ps, float&
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
*/
static inline void avg_history_probabilities(float mt, float3 const& mu_s, float3 const& mu_n,
                                             float3 const& w, float& ps, float& pn, float3& ws,
                                             float3& wn) {
    float const ms = average(mu_s * w);
    float const mn = average(mu_n * w);
    float const c  = 1.f / (ms + mn);

    ps = ms * c;
    pn = mn * c;

    ws = (mu_s / (mt * ps));
    wn = (mu_n / (mt * pn));
}
/*
static inline void avg_history_probabilities(float mt,
                                                                                         float3
const& mu_s, float3 const& mu_n, float3 const& w, float& pn, float3& wn) { float const ms =
average(mu_s * w); float const mn = average(mu_n * w); float const c = 1.f / (ms + mn);

        pn = mn * c;

        wn = (mu_n / (mt * pn));
}
*/
bool Tracking_single::transmittance(Ray const& ray, Worker& worker, float3& tr) {
    return Tracking::transmittance(ray, worker, tr);
}

Event Tracking_single::integrate(Ray& ray, Intersection& isec, Filter filter, Worker& worker,
                                 float3& li, float3& tr) {
    if (!worker.intersect_and_resolve_mask(ray, isec, filter)) {
        li = float3(0.f);
        tr = float3(1.f);
        return Event::Abort;
    }

    float const d = ray.max_t();

    // Not sure wether the first test still makes sense.
    // The second test avoids falsely reporting very long volume sections,
    // when in fact a very close isec was missed.
    // However, this might cause problems if we ever want to support "infinite" volumes.

    if (scene::offset_f(ray.min_t()) >= d || scene::Almost_ray_max_t <= d) {
        li = float3(0.f);
        tr = float3(1.f);
        return Event::Pass;
    }

    SOFT_ASSERT(!worker.interface_stack().empty());

    auto const interface = worker.interface_stack().top();

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
                    auto const& light_ref = scene.light(light.id);

                    li = w * direct_light(light_ref, light.pdf, ray, p, 0, isec, worker);
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
        auto const& light_ref = worker.scene().light(light.id);

        float3 const l = direct_light(light_ref, light.pdf, ray, p, 0, isec, worker);

        li = l * (1.f - tr) * scattering_albedo;
    } else {
        auto const mu = material.collision_coefficients();

        float3 const attenuation = mu.a + mu.s;

        float3 const scattering_albedo = mu.s / attenuation;

        tr = exp(-(d - ray.min_t()) * attenuation);

        float const select = light_sampler(ray.depth).sample_1D(rng, light::Tree::Max_lights);

        bool const split = ray.depth < Num_dedicated_samplers;

        worker.scene().random_light(ray.point(ray.min_t()), ray.point(d), select, split, lights_);

        // li = one_bounce(ray, isec, material, worker);

        float3 lli(0.f);

        for (uint32_t il = 0, len = lights_.size(); il < len; ++il) {
            auto const  light     = lights_[il];
            auto const& light_ref = worker.scene().light(light.id);

            if (light_ref.is_finite(worker.scene())) {
                // Equi-angular sampling
                float3 const position = worker.scene().light_aabb(light.id).position();

                float const delta = dot(position - ray.origin, ray.direction);

                float3 const closest_point = ray.point(delta);

                float const D = distance(closest_point, position);

                float const theta_a = std::atan2(ray.min_t() - delta, D);
                float const theta_b = std::atan2(d - delta, D);

                float const r = material_sampler(ray.depth).sample_1D(rng, il);
                float const t = D * std::tan(lerp(theta_a, theta_b, r));

                float const sample_t = delta + t;

                float3 const p = ray.point(sample_t);

                float3 const l = direct_light(light_ref, light.pdf, ray, p, il, isec, worker);

                float const pdf = D / ((theta_b - theta_a) * (D * D + t * t));

                float3 const w = exp(-(sample_t - ray.min_t()) * attenuation);

                lli += (l * attenuation) * (scattering_albedo * w) / pdf;
            } else {
                // Distance sampling
                float const r = material_sampler(ray.depth).sample_1D(rng, 0);
                float const t = -std::log(1.f - r * (1.f - average(tr))) / average(attenuation);

                float3 const p = ray.point(ray.min_t() + t);

                float3 const l = direct_light(light_ref, light.pdf, ray, p, il, isec, worker);

                // Short version
                lli += l * (1.f - tr) * scattering_albedo;

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
                                     float3 const& position, uint32_t sampler_d,
                                     Intersection const& isec, Worker& worker) {
    shape::Sample_to light_sample;
    if (!light.sample(position, ray.time, light_sampler(ray.depth), sampler_d, worker,
                      light_sample)) {
        return float3(0.f);
    }

    Ray shadow_ray(position, light_sample.wi, 0.f, light_sample.t(), ray.depth, ray.wavelength,
                   ray.time);

    float3 tr;
    if (!worker.transmitted(shadow_ray, float3(0.f), isec, Filter::Nearest, tr)) {
        return float3(0.f);
    }

    SOFT_ASSERT(all_finite(tr));

    //    auto const bxdf = mat_sample.evaluate_f(light_sample.wi, evaluate_back);

    float const phase = 1.f / (4.f * Pi);

    float3 const radiance = light.evaluate(light_sample, Filter::Nearest, worker);

    return (phase * tr * radiance) / (light_sample.pdf() * light_pdf);
}

float3 Tracking_single::one_bounce(Ray const& ray, Intersection const& isec,
                                   Material const& material, Worker& worker) {
    auto const mu = material.collision_coefficients();

    float3 const attenuation = mu.a + mu.s;

    float3 const scattering_albedo = mu.s / attenuation;

    float const d = ray.max_t();

    float3 const tr = exp(-(d - ray.min_t()) * attenuation);

    auto& rng = worker.rng();

    float const r = material_sampler(ray.depth).sample_1D(rng, 0);
    float const t = -std::log(1.f - r * (1.f - average(tr))) / average(attenuation);

    float3 const p = ray.point(ray.min_t() + t);

    float2 const uv(rng.random_float(), rng.random_float());

    float3 const wi = sample_sphere_uniform(uv);

    Ray bounce_ray(p, wi, 0.f, scene::Ray_max_t, ray.depth, ray.wavelength, ray.time);

    Intersection tisec = isec;

    if (!worker.intersect_and_resolve_mask(bounce_ray, tisec, Filter::Undefined)) {
        return float3(0.f);
    }

    uint32_t const light_id = tisec.light_id(worker);
    if (!Light::is_area_light(light_id)) {
        return float3(0.f);
    }

    float light_pdf = 0.f;
    /*
        if (state.no(State::Treat_as_singular)) {
            bool const calculate_pdf = Light_sampling::Single ==
                                       settings_.light_sampling;

            bool const translucent = state.is(State::Is_translucent);

            auto const light = worker.scene().light(light_id, ray.origin, geo_n, translucent,
                                                    calculate_pdf);

            float const ls_pdf = light.ptr->pdf(ray, isec.geo, translucent, Filter::Nearest,
                                               worker);

            light_pdf = ls_pdf * light.pdf;
        }
    */
    float3 const wo = -wi;  // sample_result.wi;

    // This will invalidate the contents of previous material sample.
    auto const& mat_sample = tisec.sample(wo, bounce_ray, Filter::Undefined, 0.f, false, sampler_,
                                          worker);

    if (!mat_sample.same_hemisphere(wo)) {
        return float3(0.f);
    }

    float3 const ls_energy = mat_sample.radiance();

    bounce_ray.max_t() = std::max(scene::offset_b(bounce_ray.max_t()), 0.f);

    float3 trans;
    if (!worker.transmitted(bounce_ray, float3(0.f), tisec, Filter::Nearest, trans)) {
        return float3(0.f);
    }

    float3 const ltr = exp(-t * attenuation);

    float3 const weight = (1.f - tr) / (ltr * attenuation);

    return (trans * ls_energy) * attenuation * scattering_albedo * ltr * weight;

    //   return  ((1.f - tr) * scattering_albedo) * (trans * ls_energy);
}

sampler::Sampler& Tracking_single::material_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return *material_samplers_[bounce];
    }

    return sampler_;
}

sampler::Sampler& Tracking_single::light_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return *light_samplers_[bounce];
    }

    return sampler_;
}

Tracking_single_pool::Tracking_single_pool(uint32_t num_integrators, bool progressive)
    : Typed_pool<Tracking_single>(num_integrators), progressive_(progressive) {}

Integrator* Tracking_single_pool::get(uint32_t id) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Tracking_single(progressive_);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::volume
