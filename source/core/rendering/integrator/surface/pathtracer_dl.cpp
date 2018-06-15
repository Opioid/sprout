#include "pathtracer_dl.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

#include <iostream>

namespace rendering::integrator::surface {

Pathtracer_DL::Pathtracer_DL(rnd::Generator& rng, take::Settings const& take_settings,
                             Settings const& settings)
    : Integrator(rng, take_settings), settings_(settings), sampler_(rng) {}

void Pathtracer_DL::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Pathtracer_DL::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
    sampler_.resume_pixel(sample, scramble);
}

float3 Pathtracer_DL::li(Ray& ray, Intersection& intersection, Worker& worker) {
    Sampler_filter filter = Sampler_filter::Undefined;

    Bxdf_sample sample_result;

    bool primary_ray       = true;
    bool treat_as_singular = true;

    float3 throughput(1.f);
    float3 result(0.f);

    for (uint32_t i = ray.depth;; ++i) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = settings_.avoid_caustics && !primary_ray &&
                                    worker.interface_stack().top_is_vacuum();

        auto& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                    worker);

        if (treat_as_singular && material_sample.same_hemisphere(wo)) {
            result += throughput * material_sample.radiance();
        }

        if (material_sample.is_pure_emissive()) {
            break;
        }

        result += throughput * direct_light(ray, intersection, material_sample, filter, worker);

        if (ray.depth >= settings_.max_bounces - 1) {
            break;
        }

        if (ray.depth > settings_.min_bounces) {
            float const q = settings_.path_continuation_probability;
            if (rendering::russian_roulette(throughput, q, sampler_.generate_sample_1D())) {
                break;
            }
        }

        material_sample.sample(sampler_, sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.test_any(Bxdf_type::Specular, Bxdf_type::Transmission)) {
            if (material_sample.ior_greater_one()) {
                if (avoid_caustics) {
                    break;
                }

                bool const scattering = intersection.material()->is_scattering_volume();
                treat_as_singular     = scattering ? primary_ray : true;
            }
        } else {
            primary_ray       = false;
            filter            = Sampler_filter::Nearest;
            treat_as_singular = false;
        }

        throughput *= sample_result.reflection / sample_result.pdf;

        float const ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

        if (material_sample.ior_greater_one()) {
            ray.origin = intersection.geo.p;
            ray.set_direction(sample_result.wi);
            ray.min_t = ray_offset;
            ray.max_t = scene::Ray_max_t;
            ++ray.depth;
        } else {
            ray.min_t = ray.max_t + ray_offset;
            ray.max_t = scene::Ray_max_t;
        }

        if (sample_result.type.test(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, intersection);
        }

        if (!worker.interface_stack().empty()) {
            float3     vli;
            float3     vtr;
            bool const hit = worker.volume(ray, intersection, filter, vli, vtr);

            result += throughput * vli;
            throughput *= vtr;

            if (!hit) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }
    }

    return result;
}

float3 Pathtracer_DL::direct_light(Ray const& ray, Intersection const& intersection,
                                   const Material_sample& material_sample, Sampler_filter filter,
                                   Worker& worker) {
    float3 result(0.f);

    if (!material_sample.ior_greater_one()) {
        return result;
    }

    Ray shadow_ray;
    shadow_ray.origin = intersection.geo.p;
    shadow_ray.min_t  = take_settings_.ray_offset_factor * intersection.geo.epsilon;
    shadow_ray.depth  = ray.depth + 1;
    shadow_ray.time   = ray.time;

    for (uint32_t i = settings_.num_light_samples; i > 0; --i) {
        auto const light = worker.scene().random_light(rng_.random_float());

        scene::light::Sample_to light_sample;
        if (light.ref.sample(intersection.geo.p, material_sample.geometric_normal(), ray.time,
                             material_sample.is_translucent(), sampler_, 0, Sampler_filter::Nearest,
                             worker, light_sample)) {
            shadow_ray.set_direction(light_sample.shape.wi);
            float const offset = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
            shadow_ray.max_t   = light_sample.shape.t - offset;

            //	float3 const tv = worker.tinted_visibility(shadow_ray, filter);
            float3 const tv = worker.tinted_visibility(shadow_ray, intersection, filter);
            if (math::any_greater_zero(tv)) {
                float3 const tr = worker.transmittance(shadow_ray);

                auto const bxdf = material_sample.evaluate(light_sample.shape.wi);

                result += (tv * tr) * (light_sample.radiance * bxdf.reflection) /
                          (light.pdf * light_sample.shape.pdf);
            }
        }
    }

    return settings_.num_light_samples_reciprocal * result;
}

size_t Pathtracer_DL::num_bytes() const {
    return sizeof(*this) + sampler_.num_bytes();
}

Pathtracer_DL_factory::Pathtracer_DL_factory(take::Settings const& take_settings,
                                             uint32_t num_integrators, uint32_t min_bounces,
                                             uint32_t max_bounces,
                                             float    path_termination_probability,
                                             uint32_t num_light_samples, bool enable_caustics)
    : Factory(take_settings),
      integrators_(memory::allocate_aligned<Pathtracer_DL>(num_integrators)) {
    settings_.min_bounces                   = min_bounces;
    settings_.max_bounces                   = max_bounces;
    settings_.path_continuation_probability = 1.f - path_termination_probability;
    settings_.num_light_samples             = num_light_samples;
    settings_.num_light_samples_reciprocal  = 1.f / static_cast<float>(num_light_samples);
    settings_.avoid_caustics                = !enable_caustics;
}

Pathtracer_DL_factory::~Pathtracer_DL_factory() {
    memory::free_aligned(integrators_);
}

Integrator* Pathtracer_DL_factory::create(uint32_t id, rnd::Generator& rng) const {
    return new (&integrators_[id]) Pathtracer_DL(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::surface
