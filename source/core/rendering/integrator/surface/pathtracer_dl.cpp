#include "pathtracer_dl.hpp"
#include "integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "image/texture/sampler/sampler_linear_2d.inl"
#include "image/texture/sampler/sampler_nearest_2d.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_intersection.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "base/spectrum/rgb.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Pathtracer_DL::Pathtracer_DL(rnd::Generator& rng, const take::Settings& take_settings,
							 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng),
	transmittance_(rng, take_settings)
{}

void Pathtracer_DL::prepare(const Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Pathtracer_DL::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	sampler_.resume_pixel(sample, scramble);
}

float4 Pathtracer_DL::li(Worker& worker, Ray& ray, Intersection& intersection) {
	Sampler_filter filter;
	Bxdf_result sample_result;
	Bxdf_result::Type_flag previous_sample_type;

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
			result += throughput * vli.xyz();
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

		if (i > settings_.min_bounces) {
			if (rendering::russian_roulette(throughput, settings_.path_continuation_probability,
											sampler_.generate_sample_1D())) {
				break;
			}
		}

		material_sample.sample(sampler_, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		if (ray.depth > 0 && !settings_.enable_caustics
		&&  sample_result.type.test(Bxdf_type::Specular)) {
			break;
		}

		if (sample_result.type.test(Bxdf_type::Transmission)) {
			float3 transmitted = transmittance_.resolve(worker, ray, intersection,
														material_sample.absorption_coffecient(),
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
		ray.max_t = scene::Ray_max_t;
		++ray.depth;

		if (!worker.intersect(ray, intersection)) {
			break;
		}
	}

	return float4(result, opacity);
}

float3 Pathtracer_DL::estimate_direct_light(Worker& worker, const Ray& ray,
											const Intersection& intersection,
											const Material_sample& material_sample,
											Sampler_filter filter) {
	float3 result = float3::identity();

	float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	Ray shadow_ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t  = ray_offset;
	shadow_ray.depth  = ray.depth + 1;
	shadow_ray.time   = ray.time;

	for (uint32_t i = 0; i < settings_.num_light_samples; ++i) {
		float light_pdf;
		const auto light = worker.scene().random_light(rng_.random_float(), light_pdf);
		if (!light) {
			continue;
		}

		scene::light::Sample light_sample;
		light->sample(intersection.geo.p, material_sample.geometric_normal(), ray.time,
					  material_sample.is_translucent(), sampler_, 0,
					  worker, Sampler_filter::Nearest, light_sample);

		if (light_sample.shape.pdf > 0.f) {
			shadow_ray.set_direction(light_sample.shape.wi);
			shadow_ray.max_t = light_sample.shape.t - ray_offset;

			float3 tv = worker.tinted_visibility(shadow_ray, filter);
			if (math::any_greater_zero(tv)) {
				float3 t = worker.transmittance(shadow_ray);

				float bxdf_pdf;
				float3 f = material_sample.evaluate(light_sample.shape.wi, bxdf_pdf);

				result += tv * t * light_sample.radiance * f / (light_pdf * light_sample.shape.pdf);
			}
		}
	}

	return settings_.num_light_samples_reciprocal * result;
}

size_t Pathtracer_DL::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

Pathtracer_DL_factory::Pathtracer_DL_factory(const take::Settings& take_settings,
											 uint32_t num_integrators,
											 uint32_t min_bounces, uint32_t max_bounces,
											 float path_termination_probability,
											 uint32_t num_light_samples, bool enable_caustics) :
	Factory(take_settings),
	integrators_(memory::allocate_aligned<Pathtracer_DL>(num_integrators)) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
	settings_.path_continuation_probability = 1.f - path_termination_probability;
	settings_.num_light_samples = num_light_samples;
	settings_.num_light_samples_reciprocal = 1.f / static_cast<float>(num_light_samples);
	settings_.enable_caustics = enable_caustics;
}

Pathtracer_DL_factory::~Pathtracer_DL_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Pathtracer_DL_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Pathtracer_DL(rng, take_settings_, settings_);
}

}}}
