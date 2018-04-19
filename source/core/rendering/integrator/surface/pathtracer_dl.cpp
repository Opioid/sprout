#include "pathtracer_dl.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"

#include <iostream>

namespace rendering::integrator::surface {

Pathtracer_DL::Pathtracer_DL(rnd::Generator& rng, const take::Settings& take_settings,
							 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Pathtracer_DL::prepare(const Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Pathtracer_DL::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	sampler_.resume_pixel(sample, scramble);
}

float3 Pathtracer_DL::li(Ray& ray, Intersection& intersection, Worker& worker) {
	Sampler_filter filter = ray.is_primary() ? Sampler_filter::Undefined
											 : Sampler_filter::Nearest;
	Bxdf_sample sample_result;

	bool treat_as_singular = ray.is_primary();

	float3 throughput(1.f);
	float3 result(0.f);

	bool was_subsurface = false;

	for (uint32_t i = ray.depth;; ++i) {
		const float3 wo = -ray.direction;
		auto& material_sample = intersection.sample(wo, ray, filter, sampler_, worker);

		if (treat_as_singular && material_sample.same_hemisphere(wo)) {
			result += throughput * material_sample.radiance();
		}

		if (material_sample.is_pure_emissive()) {
			break;
		}

		result += throughput * direct_light(ray, intersection, material_sample,
											filter, worker);
		if (i >= settings_.max_bounces - 1) {
			break;
		}

		if (i > settings_.min_bounces) {
			const float q = std::max(spectrum::luminance(throughput),
									 settings_.path_continuation_probability);
			if (rendering::russian_roulette(throughput, q, sampler_.generate_sample_1D())) {
				break;
			}
		}

		material_sample.sample(sampler_, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		if (!was_subsurface) {
			const bool singular = sample_result.type.test_any(Bxdf_type::Specular,
															  Bxdf_type::Transmission);

			if (singular) {
				if (material_sample.ior_greater_one()) {
					if (settings_.disable_caustics && !ray.is_primary()) {
						break;
					}
					treat_as_singular = true;
				} else {
					treat_as_singular = ray.is_primary();
				}
			} else {
				ray.set_primary(false);
				filter = Sampler_filter::Nearest;
				treat_as_singular = false;
			}
		}

		was_subsurface = intersection.geo.subsurface;

		throughput *= sample_result.reflection / sample_result.pdf;

		const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

		if (material_sample.ior_greater_one()) {
			ray.origin = intersection.geo.p;
			ray.set_direction(sample_result.wi);
			ray.min_t = ray_offset;
			ray.max_t = scene::Ray_max_t;
			++ray.depth;
		} else {
			ray.min_t = ray.max_t + ray_offset;
			ray.max_t = scene::Ray_max_t;
		}

		if (sample_result.type.test(Bxdf_type::Transmission)) {
			worker.interface_change(sample_result.wi, intersection);
		}

		if (!worker.interface_stack().empty()) {
			float3 vli;
			float3 vtr;
			float3 weight;
			const bool hit = worker.volume(ray, intersection, filter, vli, vtr, weight);

			result += throughput * vli;
			throughput *= vtr * weight;

			if (!hit) {
				break;
			}
		} else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
			break;
		}
	}

	return result;
}

float3 Pathtracer_DL::direct_light(const Ray& ray, const Intersection& intersection,
								   const Material_sample& material_sample,
								   Sampler_filter filter, Worker& worker) {
	float3 result(0.f);

	if (!material_sample.ior_greater_one()) {
		return result;
	}

	Ray shadow_ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t  = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	shadow_ray.depth  = ray.depth + 1;
	shadow_ray.time   = ray.time;

	for (uint32_t i = settings_.num_light_samples; i > 0; --i) {
		const auto light = worker.scene().random_light(rng_.random_float());

		scene::light::Sample light_sample;
		if (light.ref.sample(intersection.geo.p, material_sample.geometric_normal(), ray.time,
							 material_sample.is_translucent(), sampler_, 0,
							 Sampler_filter::Nearest, worker, light_sample)) {
			shadow_ray.set_direction(light_sample.shape.wi);
			const float offset = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
			shadow_ray.max_t = light_sample.shape.t - offset;

		//	const float3 tv = worker.tinted_visibility(shadow_ray, filter);
			const float3 tv = worker.tinted_visibility(shadow_ray, intersection, filter);
			if (math::any_greater_zero(tv)) {
				const float3 tr = worker.transmittance(shadow_ray);

//				if (math::all_greater_equal(tr, 1.f) && !intersection.geo.subsurface) {
//					std::cout << ray.length() << std::endl;
//				}

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
	settings_.disable_caustics = !enable_caustics;
}

Pathtracer_DL_factory::~Pathtracer_DL_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Pathtracer_DL_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Pathtracer_DL(rng, take_settings_, settings_);
}

}
