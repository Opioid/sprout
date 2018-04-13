#include "pathtracer_mis.hpp"
#include "sub/sub_integrator.hpp"
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
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"

#include "base/debug/assert.hpp"

namespace rendering::integrator::surface {

Pathtracer_MIS::Pathtracer_MIS(rnd::Generator& rng, const take::Settings& take_settings,
							   const Settings& settings, sub::Integrator& subsurface) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng),
	material_samplers_{rng, rng, rng},
	light_samplers_{rng, rng, rng},
	subsurface_(subsurface) {}

Pathtracer_MIS::~Pathtracer_MIS() {
	memory::safe_destruct(subsurface_);
}

void Pathtracer_MIS::prepare(const Scene& scene, uint32_t num_samples_per_pixel) {
	const uint32_t num_lights = static_cast<uint32_t>(scene.lights().size());

	num_lights_reciprocal_ = num_lights > 0 ? 1.f / static_cast<float>(num_lights) : 0.f;

	sampler_.resize(num_samples_per_pixel, 1, 1, 1);

	for (auto& s : material_samplers_) {
		s.resize(num_samples_per_pixel, 1, 1, 1);
	}

	const uint32_t num_light_samples = settings_.light_sampling.num_samples;

	if (Light_sampling::Strategy::Single == settings_.light_sampling.strategy) {
		for (auto& s : light_samplers_) {
			s.resize(num_samples_per_pixel, num_light_samples, 1, 2);
		}
	} else {
		for (auto& s : light_samplers_) {
			s.resize(num_samples_per_pixel, num_light_samples, num_lights, num_lights);
		}
	}

	subsurface_.prepare(scene, num_samples_per_pixel);
}

void Pathtracer_MIS::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	sampler_.resume_pixel(sample, scramble);

	for (auto& s : material_samplers_) {
		s.resume_pixel(sample, scramble);
	}

	for (auto& s : light_samplers_) {
		s.resume_pixel(sample, scramble);
	}

	subsurface_.resume_pixel(sample, scramble);
}

float3 Pathtracer_MIS::li(Ray& ray, Intersection& intersection, Worker& worker) {
	const uint32_t max_bounces = settings_.max_bounces;

	Sampler_filter filter = ray.is_primary() ? Sampler_filter::Undefined 
											 : Sampler_filter::Nearest;
	Bxdf_sample sample_result;

	bool treat_as_singular = ray.is_primary();

	bool was_subsurface = false;

	float3 throughput(1.f);
	float3 result(0.f);

	{
		const float3 wo = -ray.direction;
		const auto& material_sample = intersection.sample(wo, ray, filter, sampler_, worker);

		if (material_sample.same_hemisphere(wo)) {
			result += material_sample.radiance();

			if (material_sample.is_pure_emissive()) {
				return result;
			}
		}
	}

	for (uint32_t i = ray.depth;; ++i) {
		float3 wo = -ray.direction;
		const auto& material_sample = intersection.sample(wo, ray, filter, sampler_, worker);

		const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

		result += throughput * sample_lights(ray, ray_offset, intersection,
											 material_sample, filter, worker);;

		SOFT_ASSERT(math::all_finite_and_positive(result));

		// Material BSDF importance sample
		material_sample.sample(material_sampler(ray.depth, ray.properties), sample_result);

		if (0.f == sample_result.pdf) {
			break;
		}

		if (!was_subsurface) {
			treat_as_singular = sample_result.type.test_any(Bxdf_type::Specular,
															Bxdf_type::Transmission);
			if (treat_as_singular) {
				if (settings_.disable_caustics && !ray.is_primary()) {
					break;
				}
			} else {
				ray.set_primary(false);
				filter = Sampler_filter::Nearest;
			}
		}

		const bool was_was_subsurface = was_subsurface;
		was_subsurface = intersection.geo.subsurface;

		const bool is_translucent = material_sample.is_translucent();

		if (0.f == ray.wavelength) {
			ray.wavelength = sample_result.wavelength;
		}

		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = scene::Ray_max_t;
		if (material_sample.ior() > 1.f) {
			++ray.depth;
		}

		throughput *= sample_result.reflection / sample_result.pdf;

		if (sample_result.type.test(Bxdf_type::Transmission)) {
			if (intersection.same_hemisphere(sample_result.wi)) {
				worker.interface_stack().pop();
			} else {
				worker.interface_stack().push(intersection);
			}
		}

		if (!worker.interface_stack().empty()) {
			float3 vli;
			float3 vtr;
			float3 weight;
			const bool hit = worker.volume(ray, intersection, vli, vtr, weight);

			result += throughput * vli;
			vtr *= weight;
			throughput *= vtr;

			if (!hit) {
				break;
			}
		} else {
			const bool hit = worker.intersect_and_resolve_mask(ray, intersection, filter);

			float3 vtr;
			const float3 vli = worker.volume_li(ray, vtr);
			result += throughput * vli;
			throughput *= vtr;

			if (!hit) {
				break;
			}
		}

		SOFT_ASSERT(math::all_finite_and_positive(result));

		if (!was_was_subsurface) {
			float3 radiance;
			const bool pure_emissive = evaluate_light(ray, intersection, sample_result,
													  treat_as_singular, is_translucent,
													  filter, worker, radiance);

			result += throughput * radiance;

			if (pure_emissive) {
				break;
			}
		}

		if (i >= max_bounces - 1) {
			break;
		}

		if (i > settings_.min_bounces) {
			const float q = std::max(spectrum::luminance(throughput),
									 settings_.path_continuation_probability);
			if (rendering::russian_roulette(throughput, q, sampler_.generate_sample_1D())) {
				break;
			}
		}
	}

	return result;
}

size_t Pathtracer_MIS::num_bytes() const {
	size_t sampler_bytes = 0;

	for (const auto& s : material_samplers_) {
		sampler_bytes += s.num_bytes();
	}

	for (const auto& s : light_samplers_) {
		sampler_bytes += s.num_bytes();
	}

	return sizeof(*this) + sampler_.num_bytes() + sampler_bytes;
}

float3 Pathtracer_MIS::sample_lights(const Ray& ray, float ray_offset, Intersection& intersection,
									 const Material_sample& material_sample,
									 Sampler_filter filter, Worker& worker) {
	float3 result(0.f);

	if (1.f == material_sample.ior()) {
		return result;
	}

	const bool do_mis = !intersection.geo.subsurface;

	if (Light_sampling::Strategy::Single == settings_.light_sampling.strategy) {
		for (uint32_t i = settings_.light_sampling.num_samples; i > 0; --i) {
			const float select = light_sampler(ray.depth, ray.properties).generate_sample_1D(1);

			const auto light = worker.scene().random_light(select);

			result += evaluate_light(light.ref, light.pdf, ray, ray_offset, 0, do_mis,
									 intersection, material_sample, filter, worker);
		}

		result *= settings_.num_light_samples_reciprocal;
	} else {
		const auto& lights = worker.scene().lights();
		const float light_weight = num_lights_reciprocal_;
		for (uint32_t l = 0, len = static_cast<uint32_t>(lights.size()); l < len; ++l) {
			const auto& light = *lights[l];
			for (uint32_t i = settings_.light_sampling.num_samples; i > 0; --i) {
				result += evaluate_light(light, light_weight, ray, ray_offset, l, do_mis,
										 intersection, material_sample, filter, worker);
			}
		}

		result *= settings_.num_light_samples_reciprocal * light_weight;
	}

	return result;
}

float3 Pathtracer_MIS::evaluate_light(const Light& light, float light_weight, const Ray& history,
									  float ray_offset, uint32_t sampler_dimension, bool do_mis,
									  const Intersection& intersection,
									  const Material_sample& material_sample,
									  Sampler_filter filter, Worker& worker) {
	// Light source importance sample
	scene::light::Sample light_sample;
	if (!light.sample(intersection.geo.p, material_sample.geometric_normal(),
					  history.time, material_sample.is_translucent(),
					  light_sampler(history.depth, history.properties),
					  sampler_dimension, Sampler_filter::Nearest, worker, light_sample)) {
		return float3(0.f);
	}

	const float shadow_offset = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
	Ray shadow_ray(intersection.geo.p, light_sample.shape.wi, ray_offset,
				   light_sample.shape.t - shadow_offset, history.depth, history.time,
				   history.wavelength, history.properties);

	const float3 tv = worker.tinted_visibility(shadow_ray, intersection, filter);

	SOFT_ASSERT(math::all_finite_and_positive(tv));

	if (math::any_greater_zero(tv)) {
		const float3 tr = worker.transmittance(shadow_ray);

		SOFT_ASSERT(math::all_finite_and_positive(tr));

		const auto bxdf = material_sample.evaluate(light_sample.shape.wi);

		const float light_pdf = light_sample.shape.pdf * light_weight;
		const float weight = do_mis ? power_heuristic(light_pdf, bxdf.pdf) : 1.f;

		return (weight / light_pdf) * (tv * tr) * (light_sample.radiance * bxdf.reflection);
	}

	return float3(0.f);
}

bool Pathtracer_MIS::evaluate_light(const Ray& ray, const Intersection& intersection,
									Bxdf_sample sample_result, bool treat_as_singular,
									bool is_translucent, Sampler_filter filter,
									Worker& worker, float3& radiance) {
	const uint32_t light_id = intersection.light_id();
	if (!Light::is_light(light_id)) {
		radiance = float3::identity();
		return false;
	}

	float light_pdf = 0.f;

	if (!treat_as_singular) {
		auto light = worker.scene().light(light_id);

		if (Light_sampling::Strategy::All == settings_.light_sampling.strategy) {
			light.pdf = num_lights_reciprocal_;
		}

		const float ls_pdf = light.ref.pdf(ray, intersection.geo, is_translucent,
										   Sampler_filter::Nearest, worker);

		if (0.f == ls_pdf) {
			radiance = float3::identity();
			return true;
		}

		light_pdf = ls_pdf * light.pdf;
	}

	const float3 wo = -sample_result.wi;

	// This will invalidate the contents of previous previous material samples.
	const auto& light_material_sample = intersection.sample(wo, ray, filter,
															sampler_, worker);

	if (light_material_sample.same_hemisphere(wo)) {
		const float3 ls_energy = light_material_sample.radiance();

		const float weight = power_heuristic(sample_result.pdf, light_pdf);

		radiance = weight * ls_energy;
	} else {
		radiance = float3::identity();
	}

	SOFT_ASSERT(math::all_finite_and_positive(radiance));

	return !light_material_sample.is_pure_emissive();
}

sampler::Sampler& Pathtracer_MIS::material_sampler(uint32_t bounce, Ray::Properties properties) {
	if (Num_material_samplers > bounce && properties.test_not(Ray::Property::Recursive)) {
		return material_samplers_[bounce];
	}

	return sampler_;
}

sampler::Sampler& Pathtracer_MIS::light_sampler(uint32_t bounce, Ray::Properties properties) {
	if (Num_light_samplers > bounce && properties.test_not(Ray::Property::Recursive)) {
		return light_samplers_[bounce];
	}

	return sampler_;
}

Pathtracer_MIS_factory::Pathtracer_MIS_factory(const take::Settings& take_settings,
											   uint32_t num_integrators,
											   std::unique_ptr<sub::Factory> sub_factory,
											   uint32_t min_bounces, uint32_t max_bounces,
											   float path_termination_probability,
											   Light_sampling light_sampling,
											   bool enable_caustics) :
	Factory(take_settings),
	sub_factory_(std::move(sub_factory)),
	integrators_(memory::allocate_aligned<Pathtracer_MIS>(num_integrators)),
	settings_{
		min_bounces,
		max_bounces,
		1.f - path_termination_probability,
		light_sampling,
		1.f / static_cast<float>(light_sampling.num_samples),
		!enable_caustics
	} {}

Pathtracer_MIS_factory::~Pathtracer_MIS_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Pathtracer_MIS_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Pathtracer_MIS(rng, take_settings_, settings_,
												*sub_factory_->create(id, rng));
}

}
