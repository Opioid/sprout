#include "pathtracer_dl.hpp"
#include "integrator_helper.hpp"
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

Pathtracer_DL::Pathtracer_DL(const take::Settings& take_settings,
							 math::random::Generator& rng,
							 const Settings& settings) :
	Surface_integrator(take_settings, rng), settings_(settings), sampler_(rng, 1), transmission_(take_settings, rng) {
	light_samples_.reserve(settings.max_light_samples);
}

void Pathtracer_DL::start_new_pixel(uint32_t num_samples) {
	sampler_.restart(num_samples);
}

math::float3 Pathtracer_DL::li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) {
	scene::material::BxDF_result sample_result;
	scene::material::BxDF_result::Type previous_sample_type;
	float bxdf_pdf;

	math::float3 throughput = math::float3(1.f, 1.f, 1.f);
	math::float3 result = math::float3::identity;

	for (uint32_t i = 0; i < settings_.max_bounces; ++i) {
		bool primary_ray = 0 == i || previous_sample_type.test(scene::material::BxDF_type::Specular);

		const image::texture::sampler::Sampler_2D* texture_sampler;

		if (primary_ray) {
			texture_sampler = &settings_.sampler_linear;
		} else {
			texture_sampler = &settings_.sampler_nearest;
		}

		if (!resolve_mask(worker, ray, intersection, *texture_sampler)) {
			break;
		}

		math::float3 wo = -ray.direction;
		auto material = intersection.material();
		auto& material_sample = material->sample(intersection.geo, wo, *texture_sampler, worker.id());

		if (material_sample.same_hemisphere(wo) && primary_ray) {
			result += throughput * material_sample.emission();
		}

		if (material_sample.is_pure_emissive()) {
			break;
		}

		float ray_offset = take_settings_.ray_offset_modifier * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.min_t  = ray_offset;

		float light_pdf;
		const scene::light::Light* light = worker.scene().montecarlo_light(rng_.random_float(), light_pdf);
		if (light) {
			light->sample(ray.time, intersection.geo.p, intersection.geo.geo_n, settings_.sampler_nearest,
						  sampler_, settings_.max_light_samples, light_samples_);

			float num_samples_reciprocal = 1.f / static_cast<float>(light_samples_.size());

			for (auto& ls : light_samples_) {
				if (ls.shape.pdf > 0.f) {
					ray.set_direction(ls.shape.wi);
					ray.max_t = ls.shape.t - ray_offset;

					float mv = worker.masked_visibility(ray, *texture_sampler);
					if (mv > 0.f) {
						result += num_samples_reciprocal * mv
							   * (throughput * ls.energy * material_sample.evaluate(ls.shape.wi, bxdf_pdf))
							   / (light_pdf * ls.shape.pdf);
					}
				}
			}
		}

		material_sample.sample_evaluate(sampler_, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		if (ray.depth > 0 && settings_.disable_caustics
		&&  sample_result.type.test(scene::material::BxDF_type::Specular)) {
			break;
		}

		if (sample_result.type.test(scene::material::BxDF_type::Transmission)) {
			throughput *= transmission_.resolve(worker, ray, intersection, material_sample.attenuation(),
												sampler_, settings_.sampler_nearest, sample_result);

			if (0.f == sample_result.pdf) {
				break;
			}
		} else {
			throughput *= sample_result.reflection / sample_result.pdf;
		}

		previous_sample_type = sample_result.type;

		ray_offset = take_settings_.ray_offset_modifier * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = 1000.f;
		++ray.depth;

		if (!worker.intersect(ray, intersection)) {
			break;
		}
	}

	return result;
}

Pathtracer_DL_factory::Pathtracer_DL_factory(const take::Settings& take_settings,
											 uint32_t min_bounces, uint32_t max_bounces,
											 uint32_t max_light_samples, bool disable_caustics) :
	Surface_integrator_factory(take_settings) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
	settings_.max_light_samples = max_light_samples;
	settings_.disable_caustics = disable_caustics;
}

Surface_integrator* Pathtracer_DL_factory::create(math::random::Generator& rng) const {
	return new Pathtracer_DL(take_settings_, rng, settings_);
}

}


