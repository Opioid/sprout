#include "whitted.hpp"
#include "rendering/worker.hpp"
#include "image/texture/sampler/sampler_2d_linear.inl"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "scene/scene.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"
#include <iostream>

namespace rendering {

Whitted::Whitted(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings) :
	Surface_integrator(take_settings, rng), settings_(settings), sampler_(rng, 1) {
	light_samples_.reserve(settings.max_light_samples);
}

void Whitted::start_new_pixel(uint32_t num_samples) {
	sampler_.restart(num_samples);
}

math::float3 Whitted::li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) {
	math::float3 result = math::float3::identity;

	float opacity = intersection.opacity(settings_.sampler_linear);
	float throughput = opacity;

	while (opacity < 1.f) {
		if (opacity > 0.f) {
			result += throughput * shade(worker, ray, intersection);
		}

		ray.min_t = ray.max_t;
		ray.max_t = 1000.f;
		if (!worker.intersect(ray, intersection)) {
			return result;
		}

		throughput = (1.f - opacity) * intersection.opacity(settings_.sampler_linear);
		opacity   += throughput;
	}

	result += throughput * shade(worker, ray, intersection);

	return result;
}

math::float3 Whitted::shade(Worker& worker, const math::Oray& ray, const scene::Intersection& intersection) {
	math::float3 result = math::float3::identity;

	math::float3 wo = -ray.direction;
	auto& sample = intersection.material()->sample(intersection.geo, wo, settings_.sampler_linear, worker.id());

	float bxdf_pdf;

	result += sample.emission();

	if (sample.is_pure_emissive()) {
		return result;
	}

	float ray_offset = take_settings_.ray_offset_modifier * intersection.geo.epsilon;
	math::Oray shadow_ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t  = ray_offset;
	shadow_ray.depth  = ray.depth + 1;

	for (auto l : worker.scene().lights()) {
		l->sample(ray.time, intersection.geo.p, intersection.geo.geo_n, settings_.sampler_nearest, sampler_, settings_.max_light_samples, light_samples_);

		float num_samples_reciprocal = 1.f / static_cast<float>(light_samples_.size());

		for (auto& ls : light_samples_) {
			if (ls.shape.pdf > 0.f) {
				shadow_ray.set_direction(ls.shape.wi);
				shadow_ray.max_t = ls.shape.t - ray_offset;

				float mv = worker.masked_visibility(shadow_ray, settings_.sampler_linear);
				if (mv > 0.f) {
					result += num_samples_reciprocal * mv * (ls.energy * sample.evaluate(ls.shape.wi, bxdf_pdf)) / ls.shape.pdf;
				}
			}
		}
	}

	return result;
}

Whitted_factory::Whitted_factory(const take::Settings& take_settings, uint32_t max_light_samples) :
	Surface_integrator_factory(take_settings) {
	settings_.max_light_samples = max_light_samples;
}

Surface_integrator* Whitted_factory::create(math::random::Generator& rng) const {
	return new Whitted(take_settings_, rng, settings_);
}

}

