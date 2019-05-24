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
#include "tracking.hpp"

#include "base/debug/assert.hpp"

namespace rendering::integrator::volume {

Tracking_single::Tracking_single(rnd::Generator& rng, take::Settings const& take_settings) noexcept
    : Integrator(rng, take_settings), sampler_(rng) {}

void Tracking_single::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Tracking_single::start_pixel() noexcept {}
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
bool Tracking_single::transmittance(Ray const& ray, Worker& worker,
                                    float3& transmittance) noexcept {
    return Tracking::transmittance(ray, rng_, worker, transmittance);
}

Event Tracking_single::integrate(Ray& ray, Intersection& intersection, Filter filter,
                                 Worker& worker, float3& li, float3& transmittance) noexcept {
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

    if (scene::offset_f(ray.min_t) >= d || scene::Almost_ray_max_t_minus_epsilon <= d) {
        li            = float3(0.f);
        transmittance = float3(1.f);
        return Event::Pass;
    }

    SOFT_ASSERT(!worker.interface_stack().empty());

    auto const interface = worker.interface_stack().top();

    auto const& material = *intersection.material(worker);

    if (!material.is_scattering_volume()) {
        // Basically the "glass" case
        float3 const mu_a = material.absorption_coefficient(interface->uv, filter, worker);

        li            = float3(0.f);
        transmittance = attenuation(d - ray.min_t, mu_a);
        //	weight = float3(1.f);
        return Event::Pass;
    }

    if (material.is_heterogeneous_volume()) {
        auto const prop = worker.scene().prop(interface->prop);

        auto const shape = prop->shape();

        Transformation temp;
        auto const&    transformation = prop->transformation_at(interface->prop, ray.time, temp,
                                                             worker.scene());

        float3 const local_origin = transformation.world_to_object_point(ray.origin);
        float3 const local_dir    = transformation.world_to_object_vector(ray.direction);

        float3 const origin = shape->object_to_texture_point(local_origin);
        float3 const dir    = shape->object_to_texture_vector(local_dir);

        Ray local_ray(origin, dir, ray.min_t, ray.max_t);

        auto const& tree = *material.volume_tree();

        float3 w(1.f);
        for (; local_ray.min_t < d;) {
            if (Tracking::CM data; tree.intersect(local_ray, data)) {
                if (float t; Tracking::tracking(local_ray, data, material, 1.f, filter, rng_,
                                                worker, t, w)) {
                    li            = w * direct_light(ray, ray.point(t), intersection, worker);
                    transmittance = float3(0.f);
                    return Event::Pass;
                }
            }

            local_ray.min_t = local_ray.max_t + 0.00001f;
            local_ray.max_t = d;
        }

        li            = float3(0.f);
        transmittance = w;
        return Event::Pass;

    } else if (material.is_textured_volume()) {
        auto const mu = material.collision_coefficients(float2(0.f), filter, worker);

        float3 const extinction = mu.a + mu.s;

        float3 const scattering_albedo = mu.s / extinction;

        transmittance = math::exp(-(d - ray.min_t) * extinction);

        float const r = rng_.random_float();
        float const t = -std::log(1.f - r * (1.f - average(transmittance))) / average(extinction);

        float3 const p = ray.point(ray.min_t + t);

        float3 l = direct_light(ray, p, intersection, worker);

        l *= (1.f - transmittance) * scattering_albedo;

        li = l;
    } else {
        auto const mu = material.collision_coefficients();

        float3 const extinction = mu.a + mu.s;

        float3 const scattering_albedo = mu.s / extinction;

        transmittance = math::exp(-(d - ray.min_t) * extinction);

        float const r = rng_.random_float();
        float const t = -std::log(1.f - r * (1.f - average(transmittance))) / average(extinction);

        float3 const p = ray.point(ray.min_t + t);

        float3 l = direct_light(ray, p, intersection, worker);

        l *= (1.f - transmittance) * scattering_albedo;

        li = l;
    }

    return Event::Pass;
}

size_t Tracking_single::num_bytes() const noexcept {
    return sizeof(*this) + sampler_.num_bytes();
}

float3 Tracking_single::direct_light(Ray const& ray, float3 const& position,
                                     Intersection const& intersection, Worker& worker) noexcept {
    float3 result = float3(0.f);

    Ray shadow_ray;
    shadow_ray.origin = position;
    shadow_ray.min_t  = 0.f;
    shadow_ray.depth  = ray.depth + 1;
    shadow_ray.time   = ray.time;

    auto const light = worker.scene().random_light(rng_.random_float());

    scene::shape::Sample_to light_sample;
    if (light.ref.sample(position, ray.time, sampler_, 0, worker, light_sample)) {
        shadow_ray.set_direction(light_sample.wi);
        shadow_ray.max_t = light_sample.t;

        //	float3 const tv = worker.tinted_visibility(shadow_ray, Filter::Nearest);

        Intersection tintersection = intersection;
        tintersection.subsurface   = true;

        float3 tv;
        if (worker.transmitted_visibility(shadow_ray, float3(0.f), tintersection, Filter::Nearest,
                                          tv)) {
            float const phase = 1.f / (4.f * Pi);

            float3 const radiance = light.ref.evaluate(light_sample, Filter::Nearest, worker);

            result += (phase * tv * radiance) / (light.pdf * light_sample.pdf);
        }
    }

    return result;
}

Tracking_single_factory::Tracking_single_factory(take::Settings const& take_settings,
                                                 uint32_t              num_integrators) noexcept
    : Factory(take_settings, num_integrators),
      integrators_(memory::allocate_aligned<Tracking_single>(num_integrators)) {}

Tracking_single_factory::~Tracking_single_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* Tracking_single_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Tracking_single(rng, take_settings_);
}

}  // namespace rendering::integrator::volume
