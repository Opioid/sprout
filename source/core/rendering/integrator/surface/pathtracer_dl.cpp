#include "pathtracer_dl.hpp"
#include "integrator_helper.hpp"
#include "rendering/worker.hpp"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "scene/scene.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/surrounding/surrounding.hpp"
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

Pathtracer_DL::Pathtracer_DL(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings) :
	Surface_integrator(take_settings, rng), settings_(settings), sampler_(rng, 1) {}

void Pathtracer_DL::start_new_pixel(uint32_t num_samples) {
	sampler_.restart(num_samples);
}

math::float3 Pathtracer_DL::li(Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) {
	sampler_.start_iteration(subsample);

	scene::material::BxDF_result sample_result;
	scene::material::BxDF_result::Type previous_sample_type;
	math::float3 previous_sample_attenuation = math::float3(1.f, 1.f, 1.f);
	float bxdf_pdf;

	bool hit = true;
	math::float3 throughput = math::float3(1.f, 1.f, 1.f);
	math::float3 result = math::float3::identity;

	for (uint32_t i = 0; i < settings_.max_bounces; ++i) {
		if (!resolve_mask(worker, ray, intersection)) {
			hit = false;
			break;
		}

		math::float3 wo = -ray.direction;
		auto material = intersection.material();
		auto& material_sample = material->sample(intersection.geo, wo, settings_.sampler, worker.id());

		if (material_sample.same_hemisphere(wo)) {
			if (0 == i || previous_sample_type.test(scene::material::BxDF_type::Specular)) {
				result += throughput * material_sample.emission();
			}
		} else {
			throughput *= attenuation(ray.origin, intersection.geo.p, previous_sample_attenuation);
		}

		float ray_offset = take_settings_.ray_offset_modifier * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.min_t  = ray_offset;
		++ray.depth;

		float light_pdf;
		scene::light::Light* light = worker.scene().montecarlo_light(rng_.random_float(), light_pdf);
		if (light) {
			light->sample(ray.time, intersection.geo.p, intersection.geo.geo_n, sampler_, 1, light_samples_);

			auto& ls = light_samples_[0];
			if (ls.pdf > 0.f) {
				ray.set_direction(ls.l);
				ray.max_t = ls.t - ray_offset;

				float mv = worker.masked_visibility(ray, settings_.sampler);
				if (mv > 0.f) {
					result += mv * (throughput * ls.energy * material_sample.evaluate(ls.l, bxdf_pdf)) / (light_pdf * ls.pdf);
				}
			}
		}

		material_sample.sample_evaluate(sampler_, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		throughput *= sample_result.reflection / sample_result.pdf;

		previous_sample_type = sample_result.type;
		previous_sample_attenuation = material_sample.attenuation();

		ray.set_direction(sample_result.wi);
		ray.max_t = 1000.f;

		hit = worker.intersect(ray, intersection);
		if (!hit) {
			break;
		}
	}

//	if (!hit) {
	if (!hit && previous_sample_type.test(scene::material::BxDF_type::Specular)) {
		math::float3 r = worker.scene().surrounding()->sample(ray);
		result += throughput * r;
	}

	return result;
}

bool Pathtracer_DL::resolve_mask(Worker& worker, math::Oray& ray, scene::Intersection& intersection) {
	float opacity = intersection.opacity(settings_.sampler);

	while (opacity < 1.f) {
		if (opacity > 0.f && opacity > rng_.random_float()) {
			return true;
		}

		// We never change the ray origin and just slide along the segment instead.
		// This seems to be more robust than setting the new origin from the last intersection.
		// Possible indicator of imprecision issues in other parts of the code, but this seems to work well enough.
		ray.min_t = ray.max_t;
		ray.max_t = 1000.f;
		if (!worker.intersect(ray, intersection)) {
			return false;
		}

		opacity = intersection.opacity(settings_.sampler);
	}

	return true;
}

Pathtracer_DL_factory::Pathtracer_DL_factory(const take::Settings& take_settings, uint32_t min_bounces, uint32_t max_bounces) :
	Surface_integrator_factory(take_settings) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
}

Surface_integrator* Pathtracer_DL_factory::create(math::random::Generator& rng) const {
	return new Pathtracer_DL(take_settings_, rng, settings_);
}

}


