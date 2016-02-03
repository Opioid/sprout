#include "pathtracer_mis.hpp"
#include "integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "image/texture/sampler/sampler_2d_linear.inl"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "take/take_settings.hpp"
#include "base/color/color.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Pathtracer_MIS::Pathtracer_MIS(const take::Settings& take_settings,
							   math::random::Generator& rng,
							   const Settings& settings) :
	Integrator(take_settings, rng), settings_(settings), sampler_(rng, 1),
	transmittance_open_(take_settings, rng, settings.max_bounces),
	transmittance_closed_(take_settings, rng) {}

void Pathtracer_MIS::start_new_pixel(uint32_t num_samples) {
	sampler_.restart_and_seed(num_samples);
}

math::float4 Pathtracer_MIS::li(Worker& worker, scene::Ray& ray, bool volume, scene::Intersection& intersection) {
	scene::material::bxdf::Result sample_result;

	math::float3 throughput = math::float3(1.f, 1.f, 1.f);
	math::float3 result = math::float3::identity;
	float opacity = 0.f;
	bool primary_ray = 0 == ray.depth;

	for (uint32_t i = 0; i < settings_.max_bounces; ++i) {
		const image::texture::sampler::Sampler_2D* texture_sampler;

		if (primary_ray) {
			texture_sampler = &settings_.sampler_linear;
		} else {
			texture_sampler = &settings_.sampler_nearest;
		}

		if (!resolve_mask(worker, ray, intersection, *texture_sampler)) {
			break;
		}

		if (volume && i > 0) {
		//	throughput *= worker.transmittance(ray);
			math::float3 tr;
			math::float4 vli = worker.volume_li(ray, tr);
			result += throughput * vli.xyz();
			throughput *= tr;
		}

		math::float3 wo = -ray.direction;
		auto material = intersection.material();
		auto& material_sample = material->sample(intersection.geo, wo, ray.time, 1.f, *texture_sampler, worker.id());

		if (material_sample.same_hemisphere(wo)
		&& (primary_ray || sample_result.type.test(scene::material::bxdf::Type::Specular))) {
			result += throughput * material_sample.emission();
		}

		if (material_sample.is_pure_emissive()) {
			opacity = 1.f;
			break;
		}

		result += throughput * estimate_direct_light(worker, ray, intersection, material_sample, *texture_sampler);

		if (i == settings_.max_bounces - 1) {
			break;
		}

		material_sample.sample_evaluate(sampler_, sample_result);
		if (0.f == sample_result.pdf || math::float3::identity == sample_result.reflection) {
			break;
		}

		if (settings_.disable_caustics && !primary_ray
		&& sample_result.type.test(scene::material::bxdf::Type::Specular)) {
			break;
		}

		if (sample_result.type.test(scene::material::bxdf::Type::Transmission)) {
			math::float3 tr = resolve_transmission(worker, ray, intersection, material_sample.attenuation(),
												   settings_.sampler_nearest, sample_result);
			if (0.f == sample_result.pdf) {
				break;
			}

			throughput *= tr;
			opacity += /*1.f - sample_result.pdf **/ color::luminance(tr);
		} else {
			throughput *= sample_result.reflection / sample_result.pdf;
			opacity = 1.f;
		}

		if (!sample_result.type.test(scene::material::bxdf::Type::Specular)) {
			primary_ray = false;
		}

		float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = take_settings_.ray_max_t;
		++ray.depth;

		if (!worker.intersect(ray, intersection)) {
			break;
		}
	}

	return math::float4(result, opacity);
}

float power_heuristic(float fpdf, float gpdf) {
	float f2 = fpdf * fpdf;
	return f2 / (f2 + gpdf * gpdf);
}

math::float3 Pathtracer_MIS::estimate_direct_light(Worker& worker, const scene::Ray& ray,
												   const scene::Intersection& intersection,
												   const scene::material::Sample& material_sample,
												   const image::texture::sampler::Sampler_2D& texture_sampler) {
	math::float3 result = math::float3::identity;

	float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	scene::Ray shadow_ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t  = ray_offset;
	shadow_ray.depth  = ray.depth + 1;
	shadow_ray.tick_time   = ray.tick_time;

	for (uint32_t i = 0; i < settings_.num_light_samples; ++i) {
		float light_pdf;
		const scene::light::Light* light = worker.scene().montecarlo_light(rng_.random_float(), light_pdf);
		if (!light) {
			continue;
		}

		float light_pdf_reciprocal = 1.f / light_pdf;

		scene::entity::Composed_transformation transformation;
		light->transformation_at(ray.tick_time, transformation);

		// Light source importance sample
		scene::light::Sample light_sample;
		light->sample(transformation,
					  intersection.geo.p, material_sample.geometric_normal(), material_sample.is_translucent(),
					  settings_.sampler_nearest, sampler_, worker.node_stack(), light_sample);

		if (light_sample.shape.pdf > 0.f) {
			shadow_ray.set_direction(light_sample.shape.wi);
			shadow_ray.max_t = light_sample.shape.t - ray_offset;

			float mv = worker.masked_visibility(shadow_ray, texture_sampler);
			if (mv > 0.f) {
				math::float3 t = worker.transmittance(shadow_ray);

				float bxdf_pdf;
				math::float3 f = material_sample.evaluate(light_sample.shape.wi, bxdf_pdf);

				float weight = power_heuristic(light_sample.shape.pdf, bxdf_pdf);

				result += (weight / light_sample.shape.pdf * light_pdf_reciprocal)
					   * mv * t * light_sample.energy * f;
			}
		}

		// Material BSDF importance sample
		scene::material::bxdf::Result sample_result;
		material_sample.sample_evaluate(sampler_, sample_result);
		if (0.f == sample_result.pdf
		||  sample_result.type.test(scene::material::bxdf::Type::Specular)) {
			continue;
		}

		float ls_pdf = light->pdf(transformation, intersection.geo.p, sample_result.wi,
								  material_sample.is_translucent(), settings_.sampler_nearest, worker.node_stack());
		if (0.f == ls_pdf) {
			continue;
		}

		math::float3 wo = -sample_result.wi;
		shadow_ray.set_direction(sample_result.wi);
		shadow_ray.max_t = take_settings_.ray_max_t;

		scene::Intersection light_intersection;
		if (worker.intersect(shadow_ray, light_intersection)
		&&  resolve_mask(worker, shadow_ray, light_intersection, texture_sampler)) {
			if (light->equals(light_intersection.prop, light_intersection.geo.part)) {
				auto light_material = light_intersection.material();
				auto& light_material_sample = light_material->sample(light_intersection.geo, wo, ray.time, 1.f,
																	 settings_.sampler_nearest, worker.id());

				if (light_material_sample.same_hemisphere(wo)) {
					math::float3 t = worker.transmittance(shadow_ray);

					math::float3 ls_energy = t * light_material_sample.emission();

					float weight = power_heuristic(sample_result.pdf, ls_pdf);

					result += (weight / sample_result.pdf * light_pdf_reciprocal)
						   * ls_energy * sample_result.reflection;
				}
			}
		}
	}

	return settings_.num_light_samples_reciprocal * result;
}

math::float3 Pathtracer_MIS::resolve_transmission(Worker& worker, scene::Ray& ray,
												  scene::Intersection& intersection,
												  const math::float3& attenuation,
												  const image::texture::sampler::Sampler_2D& texture_sampler,
												  scene::material::bxdf::Result& sample_result) {
	if (intersection.prop->is_open()) {
		return transmittance_open_.resolve(worker, ray, intersection, attenuation,
										   sampler_, texture_sampler, sample_result);
	} else {
		return transmittance_closed_.resolve(worker, ray, intersection, attenuation,
											 sampler_, texture_sampler, sample_result);
	}
}

Pathtracer_MIS_factory::Pathtracer_MIS_factory(const take::Settings& take_settings,
											   uint32_t min_bounces, uint32_t max_bounces,
											   uint32_t num_light_samples, bool disable_caustics) :
	Integrator_factory(take_settings) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
	settings_.num_light_samples = num_light_samples;
	settings_.num_light_samples_reciprocal = 1.f / static_cast<float>(num_light_samples);
	settings_.disable_caustics = disable_caustics;
}

Integrator* Pathtracer_MIS_factory::create(math::random::Generator& rng) const {
	return new Pathtracer_MIS(take_settings_, rng, settings_);
}

}}}
