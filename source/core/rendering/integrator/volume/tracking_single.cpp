#include "tracking_single.hpp"
#include "base/math/aabb.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
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

Tracking_single::Tracking_single(rnd::Generator& rng, take::Settings const& take_settings)
    : Integrator(rng, take_settings), sampler_(rng) {}

void Tracking_single::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Tracking_single::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}
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
*/
static inline void avg_history_probabilities(float mt, float3 const& mu_s, float3 const& mu_n,
                                             float3 const& w, float& ps, float& pn, float3& ws,
                                             float3& wn) {
    float const ms = math::average(mu_s * w);
    float const mn = math::average(mu_n * w);
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
math::average(mu_s * w); float const mn = math::average(mu_n * w); float const c = 1.f / (ms + mn);

        pn = mn * c;

        wn = (mu_n / (mt * pn));
}
*/
float3 Tracking_single::transmittance(Ray const& ray, Worker& worker) {
    return Tracking::transmittance(ray, rng_, worker);
}

bool Tracking_single::integrate(Ray& ray, Intersection& intersection, Sampler_filter filter,
                                Worker& worker, float3& li, float3& transmittance) {
    bool const hit = worker.intersect_and_resolve_mask(ray, intersection, filter);
    if (!hit) {
        li            = float3(0.f);
        transmittance = float3(1.f);
        return false;
    }

    float const d = ray.max_t;

    if (d - ray.min_t < 0.0005f) {
        li            = float3(0.f);
        transmittance = float3(1.f);
        //	weight = float3(1.f);
        return true;
    }

    SOFT_ASSERT(!worker.interface_stack().empty());

    auto const interface = worker.interface_stack().top();

    auto const& material = *intersection.material();

    if (!material.is_scattering_volume()) {
        // Basically the "glass" case
        float3 const mu_a = material.absorption_coefficient(interface->uv, filter, worker);

        li            = float3(0.f);
        transmittance = attenuation(d, mu_a);
        //	weight = float3(1.f);
        return true;
    }

    if (material.is_heterogeneous_volume()) {
        auto const shape = interface->prop->shape();

        Transformation temp;
        auto const&    transformation = interface->prop->transformation_at(ray.time, temp);

        if (auto const tree = material.volume_octree(); tree) {
            float3 const local_origin = transformation.world_to_object_point(ray.origin);
            float3 const local_dir    = transformation.world_to_object_vector(ray.direction);

            auto const   shape  = interface->prop->shape();
            float3 const origin = shape->object_to_texture_point(local_origin);
            float3 const dir    = shape->object_to_texture_vector(local_dir);

            Ray local_ray(origin, dir, ray.min_t, ray.max_t);

            float3 w(1.f);
            for (; local_ray.min_t < d;) {
                if (float mt; tree->intersect(local_ray, mt)) {
                    if (float t;
                        Tracking::track(local_ray, mt, material, filter, rng_, worker, t, w)) {
                        li            = w * direct_light(ray, ray.point(t), intersection, worker);
                        transmittance = float3(0.f);
                        return true;
                    }
                }

                local_ray.min_t = local_ray.max_t + 0.00001f;
                local_ray.max_t = d;
            }

            li            = float3(0.f);
            transmittance = w;
            return true;
        }

        float3 w(1.f);
        float  t = 0.f;

        float const mt = material.majorant_mu_t();
        while (true) {
            float const r = rng_.random_float();
            t             = t - std::log(1.f - r) / mt;
            if (t > d) {
                li            = float3(0.f);
                transmittance = w;
                return true;
            }

            float3 const p = ray.point(ray.min_t + t);

            float3 const local_p = transformation.world_to_object_point(p);
            float3 const uvw     = shape->object_to_texture_point(local_p);
            auto const   mu      = material.collision_coefficients(uvw, filter, worker);

            float3 const mu_t = mu.a + mu.s;

            float3 const mu_n = float3(mt) - mu_t;

            float  ps, pn;
            float3 ws, wn;
            // avg_probabilities(mt, mu_a, mu_s, mu_n, pa, ps, pn, wa, ws, wn);

            avg_history_probabilities(mt, mu.s, mu_n, w, ps, pn, ws, wn);

            float const r2 = rng_.random_float();
            if (r2 <= 1.f - pn) {
                transmittance = float3(0.f);
                SOFT_ASSERT(math::all_finite(ws));
                li = w * ws * direct_light(ray, p, intersection, worker);
                return true;
            } else {
                SOFT_ASSERT(math::all_finite(wn));

                //				if (!math::all_finite(wn)) {
                //					std::cout << "problem" << std::endl;
                //				}

                w *= wn;

                if (math::average(w) == 0.f) {
                    transmittance = w;
                    li            = float3(0.f);
                    return true;
                }
            }
        }
    } else {
        auto const mu = material.collision_coefficients(float2(0.f), filter, worker);

        float3 const extinction = mu.a + mu.s;

        float3 const scattering_albedo = mu.s / extinction;

        transmittance = math::exp(-d * extinction);

        float const r = rng_.random_float();
        float const scatter_distance =
            -std::log(1.f - r * (1.f - math::average(transmittance))) / math::average(extinction);

        float3 const p = ray.point(scatter_distance);

        float3 l = direct_light(ray, p, intersection, worker);

        l *= (1.f - transmittance) * scattering_albedo;

        li = l;
    }

    return true;
}

size_t Tracking_single::num_bytes() const {
    return sizeof(*this) + sampler_.num_bytes();
}

float3 Tracking_single::direct_light(Ray const& ray, f_float3 position, Worker& worker) {
    float3 result = float3::identity();

    Ray shadow_ray;
    shadow_ray.origin = position;
    shadow_ray.min_t  = 0.f;
    shadow_ray.depth  = ray.depth + 1;
    shadow_ray.time   = ray.time;

    auto const light = worker.scene().random_light(rng_.random_float());

    scene::light::Sample light_sample;
    if (light.ref.sample(position, float3(0.f, 0.f, 1.f), ray.time, true, sampler_, 0,
                         Sampler_filter::Nearest, worker, light_sample)) {
        shadow_ray.set_direction(light_sample.shape.wi);
        float const offset = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
        shadow_ray.max_t   = light_sample.shape.t - offset;

        float3 const tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);
        if (math::any_greater_zero(tv)) {
            float3 const tr = worker.transmittance(shadow_ray);

            float const phase = 1.f / (4.f * math::Pi);

            result +=
                (tv * tr) * (phase * light_sample.radiance) / (light.pdf * light_sample.shape.pdf);
        }
    }

    return result;
}

float3 Tracking_single::direct_light(Ray const& ray, f_float3 position,
                                     Intersection const& intersection, Worker& worker) {
    float3 result = float3::identity();

    Ray shadow_ray;
    shadow_ray.origin = position;
    shadow_ray.min_t  = 0.f;
    shadow_ray.depth  = ray.depth + 1;
    shadow_ray.time   = ray.time;

    auto const light = worker.scene().random_light(rng_.random_float());

    scene::light::Sample light_sample;
    if (light.ref.sample(position, ray.time, sampler_, 0, Sampler_filter::Nearest, worker,
                         light_sample)) {
        shadow_ray.set_direction(light_sample.shape.wi);
        float const offset = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
        shadow_ray.max_t   = light_sample.shape.t - offset;

        //	float3 const tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);

        Intersection tintersection   = intersection;
        tintersection.geo.subsurface = true;
        float3 const tv =
            worker.tinted_visibility(shadow_ray, tintersection, Sampler_filter::Nearest);
        if (math::any_greater_zero(tv)) {
            float3 const tr = worker.transmittance(shadow_ray);

            float const phase = 1.f / (4.f * math::Pi);

            result +=
                (tv * tr) * (phase * light_sample.radiance) / (light.pdf * light_sample.shape.pdf);
        }
    }

    return result;
}

Tracking_single_factory::Tracking_single_factory(take::Settings const& take_settings,
                                                 uint32_t              num_integrators)
    : Factory(take_settings, num_integrators),
      integrators_(memory::allocate_aligned<Tracking_single>(num_integrators)) {}

Tracking_single_factory::~Tracking_single_factory() {
    memory::free_aligned(integrators_);
}

Integrator* Tracking_single_factory::create(uint32_t id, rnd::Generator& rng) const {
    return new (&integrators_[id]) Tracking_single(rng, take_settings_);
}

}  // namespace rendering::integrator::volume
