#include "whitted.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/rendering_worker.hpp"
#include "scene/light/light.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_sample.hpp"
#include "take/take_settings.hpp"

namespace rendering::integrator::surface {

Whitted::Whitted(rnd::Generator& rng, take::Settings const& take_settings,
                 Settings const& settings) noexcept
    : Integrator(rng, take_settings), settings_(settings), sampler_(rng) {}

void Whitted::prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept {
    uint32_t num_lights = static_cast<uint32_t>(scene.lights().size());
    sampler_.resize(num_samples_per_pixel, settings_.num_light_samples, num_lights, num_lights);
}

void Whitted::start_pixel() noexcept {
    sampler_.start_pixel();
}

float3 Whitted::li(Ray& ray, Intersection& intersection, Worker& worker,
                   Interface_stack const& initial_stack) noexcept {
    worker.reset_interface_stack(initial_stack);

    //	float3 result(0.f);

    //	float3 const wo = -ray.direction;

    //	float3 opacity = intersection.thin_absorption(wo, ray.time, Sampler_filter::Undefined,
    // worker); 	float3 throughput = opacity;

    //	while (math::any_less_one(opacity)) {
    //		if (math::any_greater_zero(opacity)) {
    //			result += throughput * shade(ray, intersection, worker);
    //		}

    //		ray.min_t = ray.max_t + take_settings_.ray_offset_factor * intersection.geo.epsilon;
    //		ray.max_t = scene::Ray_max_t;
    //		if (!worker.intersect(ray, intersection)) {
    //			return float4(result, spectrum::luminance(opacity));
    //		}

    //		throughput = (1.f - opacity) * intersection.thin_absorption(wo, ray.time,
    //																	Sampler_filter::Undefined,
    //																	worker);
    //		opacity += throughput;
    //	}

    //	result += throughput * shade(ray, intersection, worker);

    //	return float4(result, spectrum::luminance(opacity));

    return shade(ray, intersection, worker);
}

float3 Whitted::shade(Ray const& ray, Intersection const& intersection, Worker& worker) noexcept {
    float3 result(0.f);

    float3 const wo = -ray.direction;

    auto& material_sample = intersection.sample(wo, ray, Sampler_filter::Undefined, false, sampler_,
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
                                      const Material_sample& material_sample,
                                      Worker&                worker) noexcept {
    float3 result(0.f);

    float const ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

    Ray shadow_ray;
    shadow_ray.origin = intersection.geo.p;
    shadow_ray.min_t  = ray_offset;
    shadow_ray.depth  = ray.depth;
    shadow_ray.time   = ray.time;

    auto const& lights = worker.scene().lights();
    for (uint32_t l = 0, len = static_cast<uint32_t>(lights.size()); l < len; ++l) {
        auto const light = lights[l];
        for (uint32_t i = 0, nls = settings_.num_light_samples; i < nls; ++i) {
            if (scene::shape::Sample_to light_sample; light->sample(
                    intersection.geo.p, material_sample.geometric_normal(), ray.time,
                    material_sample.is_translucent(), sampler_, l, worker, light_sample)) {
                shadow_ray.set_direction(light_sample.wi);
                shadow_ray.max_t = light_sample.t - ray_offset;

                if (float3 tv; worker.transmitted_visibility(shadow_ray, intersection,
                                                             Sampler_filter::Undefined, tv)) {
                    auto const bxdf = material_sample.evaluate(light_sample.wi, true);

                    float3 const radiance = light->evaluate(light_sample, ray.time,
                                                            Sampler_filter::Nearest, worker);

                    result += (tv * radiance * bxdf.reflection) / light_sample.pdf;
                }
            }
        }
    }

    return settings_.num_light_samples_reciprocal * result;
}

size_t Whitted::num_bytes() const noexcept {
    return sizeof(*this) + sampler_.num_bytes();
}

Whitted_factory::Whitted_factory(take::Settings const& take_settings, uint32_t num_integrators,
                                 uint32_t num_light_samples) noexcept
    : Factory(take_settings), integrators_(memory::allocate_aligned<Whitted>(num_integrators)) {
    settings_.num_light_samples            = num_light_samples;
    settings_.num_light_samples_reciprocal = 1.f / static_cast<float>(num_light_samples);
}

Whitted_factory::~Whitted_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* Whitted_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Whitted(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::surface
