#include "pathtracer_dl.hpp"
#include "integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "image/texture/sampler/sampler_2d_linear.inl"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_intersection.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "take/take_settings.hpp"
#include "base/spectrum/rgb.inl"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Pathtracer_DL::Pathtracer_DL(const take::Settings& take_settings,
							 rnd::Generator& rng,
							 const Settings& settings) :
	Integrator(take_settings, rng),
	settings_(settings),
	sampler_(rng),
	transmittance_(take_settings, rng)
{}

void Pathtracer_DL::prepare(const scene::Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Pathtracer_DL::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	sampler_.resume_pixel(sample, scramble);
}

float4 Pathtracer_DL::li(Worker& worker, scene::Ray& ray, bool /*volume*/,
						 scene::Intersection& intersection) {
	scene::material::Sampler_settings::Filter filter;
	scene::material::bxdf::Result sample_result;
	scene::material::bxdf::Result::Type_flag previous_sample_type;

	float3 throughput(1.f);
	float3 result(0.f);
	float opacity = 0.f;

	for (uint32_t i = 0; i < settings_.max_bounces; ++i) {
		bool primary_ray = 0 == i || previous_sample_type.test(Bxdf_type::Specular);

		if (primary_ray) {
			filter = Sampler_filter::Unknown;
		} else {
			filter = Sampler_filter::Nearest;
		}

		if (!resolve_mask(worker, ray, intersection, filter)) {
			break;
		}

		if (i > 0) {
		//	throughput *= worker.transmittance(ray);
			float3 tr;
			float4 vli = worker.volume_li(ray, tr);
			result += throughput * vli.xyz;
			throughput *= tr;
		}

		float3 wo = -ray.direction;
		auto& material_sample = intersection.sample(worker, wo, ray.time, filter);

		if (material_sample.same_hemisphere(wo)
		&& (primary_ray || sample_result.type.test(Bxdf_type::Specular))) {
			result += throughput * material_sample.radiance();
		}

		if (material_sample.is_pure_emissive()) {
			opacity = 1.f;
			break;
		}

		result += throughput * estimate_direct_light(worker, ray, intersection,
													 material_sample, filter);

		if (i == settings_.max_bounces - 1) {
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

		material_sample.sample(sampler_, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		if (ray.depth > 0 && settings_.disable_caustics
		&&  sample_result.type.test(Bxdf_type::Specular)) {
			break;
		}

		if (sample_result.type.test(Bxdf_type::Transmission)) {
			float3 transmitted = transmittance_.resolve(worker, ray, intersection,
														material_sample.attenuation(),
														sampler_, Sampler_filter::Nearest,
														sample_result);
			if (0.f == sample_result.pdf) {
				break;
			}

			throughput *= transmitted;
			opacity += 1.f - sample_result.pdf * spectrum::luminance(transmitted);
		} else {
			throughput *= sample_result.reflection / sample_result.pdf;
			opacity = 1.f;
		}

		previous_sample_type = sample_result.type;

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

	return float4(result, opacity);
}

float3 Pathtracer_DL::estimate_direct_light(Worker& worker, const scene::Ray& ray,
											const scene::Intersection& intersection,
											const scene::material::Sample& material_sample,
											Sampler_filter filter) {
	float3 result = math::float3_identity;

	float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	scene::Ray shadow_ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t  = ray_offset;
	shadow_ray.depth  = ray.depth + 1;
	shadow_ray.time   = ray.time;

	for (uint32_t i = 0; i < settings_.num_light_samples; ++i) {
		float light_pdf;
		const auto light = worker.scene().random_light(rng_.random_float(),light_pdf);
		if (!light) {
			continue;
		}

		scene::light::Sample light_sample;
		light->sample(ray.time, intersection.geo.p, material_sample.geometric_normal(),
					  material_sample.is_translucent(), sampler_, 0,
					  worker, Sampler_filter::Nearest, light_sample);

		if (light_sample.shape.pdf > 0.f) {
			shadow_ray.set_direction(light_sample.shape.wi);
			shadow_ray.max_t = light_sample.shape.t - ray_offset;

			float mv = worker.masked_visibility(shadow_ray, filter);
			if (mv > 0.f) {
				float3 t = worker.transmittance(shadow_ray);

				float bxdf_pdf;
				float3 f = material_sample.evaluate(light_sample.shape.wi, bxdf_pdf);

				result += mv * t * light_sample.radiance * f / (light_pdf * light_sample.shape.pdf);
			}
		}
	}

	return settings_.num_light_samples_reciprocal * result;
}

Pathtracer_DL_factory::Pathtracer_DL_factory(const take::Settings& take_settings,
											 uint32_t min_bounces, uint32_t max_bounces,
											 float path_termination_probability,
											 uint32_t num_light_samples, bool disable_caustics) :
	Factory(take_settings) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
	settings_.path_continuation_probability = 1.f - path_termination_probability;
	settings_.num_light_samples = num_light_samples;
	settings_.num_light_samples_reciprocal = 1.f / static_cast<float>(num_light_samples);
	settings_.disable_caustics = disable_caustics;
}

Integrator* Pathtracer_DL_factory::create(rnd::Generator& rng) const {
	return new Pathtracer_DL(take_settings_, rng, settings_);
}

}}}
