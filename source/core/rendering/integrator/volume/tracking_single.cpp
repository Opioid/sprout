#include "tracking_single.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/light/light.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/volumetric/volumetric_octree.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"
#include "tracking.inl"

#include "base/debug/assert.hpp"

namespace rendering::integrator::volume {

using namespace scene;

Tracking_single::Tracking_single(rnd::Generator& rng) noexcept
    : Integrator(rng), sampler_(rng), material_samplers_{rng}, light_samplers_{rng} {}

void Tracking_single::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);

    for (auto& s : material_samplers_) {
        s.resize(num_samples_per_pixel, 1, 0, 2);
    }

    for (auto& s : light_samplers_) {
        s.resize(num_samples_per_pixel, 1, 1, 2);
    }
}

void Tracking_single::start_pixel() noexcept {
    sampler_.start_pixel();

    for (auto& s : material_samplers_) {
        s.start_pixel();
    }

    for (auto& s : light_samplers_) {
        s.start_pixel();
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
                                             float3& wn) noexcept {
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
bool Tracking_single::transmittance(Ray const& ray, Worker& worker, float3& tr) noexcept {
    return Tracking::transmittance(ray, rng_, worker, tr);
}

Event Tracking_single::integrate(Ray& ray, Intersection& intersection, Filter filter,
                                 Worker& worker, float3& li, float3& tr) noexcept {
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

        float3 w(1.f);
        for (; local_ray.min_t < d;) {
            if (Tracking::CM data; tree.intersect(local_ray, data)) {
                if (float t; Tracking::tracking(local_ray, data, material, 1.f, filter, rng_,
                                                worker, t, w)) {
                    li = w * direct_light(ray, ray.point(t), intersection, worker);
                    tr = float3(0.f);
                    return Event::Pass;
                }
            }

            local_ray.min_t = scene::offset_f(local_ray.max_t);
            local_ray.max_t = d;
        }

        li = float3(0.f);
        tr = w;
        return Event::Pass;
    } else if (material.is_textured_volume()) {
        auto const mu = material.collision_coefficients(float2(0.f), filter, worker);

        float3 const extinction = mu.a + mu.s;

        float3 const scattering_albedo = mu.s / extinction;

        tr = exp(-(d - ray.min_t) * extinction);

        float const r = rng_.random_float();
        float const t = -std::log(1.f - r * (1.f - average(tr))) / average(extinction);

        float3 const p = ray.point(ray.min_t + t);

        float3 const l = direct_light(ray, p, intersection, worker);

        li = l * (1.f - tr) * scattering_albedo;
    } else {
        auto const mu = material.collision_coefficients();

        float3 const extinction = mu.a + mu.s;

        float3 const scattering_albedo = mu.s / extinction;

        tr = exp(-(d - ray.min_t) * extinction);

        //     auto const light = worker.scene().random_light(rng_.random_float());

        if (/*light.ref.is_finite(worker.scene())*/ false) {
            /*
            // Equi-angular sampling
            float3 const position = worker.scene().light_center(light.id);

            float const delta = dot(position - ray.origin, ray.direction);

            float3 const closest_point = ray.point(delta);

            float const D = distance(closest_point, position);

            float const theta_a = std::atan2(ray.min_t - delta, D);
            float const theta_b = std::atan2(d - delta, D);

            float const r = rng_.random_float();
            float const t = D * std::tan(lerp(theta_a, theta_b, r));

            float const sample_t = delta + t;

            float3 const p = ray.point(sample_t);

            float3 const l = direct_light(light.ref, light.pdf, ray, p, intersection, worker);

            float const pdf = D / ((theta_b - theta_a) * (D * D + t * t));

            float3 const w = exp(-(sample_t - ray.min_t) * extinction);

            li = (l * extinction) * (scattering_albedo * w) / pdf;
            */
        } else {
            // Distance sampling
            float const r = material_sampler(ray.depth).generate_sample_1D(0);
            float const t = -std::log(1.f - r * (1.f - average(tr))) / average(extinction);

            float3 const p = ray.point(ray.min_t + t);

            float const select = light_sampler(ray.depth).generate_sample_1D(1);

            auto const light = worker.scene().random_light(p, float3(0.f), true, select);

            float3 const l = direct_light(light.ref, light.pdf, ray, p, intersection, worker);

            // Short version
            li = l * (1.f - tr) * scattering_albedo;

            // Instructive version
            /*
            float3 const ltr = exp(-t * extinction);

            float3 const weight = (1.f - tr) / (ltr * extinction);

            li = l * extinction * scattering_albedo * ltr * weight;
            */
        }
    }

    return Event::Pass;
}

float3 Tracking_single::direct_light(Ray const& ray, float3 const& position,
                                     Intersection const& intersection, Worker& worker) noexcept {
    auto const light = worker.scene().random_light(rng_.random_float());

    return direct_light(light.ref, light.pdf, ray, position, intersection, worker);
}

float3 Tracking_single::direct_light(Light const& light, float light_pdf, Ray const& ray,
                                     float3 const& position, Intersection const& intersection,
                                     Worker& worker) noexcept {
    shape::Sample_to light_sample;
    if (!light.sample(position, ray.time, light_sampler(ray.depth), 0, worker, light_sample)) {
        return float3(0.f);
    }

    Ray shadow_ray(position, light_sample.wi, 0.f, light_sample.t, ray.depth, ray.time,
                   ray.wavelength);

    float3 tr;
    if (!worker.transmitted(shadow_ray, float3(0.f), intersection, Filter::Nearest, tr)) {
        return float3(0.f);
    }

    SOFT_ASSERT(all_finite(tr));

    //    auto const bxdf = material_sample.evaluate_f(light_sample.wi, evaluate_back);

    float const phase = 1.f / (4.f * Pi);

    float3 const radiance = light.evaluate(light_sample, Filter::Nearest, worker);

    return (phase * tr * radiance) / (light_sample.pdf * light_pdf);
}

sampler::Sampler& Tracking_single::material_sampler(uint32_t bounce) noexcept {
    if (Num_material_samplers > bounce) {
        return material_samplers_[bounce];
    }

    return sampler_;
}

sampler::Sampler& Tracking_single::light_sampler(uint32_t bounce) noexcept {
    if (Num_light_samplers > bounce) {
        return light_samplers_[bounce];
    }

    return sampler_;
}

Tracking_single_factory::Tracking_single_factory(uint32_t num_integrators) noexcept
    : integrators_(memory::allocate_aligned<Tracking_single>(num_integrators)) {}

Tracking_single_factory::~Tracking_single_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* Tracking_single_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Tracking_single(rng);
}

}  // namespace rendering::integrator::volume
