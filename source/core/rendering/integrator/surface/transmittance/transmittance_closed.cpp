#include "transmittance_closed.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/prop/prop_intersection.inl"
#include "base/math/vector4.inl"

namespace rendering::integrator::surface::transmittance {

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

void Closed::resolve(const Ray& ray, Intersection& intersection,
					 const float3& absorption_coefficient, sampler::Sampler& sampler,
					 Sampler_filter filter, Worker& worker, Bxdf_sample& sample_result) {
	float3 throughput = sample_result.reflection / sample_result.pdf;
	float3 used_absorption_coefficient = absorption_coefficient;

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

		material_sample.sample(sampler, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		throughput *= rendering::attenuation(tray.max_t, used_absorption_coefficient)
					* (sample_result.reflection / sample_result.pdf);

		// Only inner reflections are handled here
		if (sample_result.type.test(Bxdf_type::Transmission)) {
			break;
		}
	}

	sample_result.reflection = throughput;
}

size_t Closed::num_bytes() const {
	return sizeof(*this);
}

}
