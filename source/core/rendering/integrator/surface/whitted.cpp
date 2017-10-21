#include "whitted.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_intersection.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "take/take_settings.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"

namespace rendering::integrator::surface {

Whitted::Whitted(rnd::Generator& rng, const take::Settings& take_settings,
				 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Whitted::prepare(const Scene& scene, uint32_t num_samples_per_pixel) {
	uint32_t num_lights = static_cast<uint32_t>(scene.lights().size());
	sampler_.resize(num_samples_per_pixel, settings_.num_light_samples, num_lights, num_lights);
}

void Whitted::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	sampler_.resume_pixel(sample, scramble);
}

float4 Whitted::li(Ray& ray, Intersection& intersection, Worker& worker) {
	float3 result(0.f);

	const float3 wo = -ray.direction;

	float3 opacity = intersection.thin_absorption(wo, ray.time, Sampler_filter::Undefined, worker);
	float3 throughput = opacity;

	while (math::any_lesser_one(opacity)) {
		if (math::any_greater_zero(opacity)) {
			result += throughput * shade(ray, intersection, worker);
		}

		ray.min_t = ray.max_t + take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.max_t = scene::Ray_max_t;
		if (!worker.intersect(ray, intersection)) {
			return float4(result, spectrum::luminance(opacity));
		}

		throughput = (1.f - opacity) * intersection.thin_absorption(wo, ray.time,
																	Sampler_filter::Undefined,
																	worker);
		opacity += throughput;
	}

	result += throughput * shade(ray, intersection, worker);

	return float4(result, spectrum::luminance(opacity));
}

float3 Whitted::shade(const Ray& ray, const Intersection& intersection, Worker& worker) {
	float3 result(0.f);

	const float3 wo = -ray.direction;
	auto& material_sample = intersection.sample(wo, ray.time, Sampler_filter::Undefined, worker);

	if (material_sample.same_hemisphere(wo)) {
		result += material_sample.radiance();
	}

	if (material_sample.is_pure_emissive()) {
		return result;
	}

	result += estimate_direct_light(ray, intersection, material_sample, worker);

	return result;
}

float3 Whitted::estimate_direct_light(const Ray& ray, const Intersection& intersection,
									  const Material_sample& material_sample, Worker& worker) {
	float3 result(0.f);

	const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

	Ray shadow_ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t  = ray_offset;
	shadow_ray.depth  = ray.depth;
	shadow_ray.time   = ray.time;

	const auto& lights = worker.scene().lights();
	for (uint32_t l = 0, len = static_cast<uint32_t>(lights.size()); l < len; ++l) {
		const auto light = lights[l];
		for (uint32_t i = 0, nls = settings_.num_light_samples; i < nls; ++i) {
			scene::light::Sample light_sample;
			if (light->sample(intersection.geo.p, material_sample.geometric_normal(), ray.time,
							  material_sample.is_translucent(), sampler_, l,
							  Sampler_filter::Nearest, worker, light_sample)) {
				shadow_ray.set_direction(light_sample.shape.wi);
				shadow_ray.max_t = light_sample.shape.t - ray_offset;

				const float3 tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Undefined);
				if (math::any_greater_zero(tv)) {
					const float3 tr = worker.transmittance(shadow_ray);

					float bxdf_pdf;
					const float3 f = material_sample.evaluate(light_sample.shape.wi, bxdf_pdf);

					result += (tv * tr) * (light_sample.radiance * f) / light_sample.shape.pdf;
				}
			}
		}
	}

	return settings_.num_light_samples_reciprocal * result;
}

size_t Whitted::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

Whitted_factory::Whitted_factory(const take::Settings& take_settings, uint32_t num_integrators,
								 uint32_t num_light_samples) :
	Factory(take_settings),
	integrators_(memory::allocate_aligned<Whitted>(num_integrators)) {
	settings_.num_light_samples = num_light_samples;
	settings_.num_light_samples_reciprocal = 1.f / static_cast<float>(num_light_samples);
}

Whitted_factory::~Whitted_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Whitted_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Whitted(rng, take_settings_, settings_);
}

}
