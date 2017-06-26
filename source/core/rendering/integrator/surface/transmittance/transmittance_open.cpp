#include "transmittance_open.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/scene_intersection.inl"
#include "base/math/vector4.inl"

namespace rendering { namespace integrator { namespace surface { namespace transmittance {

Open::Open(rnd::Generator& rng, const take::Settings &take_settings, uint32_t max_bounces) :
	integrator::Integrator(rng, take_settings),
	max_bounces_(max_bounces) {}

void Open::prepare(const scene::Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Open::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Open::resolve(Worker& worker, const scene::Ray& ray, scene::Intersection& intersection,
					 const float3& attenuation, sampler::Sampler& sampler,
					 Sampler_filter filter, Bxdf_result& sample_result) const {
	float3 throughput = sample_result.reflection / sample_result.pdf;
	float3 used_attenuation = attenuation;

	Ray tray;
	tray.time = ray.time;
	tray.depth = ray.depth;

	for (uint32_t i = max_bounces_; i > 0;) {
		const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		tray.origin = intersection.geo.p;
		tray.set_direction(sample_result.wi);
		tray.min_t = ray_offset;
		tray.max_t = scene::Ray_max_t;

		if (!worker.intersect(tray, intersection)) {
			break;
		}

		const float3 wo = -tray.direction;
		auto& material_sample = intersection.sample(worker, wo, tray.time, filter);

		material_sample.sample(sampler, sample_result);
		if (0.f == sample_result.pdf || float3::identity() == sample_result.reflection) {
			break;
		}

		if (material_sample.is_transmissive()) {
			used_attenuation = material_sample.attenuation();
		} else {
			++tray.depth;
			--i;
		}

		throughput *= rendering::attenuation(tray.origin, intersection.geo.p, used_attenuation)
					* sample_result.reflection / sample_result.pdf;

		// Only inner reflections are handled here
		if (sample_result.type.test(scene::material::bxdf::Type::Transmission)) {
			break;
		}
	}

	return throughput;
}

size_t Open::num_bytes() const {
	return sizeof(*this);
}

}}}}
