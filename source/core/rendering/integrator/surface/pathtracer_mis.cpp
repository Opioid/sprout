#include "pathtracer_mis.hpp"
#include "integrator_helper.hpp"
#include "rendering/worker.hpp"
#include "image/texture/sampler/sampler_2d_linear.inl"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "scene/scene.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/prop/prop_intersection.inl"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"
#include "base/math/print.hpp"

// #include <iostream>

namespace rendering {

Pathtracer_MIS::Pathtracer_MIS(const take::Settings& take_settings,
							   math::random::Generator& rng,
							   const Settings& settings) :
	Surface_integrator(take_settings, rng), settings_(settings), sampler_(rng, 1), transmission_(take_settings, rng) {
	light_samples_.reserve(settings.max_light_samples);
}

void Pathtracer_MIS::start_new_pixel(uint32_t num_samples) {
	sampler_.restart(num_samples);
}

math::float3 Pathtracer_MIS::li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) {
	scene::material::BxDF_result sample_result;
	scene::material::BxDF_result::Type previous_sample_type;

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

		result += throughput * estimate_direct_light(worker, ray, intersection, material_sample, *texture_sampler);

		material_sample.sample_evaluate(sampler_, sample_result);
		if (0.f == sample_result.pdf || math::float3::identity == sample_result.reflection) {
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

		float ray_offset = take_settings_.ray_offset_modifier * intersection.geo.epsilon;
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

float power_heuristic(float fpdf, float gpdf) {
	float f2 = fpdf * fpdf;
	return f2 / (f2 + gpdf * gpdf);
}

math::float3 Pathtracer_MIS::estimate_direct_light(Worker& worker, const math::Oray& ray,
												   const scene::Intersection& intersection,
												   const scene::material::Sample& material_sample,
												   const image::texture::sampler::Sampler_2D& texture_sampler) {
	float light_pdf;
	const scene::light::Light* light = worker.scene().montecarlo_light(rng_.random_float(), light_pdf);
	if (!light) {
		return 	math::float3::identity;
	}

	math::float3 result = math::float3::identity;

	float ray_offset = take_settings_.ray_offset_modifier * intersection.geo.epsilon;
	math::Oray shadow_ray = ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t  = ray_offset;
	++shadow_ray.depth;

	scene::entity::Composed_transformation transformation;
	light->transformation_at(ray.time, transformation);

	light->sample(transformation, intersection.geo.p, intersection.geo.geo_n, settings_.sampler_nearest,
				  sampler_, settings_.max_light_samples, light_samples_);

	float num_samples_reciprocal = 1.f / static_cast<float>(light_samples_.size());

	for (auto& ls : light_samples_) {
		if (ls.shape.pdf > 0.f) {
			shadow_ray.set_direction(ls.shape.wi);
			shadow_ray.max_t = ls.shape.t - ray_offset;

			float mv = worker.masked_visibility(shadow_ray, texture_sampler);
			if (mv > 0.f) {
				float bxdf_pdf;
				math::float3 f = material_sample.evaluate(ls.shape.wi, bxdf_pdf);

				float weight = power_heuristic(ls.shape.pdf, bxdf_pdf);

				result += num_samples_reciprocal * (weight / ls.shape.pdf) * mv * ls.energy * f;
			}
		}
	}

	for (size_t i = 0, len = light_samples_.size(); i < len; ++i) {
		scene::material::BxDF_result sample_result;
		material_sample.sample_evaluate(sampler_, sample_result);
		if (0.f == sample_result.pdf
		||  sample_result.type.test(scene::material::BxDF_type::Specular)) {
			continue;
		}

		float ls_pdf = light->pdf(transformation, intersection.geo.p, sample_result.wi, settings_.sampler_nearest);
		if (0.f == ls_pdf) {
			continue;
		}

		float weight = power_heuristic(sample_result.pdf, ls_pdf);

		math::float3 wo = -sample_result.wi;
		shadow_ray.set_direction(sample_result.wi);
		shadow_ray.max_t = 1000.f;

		scene::Intersection light_intersection;
		if (worker.intersect(shadow_ray, light_intersection)
		&&  resolve_mask(worker, shadow_ray, light_intersection, texture_sampler)) {
			if (light->equals(light_intersection.prop, light_intersection.geo.part)) {
				auto light_material = light_intersection.material();
				auto& light_material_sample = light_material->sample(light_intersection.geo, wo,
																	 settings_.sampler_nearest, worker.id());

				if (light_material_sample.same_hemisphere(wo)) {
					math::float3 ls_energy = light_material_sample.emission();
					result += num_samples_reciprocal * (weight / sample_result.pdf)
						   * ls_energy * sample_result.reflection;
				}
			}
		}
	}

	return result / light_pdf;
}

Pathtracer_MIS_factory::Pathtracer_MIS_factory(const take::Settings& take_settings,
											   uint32_t min_bounces, uint32_t max_bounces,
											   uint32_t max_light_samples, bool disable_caustics) :
	Surface_integrator_factory(take_settings) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
	settings_.max_light_samples = max_light_samples;
	settings_.disable_caustics = disable_caustics;
}

Surface_integrator* Pathtracer_MIS_factory::create(math::random::Generator& rng) const {
	return new Pathtracer_MIS(take_settings_, rng, settings_);
}

}



