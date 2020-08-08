#include "whitted.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/integrator/surface/surface_integrator.inl"
#include "rendering/rendering_worker.hpp"
#include "scene/light/light.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.inl"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_sample.hpp"

namespace rendering::integrator::surface {

Whitted::Whitted(Settings const& settings) : settings_(settings) {}

void Whitted::prepare(Scene const& scene, uint32_t num_samples_per_pixel) {
    uint32_t const num_lights = scene.num_lights();
    sampler_.resize(num_samples_per_pixel, settings_.num_light_samples, num_lights, num_lights);
}

void Whitted::start_pixel(rnd::Generator& rng) {
    sampler_.start_pixel(rng);
}

float4 Whitted::li(Ray& ray, Intersection& intersection, Worker& worker,
                   Interface_stack const& initial_stack) {
    worker.reset_interface_stack(initial_stack);

    //	float3 result(0.f);

    //	float3 const wo = -ray.direction;

    //	float3 opacity = intersection.thin_absorption(wo, ray.time, Filter::Undefined,
    // worker); 	float3 throughput = opacity;

    //	while (math::any_less_one(opacity)) {
    //		if (math::any_greater_zero(opacity)) {
    //			result += throughput * shade(ray, intersection, worker);
    //		}

    //		ray.min_t() = ray.max_t + take_settings_.ray_offset_factor *
    // intersection.geo.epsilon; 		ray.max_t() = scene::Ray_max_t; 		if
    // (!worker.intersect(ray, intersection)) { 			return float4(result,
    // spectrum::luminance(opacity));
    //		}

    //		throughput = (1.f - opacity) * intersection.thin_absorption(wo, ray.time,
    //																	Filter::Undefined,
    //																	worker);
    //		opacity += throughput;
    //	}

    //	result += throughput * shade(ray, intersection, worker);

    //	return float4(result, spectrum::luminance(opacity));

    return float4(shade(ray, intersection, worker), 1.f);
}

float3 Whitted::shade(Ray const& ray, Intersection const& intersection, Worker& worker) {
    float3 result(0.f);

    float3 const wo = -ray.direction;

    auto const& material_sample = intersection.sample(wo, ray, Filter::Undefined, false, sampler_,
                                                      worker);

    if (material_sample.same_hemisphere(wo)) {
        result += material_sample.radiance();
    }

    if (material_sample.is_pure_emissive()) {
        return result;
    }

    result += estimate_direct_light(ray, intersection, material_sample, worker);

    return result;
}

float3 Whitted::estimate_direct_light(Ray const& ray, Intersection const& intersection,
                                      Material_sample const& material_sample, Worker& worker) {
    float3 result(0.f);

    bool const translucent = material_sample.is_translucent();

    Ray shadow_ray;
    shadow_ray.origin = material_sample.offset_p(intersection.geo.p, intersection.subsurface,
                                                 translucent);
    //   shadow_ray.min_t() = 0.f;
    shadow_ray.depth = ray.depth;
    shadow_ray.time  = ray.time;

    for (uint32_t l = 0, len = worker.scene().num_lights(); l < len; ++l) {
        auto const& light = worker.scene().light(l);
        for (uint32_t i = 0, nls = settings_.num_light_samples; i < nls; ++i) {
            if (scene::shape::Sample_to light_sample;
                light.sample(intersection.geo.p, material_sample.geometric_normal(), ray.time,
                             translucent, sampler_, l, worker, light_sample)) {
                shadow_ray.set_direction(light_sample.wi);
                shadow_ray.max_t() = light_sample.t();

                float3 tr;
                if (!worker.transmitted(shadow_ray, material_sample.wo(), intersection,
                                        Filter::Undefined, tr)) {
                    continue;
                }

                auto const bxdf = material_sample.evaluate_f(light_sample.wi);

                float3 const radiance = light.evaluate(light_sample, Filter::Nearest, worker);

                result += (tr * radiance * bxdf.reflection) / light_sample.pdf();
            }
        }
    }

    return settings_.num_light_samples_reciprocal * result;
}

Whitted_pool::Whitted_pool(uint32_t num_integrators, uint32_t num_light_samples)
    : Typed_pool<Whitted>(num_integrators) {
    settings_.num_light_samples            = num_light_samples;
    settings_.num_light_samples_reciprocal = 1.f / float(num_light_samples);
}

Integrator* Whitted_pool::get(uint32_t id) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Whitted(settings_);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::surface
