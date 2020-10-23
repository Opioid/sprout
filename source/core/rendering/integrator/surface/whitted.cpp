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

Whitted::Whitted() {}

void Whitted::prepare(Scene const& scene, uint32_t num_samples_per_pixel) {
    uint32_t const num_lights = scene.num_lights();
    sampler_.resize(num_samples_per_pixel, 1, num_lights, num_lights);
}

void Whitted::start_pixel(RNG& rng) {
    sampler_.start_pixel(rng);
}

float4 Whitted::li(Ray& ray, Intersection& isec, Worker& worker,
                   Interface_stack const& initial_stack) {
    worker.reset_interface_stack(initial_stack);

    //	float3 result(0.f);

    //	float3 const wo = -ray.direction;

    //	float3 opacity = isec.thin_absorption(wo, ray.time, Filter::Undefined,
    // worker); 	float3 throughput = opacity;

    //	while (math::any_less_one(opacity)) {
    //		if (math::any_greater_zero(opacity)) {
    //			result += throughput * shade(ray, isec, worker);
    //		}

    //		ray.min_t() = ray.max_t + take_settings_.ray_offset_factor *
    // isec.geo.epsilon; 		ray.max_t() = scene::Ray_max_t; 		if
    // (!worker.intersect(ray, isec)) { 			return float4(result,
    // spectrum::luminance(opacity));
    //		}

    //		throughput = (1.f - opacity) * isec.thin_absorption(wo, ray.time,
    //																	Filter::Undefined,
    //																	worker);
    //		opacity += throughput;
    //	}

    //	result += throughput * shade(ray, isec, worker);

    //	return float4(result, spectrum::luminance(opacity));

    return float4(shade(ray, isec, worker), 1.f);
}

float3 Whitted::shade(Ray const& ray, Intersection const& isec, Worker& worker) {
    float3 result(0.f);

    float3 const wo = -ray.direction;

    auto const& mat_sample = isec.sample(wo, ray, Filter::Undefined, false, sampler_, worker);

    if (mat_sample.same_hemisphere(wo)) {
        result += mat_sample.radiance();
    }

    if (mat_sample.is_pure_emissive()) {
        return result;
    }

    result += estimate_direct_light(ray, isec, mat_sample, worker);

    return result;
}

float3 Whitted::estimate_direct_light(Ray const& ray, Intersection const& isec,
                                      Material_sample const& mat_sample, Worker& worker) {
    float3 result(0.f);

    bool const translucent = mat_sample.is_translucent();

    Ray shadow_ray;
    shadow_ray.origin = mat_sample.offset_p(isec.geo.p, isec.subsurface, translucent);
    //   shadow_ray.min_t() = 0.f;
    shadow_ray.depth = ray.depth;
    shadow_ray.time  = ray.time;

    for (uint32_t l = 0, len = worker.scene().num_lights(); l < len; ++l) {
        auto const& light = worker.scene().light(l);

        if (scene::shape::Sample_to light_sample;
            light.sample(isec.geo.p, mat_sample.geometric_normal(), ray.time, translucent, sampler_,
                         l, worker, light_sample)) {
            shadow_ray.set_direction(light_sample.wi);
            shadow_ray.max_t() = light_sample.t();

            float3 tr;
            if (!worker.transmitted(shadow_ray, mat_sample.wo(), isec, Filter::Undefined, tr)) {
                continue;
            }

            auto const bxdf = mat_sample.evaluate_f(light_sample.wi);

            float3 const radiance = light.evaluate(light_sample, Filter::Nearest, worker);

            result += (tr * radiance * bxdf.reflection) / light_sample.pdf();
        }
    }

    return result;
}

Whitted_pool::Whitted_pool(uint32_t num_integrators) : Typed_pool<Whitted>(num_integrators) {}

Integrator* Whitted_pool::get(uint32_t id) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Whitted();
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::surface
