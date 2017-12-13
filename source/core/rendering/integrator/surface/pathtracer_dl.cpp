#include "pathtracer_dl.hpp"
#include "integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "image/texture/sampler/sampler_linear_2d.inl"
#include "image/texture/sampler/sampler_nearest_2d.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "base/spectrum/rgb.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

namespace rendering::integrator::surface {

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

float4 Pathtracer_DL::li(Ray& ray, Intersection& intersection, Worker& worker) {
	Sampler_filter filter = Sampler_filter::Undefined;
	Bxdf_sample sample_result;

	float opacity = 0.f;
	bool primary_ray = 0 == ray.depth;
	bool requires_bounce = false;

	float3 throughput(1.f);
	float3 result(0.f);

	for (uint32_t i = ray.depth;; ++i) {
		const float3 wo = -ray.direction;
		auto& material_sample = intersection.sample(wo, ray.time, filter, worker);

		if ((primary_ray || requires_bounce) && material_sample.same_hemisphere(wo)) {
			result += throughput * material_sample.radiance();
		}

		if (material_sample.is_pure_emissive()) {
			opacity = 1.f;
			break;
		}

		result += throughput * estimate_direct_light(ray, intersection, material_sample,
													 filter, worker);

		if (i >= settings_.max_bounces - 1) {
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

		requires_bounce = sample_result.type.test_any(Bxdf_type::Specular, Bxdf_type::Transmission);

		if (sample_result.type.test(Bxdf_type::Specular)) {
			if (!settings_.enable_caustics && !primary_ray) {
				break;
			}
		} else {
			primary_ray = false;
			filter = Sampler_filter::Nearest;
		}

		if (sample_result.type.test(Bxdf_type::Transmission)) {
			const float3 tr = transmittance_.resolve(ray, intersection,
													 material_sample.absorption_coeffecient(),
													 sampler_, Sampler_filter::Nearest,
													 worker, sample_result);
			if (0.f == sample_result.pdf) {
				break;
			}

			throughput *= tr;
			opacity += 1.f - sample_result.pdf * spectrum::luminance(tr);
		} else {
			throughput *= sample_result.reflection / sample_result.pdf;
			opacity = 1.f;
		}

		const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = scene::Ray_max_t;
		++ray.depth;

		const bool hit = worker.intersect_and_resolve_mask(ray, intersection, filter);

		float3 tr;
		const float3 vli = worker.volume_li(ray, primary_ray, tr);
		result += throughput * vli;
		throughput *= tr;

		if (!hit) {
			break;
		}
	}

	return float4(result, opacity);
}

float3 Pathtracer_DL::estimate_direct_light(const Ray& ray, const Intersection& intersection,
											const Material_sample& material_sample,
											Sampler_filter filter, Worker& worker) {
	float3 result = float3::identity();

	const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	Ray shadow_ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t  = ray_offset;
	shadow_ray.depth  = ray.depth + 1;
	shadow_ray.time   = ray.time;

	for (uint32_t i = settings_.num_light_samples; i > 0; --i) {
		const auto light = worker.scene().random_light(rng_.random_float());

		scene::light::Sample light_sample;
		if (light.ref.sample(intersection.geo.p, material_sample.geometric_normal(), ray.time,
							 material_sample.is_translucent(), sampler_, 0,
							 Sampler_filter::Nearest, worker, light_sample)) {
			shadow_ray.set_direction(light_sample.shape.wi);
			shadow_ray.max_t = light_sample.shape.t - ray_offset;

			const float3 tv = worker.tinted_visibility(shadow_ray, filter);
			if (math::any_greater_zero(tv)) {
				const float3 tr = worker.transmittance(shadow_ray);

				const auto bxdf = material_sample.evaluate(light_sample.shape.wi);

				result += (tv * tr) * (light_sample.radiance * bxdf.reflection)
						/ (light.pdf * light_sample.shape.pdf);
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

}
