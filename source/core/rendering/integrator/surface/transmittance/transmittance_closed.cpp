#include "transmittance_closed.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/scene_intersection.inl"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"

namespace rendering { namespace integrator { namespace surface { namespace transmittance {

Closed::Closed(const take::Settings &take_settings, math::random::Generator &rng) :
	integrator::Integrator(take_settings, rng) {}

float3 Closed::resolve(Worker& worker, scene::Ray& ray, scene::Intersection& intersection,
					   const float3& attenuation, sampler::Sampler& sampler,
					   Sampler_filter filter, Bxdf_result& sample_result) {
	float3 throughput = sample_result.reflection / sample_result.pdf;
	float3 used_attenuation = attenuation;

	for (;;) {
		float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = take_settings_.ray_max_t;

		if (!worker.intersect(intersection.prop, ray, intersection)) {
			break;
		}

		float3 wo = -ray.direction;
		auto& material_sample = intersection.sample(worker, wo, ray.time, filter);

		material_sample.sample(sampler, sample_result);
		if (0.f == sample_result.pdf || math::float3_identity == sample_result.reflection) {
			break;
		}

		if (material_sample.is_transmissive()) {
			used_attenuation = material_sample.attenuation();
		}

		throughput *= rendering::attenuation(ray.origin, intersection.geo.p, used_attenuation);
		throughput *= sample_result.reflection / sample_result.pdf;

		// Only inner reflections are handled here
		if (sample_result.type.test(scene::material::bxdf::Type::Transmission)) {
			break;
		}
	}

	return throughput;
}

}}}}
