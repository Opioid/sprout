#include "transmittance_closed.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/prop/prop_intersection.inl"
#include "base/math/vector4.inl"

namespace rendering { namespace integrator { namespace surface { namespace transmittance {

Closed::Closed(rnd::Generator& rng, const take::Settings& take_settings) :
	integrator::Integrator(rng, take_settings)
	/*samplers_{rng, rng}*/ {}

void Closed::prepare(const scene::Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {
//	for (auto& s : samplers_) {
//		s.resize(num_samples_per_pixel, 1, 1, 1);
//	}
}

void Closed::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {
//	for (auto& s : samplers_) {
//		s.resume_pixel(sample, scramble);
//	}
}

float3 Closed::resolve(const Ray& ray, Intersection& intersection,
					   const float3& absorption_coffecient, sampler::Sampler& sampler,
					   Sampler_filter filter, Worker& worker, Bxdf_sample& sample_result) {
	float3 throughput = sample_result.reflection / sample_result.pdf;
	float3 used_absorption_coffecient = absorption_coffecient;

	Ray tray;
	tray.time  = ray.time;
	tray.depth = ray.depth;

	const float ray_offset_factor = take_settings_.ray_offset_factor;

	for (uint32_t i = ray.depth;; ++i) {
		const float ray_offset = ray_offset_factor * intersection.geo.epsilon;
		tray.origin = intersection.geo.p;
		tray.set_direction(sample_result.wi);
		tray.min_t = ray_offset;
		tray.max_t = scene::Ray_max_t;

		if (!worker.intersect(intersection.prop, tray, intersection)) {
			break;
		}

		const float3 wo = -tray.direction;
		auto& material_sample = intersection.sample(wo, tray.time, filter, worker);

//		if (i < 2) {
//			material_sample.sample(samplers_[i], sample_result);
//		} else {
//			material_sample.sample(sampler, sample_result);
//		}

		material_sample.sample(sampler, sample_result);
		if (0.f == sample_result.pdf || float3::identity() == sample_result.reflection) {
			break;
		}

		if (material_sample.is_transmissive()) {
			used_absorption_coffecient = material_sample.absorption_coeffecient();
		}

		throughput *= rendering::attenuation(tray.max_t, used_absorption_coffecient)
					* (sample_result.reflection / sample_result.pdf);

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
