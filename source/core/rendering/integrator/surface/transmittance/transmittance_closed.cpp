#include "transmittance_closed.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/scene_intersection.inl"
#include "take/take_settings.hpp"
#include "base/math/vector4.inl"
#include "base/math/ray.inl"

namespace rendering { namespace integrator { namespace surface { namespace transmittance {

Closed::Closed(const take::Settings &take_settings, rnd::Generator &rng) :
	integrator::Integrator(take_settings, rng) {}

void Closed::prepare(const scene::Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Closed::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Closed::resolve(Worker& worker, Ray& ray, Intersection& intersection,
					   float3_p attenuation, sampler::Sampler& sampler,
					   Sampler_filter filter, Bxdf_result& sample_result) {
	float3 throughput = sample_result.reflection / sample_result.pdf;
	float3 used_attenuation = attenuation;

	for (;;) {
		const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = scene::Ray_max_t;

		if (!worker.intersect(intersection.prop, ray, intersection)) {
			break;
		}

		const float3 wo = -ray.direction;
		auto& material_sample = intersection.sample(worker, wo, ray.time, filter);

		material_sample.sample(sampler, sample_result);
		if (0.f == sample_result.pdf
		||  float3::identity() == sample_result.reflection) {
			break;
		}

		if (material_sample.is_transmissive()) {
			used_attenuation = material_sample.attenuation();
		}

		throughput *= rendering::attenuation(ray.origin, intersection.geo.p, used_attenuation);
		throughput *= sample_result.reflection / sample_result.pdf;

		// Only inner reflections are handled here
		if (sample_result.type.test(Bxdf_type::Transmission)) {
			break;
		}
	}

	return throughput;
}

size_t Closed::num_bytes() const {
	return sizeof(*this);
}

}}}}
