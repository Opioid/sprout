#include "whitted.hpp"
#include "rendering/rendering_worker.hpp"
#include "image/texture/sampler/sampler_2d_linear.inl"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Whitted::Whitted(const take::Settings& take_settings,
				 math::random::Generator& rng, const Settings& settings) :
	Integrator(take_settings, rng), settings_(settings), sampler_(rng, 1) {}

void Whitted::start_new_pixel(uint32_t num_samples) {
	sampler_.restart_and_seed(num_samples);
}

math::float4 Whitted::li(Worker& worker, scene::Ray& ray,
						 bool volume, scene::Intersection& intersection) {
	math::float3 result = math::float3_identity;

	float opacity = intersection.opacity(worker, ray.time, Sampler_filter::Unknown);
	float throughput = opacity;

	while (opacity < 1.f) {
		if (opacity > 0.f) {
			result += throughput * shade(worker, ray, intersection);
		}

		ray.min_t = ray.max_t;
		ray.max_t = take_settings_.ray_max_t;
		if (!worker.intersect(ray, intersection)) {
			return math::float4(result, opacity);
		}

		throughput = (1.f - opacity) * intersection.opacity(worker, ray.time,
															Sampler_filter::Unknown);
		opacity += throughput;
	}

	result += throughput * shade(worker, ray, intersection);

	return math::float4(result, opacity);
}

math::float3 Whitted::shade(Worker& worker, const scene::Ray& ray,
							const scene::Intersection& intersection) {
	math::float3 result = math::float3_identity;

	math::float3 wo = -ray.direction;
	auto& material_sample = intersection.sample(worker, wo, ray.time,
												Sampler_filter::Unknown);

	if (material_sample.same_hemisphere(wo)) {
		result += material_sample.radiance();
	}

	if (material_sample.is_pure_emissive()) {
		return result;
	}

	result += estimate_direct_light(worker, ray, intersection, material_sample);

	return result;
}

math::float3 Whitted::estimate_direct_light(Worker& worker, const scene::Ray& ray,
											const scene::Intersection& intersection,
											const scene::material::Sample& material_sample) {
	math::float3 result = math::float3_identity;

	float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

	scene::Ray shadow_ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t  = ray_offset;
	shadow_ray.depth  = ray.depth + 1;
	shadow_ray.time   = ray.time;

	for (auto l : worker.scene().lights()) {
		for (uint32_t i = 0; i < settings_.num_light_samples; ++i) {
			scene::light::Sample light_sample;
			l->sample(ray.time,
					  intersection.geo.p, material_sample.geometric_normal(),
					  material_sample.is_translucent(), sampler_, worker,
					  Sampler_filter::Linear, light_sample);

			if (light_sample.shape.pdf > 0.f) {
				shadow_ray.set_direction(light_sample.shape.wi);
				shadow_ray.max_t = light_sample.shape.t - ray_offset;

				float mv = worker.masked_visibility(shadow_ray, Sampler_filter::Unknown);
				if (mv > 0.f) {
					math::float3 tr = worker.transmittance(shadow_ray);

					float bxdf_pdf;
					math::float3 f = material_sample.evaluate(light_sample.shape.wi, bxdf_pdf);

					result += mv * tr * light_sample.radiance * f / light_sample.shape.pdf;
				}
			}
		}
	}

	return settings_.num_light_samples_reciprocal * result;
}

Whitted_factory::Whitted_factory(const take::Settings& take_settings, uint32_t num_light_samples) :
	Integrator_factory(take_settings) {
	settings_.num_light_samples = num_light_samples;
	settings_.num_light_samples_reciprocal = 1.f / static_cast<float>(num_light_samples);
}

Integrator* Whitted_factory::create(math::random::Generator& rng) const {
	return new Whitted(take_settings_, rng, settings_);
}

}}}
