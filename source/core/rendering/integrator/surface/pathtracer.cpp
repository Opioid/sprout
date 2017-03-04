#include "pathtracer.hpp"
#include "integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "image/texture/sampler/sampler_linear_2d.inl"
#include "image/texture/sampler/sampler_nearest_2d.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_intersection.inl"
#include "scene/scene_ray.inl"
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

Pathtracer::Pathtracer(const take::Settings& take_settings,
					   rnd::Generator& rng,
					   const Settings& settings) :
	Integrator(take_settings, rng),
	settings_(settings),
	sampler_(rng),
	material_samplers_{rng, rng, rng},
	transmittance_(take_settings, rng),
	subsurface_(take_settings, rng) {}

void Pathtracer::prepare(const Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);

	for (auto& s : material_samplers_) {
		s.resize(num_samples_per_pixel, 1, 1, 1);
	}
}

void Pathtracer::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	sampler_.resume_pixel(sample, scramble);

	for (auto& s : material_samplers_) {
		s.resume_pixel(sample, scramble);
	}
}

float4 Pathtracer::li(Worker& worker, Ray& ray, Intersection& intersection) {
	Sampler_filter filter;
	Bxdf_result sample_result;
	Bxdf_result::Type_flag previous_sample_type;

	float3 throughput(1.f);
	float3 result(0.f);
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
			float3 tr;
			const float4 vli = worker.volume_li(ray, tr);
			result += throughput * vli.xyz();
			throughput *= tr;
		}

		opacity = 1.f;

		const float3 wo = -ray.direction;
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

			if (sampler_.generate_sample_1D() >= q) {
				break;
			}

			throughput /= q;
		}

		if (intersection.material()->is_subsurface()) {
			result += throughput * subsurface_.li(worker, ray, intersection);
		}

		material_sample.sample(material_sampler(i), sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		if (settings_.disable_caustics && ray.depth > 0
		&&  sample_result.type.test(Bxdf_type::Specular)) {
			break;
		}

		if (sample_result.type.test(Bxdf_type::Transmission)) {
			throughput *= transmittance_.resolve(worker, ray, intersection,
												 material_sample.attenuation(),
												 sampler_, Sampler_filter::Nearest,
												 sample_result);

			if (0.f == sample_result.pdf) {
				break;
			}
		} else {
			throughput *= sample_result.reflection / sample_result.pdf;
		}

		previous_sample_type = sample_result.type;

		const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = scene::Ray_max_t;
		++ray.depth;

		if (!worker.intersect(ray, intersection)) {
			break;
		}
	}

	return float4(result, opacity);
}

sampler::Sampler& Pathtracer::material_sampler(uint32_t bounce) {
	if (Num_material_samplers > bounce) {
		return material_samplers_[bounce];
	}

	return sampler_;
}

size_t Pathtracer::num_bytes() const {
	size_t sampler_bytes = 0;

	for (auto& s : material_samplers_) {
		sampler_bytes += s.num_bytes();
	}

	return sizeof(*this) + sampler_.num_bytes() + sampler_bytes;
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

Integrator* Pathtracer_factory::create(rnd::Generator& rng) const {
	return new Pathtracer(take_settings_, rng, settings_);
}

}}}
