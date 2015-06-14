#include "whitted.hpp"
#include "rendering/worker.hpp"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "scene/scene.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "take/take_settings.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"
#include <iostream>

namespace rendering {

Whitted::Whitted(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings) :
	Surface_integrator(take_settings, rng), settings_(settings), sampler_(rng, 1) {}

void Whitted::start_new_pixel(uint32_t num_samples) {
	sampler_.restart(num_samples);
}

math::float3 Whitted::li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) {
	math::float3 result = math::float3::identity;

	float opacity = intersection.opacity(settings_.sampler);

	while (opacity < 1.f) {
		if (opacity > 0.f) {
			result += opacity * shade(worker, ray, intersection);
		}

		ray.min_t = ray.max_t;
		ray.max_t = 1000.f;
		if (!worker.intersect(ray, intersection)) {
			return result;
		}

		opacity = (1.f - opacity) * intersection.opacity(settings_.sampler);
	}

	result += opacity * shade(worker, ray, intersection);

	return result;
}

math::float3 Whitted::shade(Worker& worker, const math::Oray& ray, const scene::Intersection& intersection) {
	math::float3 result = math::float3::identity;

	math::float3 wo = -ray.direction;
	auto& sample = intersection.material()->sample(intersection.geo, wo, settings_.sampler, worker.id());

	float bxdf_pdf;

	result += sample.emission();

	if (sample.is_pure_emissive()) {
		return result;
	}

	float ray_offset = take_settings_.ray_offset_modifier * intersection.geo.epsilon;
	math::Oray shadow_ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t  = ray_offset;

	for (auto l : worker.scene().lights()) {
		l->sample(ray.time, intersection.geo.p, intersection.geo.geo_n, settings_.sampler, sampler_, 1, light_samples_);

		for (auto& ls : light_samples_) {
			if (ls.pdf > 0.f) {
				shadow_ray.set_direction(ls.l);
				shadow_ray.max_t = ls.t - ray_offset;

				float mv = worker.masked_visibility(shadow_ray, settings_.sampler);
				if (mv > 0.f) {
					result += mv * (ls.energy * sample.evaluate(ls.l, bxdf_pdf)) / ls.pdf;
				}
			}
		}
	}

	return result;
}

Whitted_factory::Whitted_factory(const take::Settings& take_settings) : Surface_integrator_factory(take_settings) {}

Surface_integrator* Whitted_factory::create(math::random::Generator& rng) const {
	return new Whitted(take_settings_, rng, settings_);
}

}

