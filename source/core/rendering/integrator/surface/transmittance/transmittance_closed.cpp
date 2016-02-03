#include "transmittance_closed.hpp"
#include "../integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/prop/prop_intersection.inl"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"

namespace rendering { namespace integrator { namespace surface { namespace transmittance {

Closed::Closed(const take::Settings &take_settings, math::random::Generator &rng) :
	integrator::Integrator(take_settings, rng) {}

math::float3 Closed::resolve(Worker& worker, scene::Ray& ray, scene::Intersection& intersection,
							 const math::float3& attenuation,
							 sampler::Sampler& sampler,
							 const image::texture::sampler::Sampler_2D& texture_sampler,
							 scene::material::bxdf::Result& sample_result) {
	math::float3 throughput = sample_result.reflection / sample_result.pdf;
	math::float3 used_attenuation = attenuation;

	for (;;) {
		float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = take_settings_.ray_max_t;

		if (!worker.intersect(intersection.prop, ray, intersection)) {
			break;
		}

		math::float3 wo = -ray.direction;
		auto material = intersection.material();
		auto& material_sample = material->sample(intersection.geo, wo, ray.time, 1.f, texture_sampler, worker.id());

		material_sample.sample_evaluate(sampler, sample_result);
		if (0.f == sample_result.pdf || math::float3::identity == sample_result.reflection) {
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
