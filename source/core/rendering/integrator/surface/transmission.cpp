#include "transmission.hpp"
#include "integrator_helper.hpp"
#include "rendering/worker.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/prop/prop_intersection.inl"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"

namespace rendering {

Transmission::Transmission(const take::Settings &take_settings, math::random::Generator &rng) :
	Integrator(take_settings, rng) {}

math::float3 Transmission::resolve(Worker& worker, math::Oray& ray, scene::Intersection& intersection,
								   const math::float3& attenuation,
								   sampler::Sampler& sampler,
								   const image::texture::sampler::Sampler_2D& texture_sampler,
								   scene::material::BxDF_result& sample_result) {
	math::float3 throughput = sample_result.reflection / sample_result.pdf;
	math::float3 previous_sample_attenuation = attenuation;

	for (;;) {
		float ray_offset = take_settings_.ray_offset_modifier * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = 1000.f;

		if (!worker.intersect(intersection.prop, ray, intersection)) {
			break;
		}

		math::float3 wo = -ray.direction;
		auto material = intersection.material();
		auto& material_sample = material->sample(intersection.geo, wo, texture_sampler, worker.id());

		material_sample.sample_evaluate(sampler, sample_result);
		if (0.f == sample_result.pdf || math::float3::identity == sample_result.reflection) {
			break;
		}

		throughput *= rendering::attenuation(ray.origin, intersection.geo.p, previous_sample_attenuation);
		throughput *= sample_result.reflection / sample_result.pdf;

		// Only inner reflections are handled here (transmission path viewed from the outside)
		if (sample_result.type.test(scene::material::BxDF_type::Transmission)) {
			break;
		}

		previous_sample_attenuation = material_sample.attenuation();
	}

	return throughput;
}

}
