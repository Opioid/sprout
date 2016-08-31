#include "pathtracer_mis2.hpp"
#include "integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_intersection.inl"
#include "take/take_settings.hpp"
#include "base/spectrum/rgb.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Pathtracer_MIS2::Pathtracer_MIS2(const take::Settings& take_settings,
								 math::random::Generator& rng,
								 const Settings& settings) :
	Integrator(take_settings, rng),
	settings_(settings),
	sampler_(rng, 1),
	transmittance_open_(take_settings, rng, settings.max_bounces),
	transmittance_closed_(take_settings, rng) {}

void Pathtracer_MIS2::start_new_pixel(uint32_t num_samples) {
	sampler_.restart_and_seed(num_samples);
}

float4 Pathtracer_MIS2::li(Worker& worker, scene::Ray& ray, bool volume,
						   scene::Intersection& intersection) {
	Sampler_filter filter = Sampler_filter::Unknown;
	scene::material::bxdf::Result sample_result;

	float3 throughput = float3(1.f, 1.f, 1.f);
	float3 result = math::float3_identity;
	float opacity = 0.f;
	bool primary_ray = 0 == ray.depth;

	if (!resolve_mask(worker, ray, intersection, filter)) {
		return float4(result, opacity);
	}

	for (uint32_t i = 0; ; ++i) {
		if (volume && i > 0) {
			float3 tr;
			float4 vli = worker.volume_li(ray, tr);
			result += throughput * vli.xyz;
			throughput *= tr;
		}

		float3 wo = -ray.direction;
		auto& material_sample = intersection.sample(worker, wo, ray.time, filter);

		if (material_sample.same_hemisphere(wo)
		&& (primary_ray || sample_result.type.test(Bxdf_type::Specular)
						|| sample_result.type.test(Bxdf_type::Transmission))) {
			result += throughput * material_sample.radiance();
		}

		if (material_sample.is_pure_emissive()) {
			opacity = 1.f;
			break;
		}

		result += throughput * estimate_direct_light(worker, ray, intersection,
													 material_sample, filter, sample_result);

		if (!intersection.hit()
		||  i == settings_.max_bounces - 1
		||  0.f == sample_result.pdf
		||  math::float3_identity == sample_result.reflection) {
			break;
		}

		// Russian roulette termination
		if (i > settings_.min_bounces) {
			float q = std::min(spectrum::luminance(throughput),
							   settings_.path_continuation_probability);

			if (sampler_.generate_sample_1D() >= q) {
				break;
			}

			throughput /= q;
		}

		if (sample_result.type.test(Bxdf_type::Specular)) {
			if (settings_.disable_caustics && !primary_ray) {
				break;
			}
		} else {
			primary_ray = false;
			filter = Sampler_filter::Nearest;
		}

		if (sample_result.type.test(Bxdf_type::Transmission)) {
			float3 tr = resolve_transmission(worker, ray, intersection,
											 material_sample.attenuation(),
											 Sampler_filter::Nearest, sample_result);
			if (0.f == sample_result.pdf) {
				break;
			}

			throughput *= tr;
			opacity += spectrum::luminance(tr);
		} else {
			throughput *= sample_result.reflection / sample_result.pdf;
			opacity = 1.f;
		}

		float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = take_settings_.ray_max_t;
		++ray.depth;

		// For these cases we fallback to plain pathtracing
		if (sample_result.type.test(Bxdf_type::Specular)
		||  sample_result.type.test(Bxdf_type::Transmission)) {
			if (!intersect_and_resolve_mask(worker, ray, intersection, filter)) {
				break;
			}
		}
	}

	return float4(result, opacity);
}

float3 Pathtracer_MIS2::estimate_direct_light(Worker& worker, const scene::Ray& ray,
											  scene::Intersection& intersection,
											  const scene::material::Sample& material_sample,
											  Sampler_filter filter,
											  Bxdf_result& sample_result) {
	float3 result = math::float3_identity;

	float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	scene::Ray shadow_ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.depth  = ray.depth + 1;
	shadow_ray.time   = ray.time;

	if (Light_sampling::Strategy::One == settings_.light_sampling.strategy) {
		for (uint32_t i = 0; i < settings_.light_sampling.num_samples; ++i) {
			float light_pdf;
			const scene::light::Light* light = worker.scene().montecarlo_light(rng_.random_float(),
																			   light_pdf);
			if (!light) {
				continue;
			}

			float light_pdf_reciprocal = 1.f / light_pdf;

			shadow_ray.min_t = ray_offset;

			result += evaluate_light(light, light_pdf_reciprocal, worker, shadow_ray,
									 intersection, material_sample, filter, sample_result);
		}

		return settings_.num_light_samples_reciprocal * result;
	} else {
		float light_weight = static_cast<float>(worker.scene().lights().size());
		for (const auto light : worker.scene().lights()) {
			for (uint32_t i = 0; i < settings_.light_sampling.num_samples; ++i) {
				shadow_ray.min_t = ray_offset;

				result += evaluate_light(light, light_weight, worker, shadow_ray,
										 intersection, material_sample, filter, sample_result);
			}
		}

		return (settings_.num_light_samples_reciprocal / light_weight) * result;
	}
}

float3 Pathtracer_MIS2::evaluate_light(const scene::light::Light* light, float light_weight,
									   Worker& worker, scene::Ray& ray,
									   scene::Intersection& intersection,
									   const scene::material::Sample& material_sample,
									   Sampler_filter filter,
									   Bxdf_result& sample_result) {
	float3 result = math::float3_identity;

	scene::entity::Composed_transformation temp;
	auto& transformation = light->transformation_at(ray.time, temp);

	// Light source importance sample
	scene::light::Sample light_sample;
	light->sample(transformation, ray.time, intersection.geo.p,
				  material_sample.geometric_normal(), material_sample.is_translucent(),
				  sampler_, worker, Sampler_filter::Nearest, light_sample);

	if (light_sample.shape.pdf > 0.f) {
		ray.set_direction(light_sample.shape.wi);

		float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.max_t = light_sample.shape.t - ray_offset;

		float mv = worker.masked_visibility(ray, filter);
		if (mv > 0.f) {
			float3 t = worker.transmittance(ray);

			float bxdf_pdf;
			float3 f = material_sample.evaluate(light_sample.shape.wi, bxdf_pdf);

			float weight = power_heuristic(light_sample.shape.pdf, bxdf_pdf);

			result += (weight / light_sample.shape.pdf * light_weight)
				   * mv * t * light_sample.radiance * f;
		}
	}

	// Material BSDF importance sample
	material_sample.sample(sampler_, sample_result);

	// For these cases we fallback to plain pathtracing
	if (0.f == sample_result.pdf
	||  sample_result.type.test(Bxdf_type::Specular)
	||  sample_result.type.test(Bxdf_type::Transmission)) {
		return result;
	}

	ray.set_direction(sample_result.wi);
	ray.max_t = take_settings_.ray_max_t;

	if (intersect_and_resolve_mask(worker, ray, intersection, filter)) {
		if (light->equals(intersection.prop, intersection.geo.part)) {
			float ls_pdf = light->pdf(transformation, intersection.geo.p,
									  sample_result.wi, material_sample.is_translucent(),
									  worker, Sampler_filter::Nearest);
			if (0.f == ls_pdf) {
				return result;
			}

			float3 wo = -sample_result.wi;
			auto& light_material_sample = intersection.sample(worker, wo, ray.time,
															  Sampler_filter::Nearest);

			if (light_material_sample.same_hemisphere(wo)) {
				float3 t = worker.transmittance(ray);

				float3 ls_energy = t * light_material_sample.radiance();

				float weight = power_heuristic(sample_result.pdf, ls_pdf);

				result += (weight / sample_result.pdf * light_weight)
					   * ls_energy * sample_result.reflection;
			}
		}
	}

	return result;
}

float3 Pathtracer_MIS2::resolve_transmission(Worker& worker, scene::Ray& ray,
											 scene::Intersection& intersection,
											 const float3& attenuation,
											 Sampler_filter filter,
											 Bxdf_result& sample_result) {
	if (intersection.prop->is_open()) {
		return transmittance_open_.resolve(worker, ray, intersection, attenuation,
										   sampler_, filter, sample_result);
	} else {
		return transmittance_closed_.resolve(worker, ray, intersection, attenuation,
											 sampler_, filter, sample_result);
	}
}

Pathtracer_MIS2_factory::Pathtracer_MIS2_factory(const take::Settings& take_settings,
												 uint32_t min_bounces, uint32_t max_bounces,
												 float path_termination_probability,
												 Light_sampling light_sampling,
												 bool disable_caustics) :
	Factory(take_settings) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
	settings_.path_continuation_probability = 1.f - path_termination_probability;
	settings_.light_sampling = light_sampling;
	settings_.num_light_samples_reciprocal = 1.f / static_cast<float>(light_sampling.num_samples);
	settings_.disable_caustics = disable_caustics;
}

Integrator* Pathtracer_MIS2_factory::create(math::random::Generator& rng) const {
	return new Pathtracer_MIS2(take_settings_, rng, settings_);
}

}}}
