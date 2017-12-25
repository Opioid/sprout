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
	subsurface_(subsurface),
	sampler_(rng),
	material_samplers_{rng, rng, rng},
	light_samplers_{rng, rng, rng},
	transmittance_open_(rng, take_settings, settings.max_bounces),
	transmittance_closed_(rng, take_settings) {}

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

	transmittance_closed_.prepare(scene, num_samples_per_pixel);
}

void Pathtracer_MIS::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	sampler_.resume_pixel(sample, scramble);

	for (auto& s : material_samplers_) {
		s.resume_pixel(sample, scramble);
	}

	for (auto& s : light_samplers_) {
		s.resume_pixel(sample, scramble);
	}

	transmittance_closed_.resume_pixel(sample, scramble);
}

float4 Pathtracer_MIS::li(Ray& ray, Intersection& intersection, Worker& worker) {
	const uint32_t max_bounces = settings_.max_bounces;

	Sampler_filter filter = ray.is_primary() ? Sampler_filter::Undefined 
											 : Sampler_filter::Nearest;
	Bxdf_sample sample_result;

	float opacity = 0.f;
	bool requires_bounce = false;

	float3 throughput(1.f);
	float3 result(0.f);

	for (uint32_t i = ray.depth; ; ++i) {
		const float3 wo = -ray.direction;
		const auto& material_sample = intersection.sample(wo, ray.time, filter, worker);

		if ((ray.is_primary() || requires_bounce) && material_sample.same_hemisphere(wo)) {
			result += throughput * material_sample.radiance();

			if (i == (requires_bounce ? max_bounces + 1: max_bounces)) {
				break;
			}
		}

		if (material_sample.is_pure_emissive()) {
			opacity = 1.f;
			break;
		}

		const float3 direct_light = estimate_direct_light(ray, intersection, material_sample,
														  filter, worker,
														  sample_result, requires_bounce);
		result += throughput * direct_light;

		if (!intersection.hit()
		||  i >= (requires_bounce ? max_bounces : max_bounces - 1)
		||  0.f == sample_result.pdf) {
			opacity = 1.f; // I am not really happy with this being here
			break;
		}

		if (i > settings_.min_bounces) {
			const float q = std::max(spectrum::luminance(throughput),
									 settings_.path_continuation_probability);
			if (rendering::russian_roulette(throughput, q, sampler_.generate_sample_1D())) {
				break;
			}
		}

		if (requires_bounce) {
			if (settings_.disable_caustics && !ray.is_primary()) {
				break;
			}
		} else {
			ray.set_primary(false);
			filter = Sampler_filter::Nearest;
		}

		if (sample_result.type.test(Bxdf_type::Transmission)) {
			const float3 tli = resolve_transmission(ray, intersection,
													material_sample,
													Sampler_filter::Nearest,
													worker,
													sample_result);

			result += throughput * tli;
			if (0.f == sample_result.pdf) {
				break;
			}

			throughput *= sample_result.reflection;
		//	opacity += spectrum::luminance(tr);
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

		// For these cases we fall back to plain pathtracing
		if (requires_bounce) {
			const bool hit = worker.intersect_and_resolve_mask(ray, intersection, filter);

			float3 tr;
			const float3 vli = worker.volume_li(ray, tr);
			result += throughput * vli;
			throughput *= tr;

			if (!hit) {
				break;
			}
		}
	}

	return float4(result, opacity);
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

float3 Pathtracer_MIS::estimate_direct_light(const Ray& ray, Intersection& intersection,
											 const Material_sample& material_sample,
											 Sampler_filter filter,
											 Worker& worker, Bxdf_sample& sample_result,
											 bool& requires_bounce) {
	float3 result(0.f);

	const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

	if (Light_sampling::Strategy::Single == settings_.light_sampling.strategy) {
		for (uint32_t i = settings_.light_sampling.num_samples; i > 0; --i) {
			const float select = light_sampler(ray.depth, ray.properties).generate_sample_1D(1);

			const auto light = worker.scene().random_light(select);

			result += evaluate_light(light.ref, light.pdf, ray, ray_offset, 0,
									 intersection, material_sample, filter, worker);
		}

		result *= settings_.num_light_samples_reciprocal;
	} else {
		const auto& lights = worker.scene().lights();
		const float light_weight = num_lights_reciprocal_;
		for (uint32_t l = 0, len = static_cast<uint32_t>(lights.size()); l < len; ++l) {
			const auto& light = *lights[l];
			for (uint32_t i = settings_.light_sampling.num_samples; i > 0; --i) {
				result += evaluate_light(light, light_weight, ray, ray_offset, l,
										 intersection, material_sample, filter, worker);
			}
		}

		result *= settings_.num_light_samples_reciprocal * light_weight;
	}

	SOFT_ASSERT(math::all_finite_and_positive(result));

	// Material BSDF importance sample
	material_sample.sample(material_sampler(ray.depth, ray.properties), sample_result);

	// Those cases are handled outside
	requires_bounce = sample_result.type.test_any(Bxdf_type::Specular, Bxdf_type::Transmission);

	if (requires_bounce || 0.f == sample_result.pdf) {
		return result;
	}

	const bool is_translucent = material_sample.is_translucent();

	Ray secondary_ray(intersection.geo.p, sample_result.wi, ray_offset,
					  scene::Ray_max_t, ray.time, ray.depth + 1, ray.properties);

//	const bool hit = worker.intersect_and_resolve_mask(secondary_ray, intersection, filter);

	float3 ssstr;
	const bool hit = worker.intersect_and_resolve_mask(secondary_ray, intersection,
													   material_sample, filter, ssstr);

	const float3 weighted_reflection = sample_result.reflection / sample_result.pdf;

	// This might invalidate the contents of material_sample,
	// so we must not use the sample after this point (e.g. in the calling function)!
	// Important exceptions are the Specular and Transmission cases, which never come here.
	float3 vtr;
	const float3 vli = worker.volume_li(secondary_ray, vtr);
	vtr *= ssstr;
	result += weighted_reflection * vli;
	sample_result.reflection *= vtr;

	SOFT_ASSERT(math::all_finite_and_positive(result));

	if (!hit) {
		return result;
	}

	const uint32_t light_id = intersection.light_id();
	if (!Light::is_light(light_id)) {
		return result;
	}

	auto light = worker.scene().light(light_id);

	if (Light_sampling::Strategy::All == settings_.light_sampling.strategy) {
		light.pdf = num_lights_reciprocal_;
	}

	const float ls_pdf = light.ref.pdf(secondary_ray, intersection.geo, is_translucent,
									   Sampler_filter::Nearest, worker);

	if (0.f == ls_pdf) {
		return result;
	}

	const float3 wo = -sample_result.wi;

	// This will invalidate the contents of material_sample. See comment above.
	const auto& light_material_sample = intersection.sample(wo, ray.time, Sampler_filter::Nearest,
															worker);

	if (light_material_sample.same_hemisphere(wo)) {
		const float3 ls_energy = vtr * light_material_sample.radiance();

		const float weight = power_heuristic(sample_result.pdf, ls_pdf * light.pdf);

		result += weight * (ls_energy * weighted_reflection);
	}

	SOFT_ASSERT(math::all_finite_and_positive(result));

	return result;
}

float3 Pathtracer_MIS::evaluate_light(const Light& light, float light_weight, const Ray& history,
									  float ray_offset, uint32_t sampler_dimension,
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

	 Ray shadow_ray(intersection.geo.p, light_sample.shape.wi, ray_offset,
					light_sample.shape.t - ray_offset, history.time,
					history.depth, history.properties);

//	const float3 tv = worker.tinted_visibility(shadow_ray, filter);
	const float3 tv = worker.tinted_visibility(shadow_ray, intersection, material_sample, filter);
	if (math::any_greater_zero(tv)) {
		const float3 tr = worker.transmittance(shadow_ray);

		const auto bxdf = material_sample.evaluate(light_sample.shape.wi);

		const float light_pdf = light_sample.shape.pdf * light_weight;
		const float weight = power_heuristic(light_pdf, bxdf.pdf);

		return (weight / light_pdf) * (tv * tr) * (light_sample.radiance * bxdf.reflection);
	}

	return float3(0.f);
}

float3 Pathtracer_MIS::resolve_transmission(const Ray& ray, Intersection& intersection, 
											const Material_sample& sample, Sampler_filter filter, 
											Worker& worker, Bxdf_sample& sample_result) {
	if (sample.is_sss()) {
		return subsurface_.li(ray, intersection, sample, filter, 
							  worker, sample_result);
	} else {
		const auto& attenuation = sample.absorption_coefficient();

		if (intersection.prop->is_open()) {
			transmittance_open_.resolve(ray, intersection, attenuation,
										sampler_, filter, worker, sample_result);
		} else {
			transmittance_closed_.resolve(ray, intersection, attenuation,
										  sampler_, filter, worker, sample_result);
		}

		return float3::identity();
	}
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
