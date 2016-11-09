#include "pathtracer.hpp"
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
#include "base/math/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Pathtracer::Pathtracer(uint32_t num_samples_per_pixel,
					   const take::Settings& take_settings,
					   math::random::Generator& rng,
					   const Settings& settings) :
	Integrator(num_samples_per_pixel, take_settings, rng),
	settings_(settings),
	primary_sampler_(rng, num_samples_per_pixel),
	secondary_sampler_(rng, num_samples_per_pixel),
	transmittance_(num_samples_per_pixel, take_settings, rng) {}

void Pathtracer::resume_pixel(uint32_t sample, uint2 seed) {
	primary_sampler_.resume_pixel(sample, seed);
	secondary_sampler_.resume_pixel(sample, seed);
}

float4 Pathtracer::li(Worker& worker, scene::Ray& ray, bool /*volume*/,
					  scene::Intersection& intersection) {
	Sampler_filter filter;
	scene::material::bxdf::Result sample_result;
	scene::material::bxdf::Result::Type_flag previous_sample_type;

	float3 throughput = float3(1.f, 1.f, 1.f);
	float3 result = math::float3_identity;
	float opacity = 0.f;

	// pathtracer needs as many iterations as bounces, because it has no forward prediction
	for (uint32_t i = 0;; ++i) {
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

		opacity = 1.f;

		float3 wo = -ray.direction;
		auto& material_sample = intersection.sample(worker, wo, ray.time, filter);

		if (material_sample.same_hemisphere(wo)) {
			result += throughput * material_sample.radiance();
		}

		if (i == settings_.max_bounces) {
			break;
		}

		if (material_sample.is_pure_emissive()) {
			break;
		}

		// Russian roulette termination
		if (i > settings_.min_bounces) {
			float q = std::min(spectrum::luminance(throughput),
							   settings_.path_continuation_probability);

			if (secondary_sampler_.generate_sample_1D() >= q) {
				break;
			}

			throughput /= q;
		}

		sampler::Sampler* sampler;// = &secondary_sampler_;

		if (0 == i) {
			sampler = &primary_sampler_;
		} else {
			sampler = &secondary_sampler_;
		}

		material_sample.sample(*sampler, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		if (ray.depth > 0 && settings_.disable_caustics
		&&  sample_result.type.test(Bxdf_type::Specular)) {
			break;
		}

		if (sample_result.type.test(scene::material::bxdf::Type::Transmission)) {
			throughput *= transmittance_.resolve(worker, ray, intersection,
												 material_sample.attenuation(),
												 secondary_sampler_, Sampler_filter::Nearest,
												 sample_result);

			if (0.f == sample_result.pdf) {
				break;
			}
		} else {
			throughput *= sample_result.reflection / sample_result.pdf;
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

Pathtracer_factory::Pathtracer_factory(const take::Settings& take_settings,
									   uint32_t min_bounces, uint32_t max_bounces,
									   float path_termination_probability,
									   bool disable_caustics) :
	Factory(take_settings) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
	settings_.path_continuation_probability = 1.f - path_termination_probability;
	settings_.disable_caustics = disable_caustics;
}

Integrator* Pathtracer_factory::create(uint32_t num_samples_per_pixel,
									   math::random::Generator& rng) const {
	return new Pathtracer(num_samples_per_pixel, take_settings_, rng, settings_);
}

}}}
