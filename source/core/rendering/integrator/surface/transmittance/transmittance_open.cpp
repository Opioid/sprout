#include "transmittance_open.hpp"
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

Open::Open(const take::Settings &take_settings, math::random::Generator &rng) :
	integrator::Integrator(take_settings, rng) {}

math::float3 Open::resolve(Worker& worker, scene::Ray& ray, scene::Intersection& intersection,
						   const math::float3& attenuation,
						   sampler::Sampler& sampler,
						   const image::texture::sampler::Sampler_2D& texture_sampler,
						   scene::material::bxdf::Result& sample_result) {
	math::float3 throughput = sample_result.reflection / sample_result.pdf;
	math::float3 previous_sample_attenuation = attenuation;

	auto original_material = intersection.material();

	float ior = ray.ior;

	for (;;) {
		float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = take_settings_.ray_max_t;

		if (!worker.intersect(ray, intersection)) {
			break;
		}

		math::float3 wo = -ray.direction;

		auto intersection_material = intersection.material();
		if (original_material != intersection_material) {
			intersection.geo.revert_direction();
			ior = intersection.material()->ior();
		}

	//	auto material = intersection.material();
		auto& material_sample = original_material->sample(intersection.geo, wo, ior, texture_sampler, worker.id());

		material_sample.sample_evaluate(sampler, sample_result);
		if (0.f == sample_result.pdf || math::float3::identity == sample_result.reflection) {
			break;
		}

		throughput *= rendering::attenuation(ray.origin, intersection.geo.p, previous_sample_attenuation);
		throughput *= sample_result.reflection / sample_result.pdf;

		// Only inner reflections are handled here
		if (sample_result.type.test(scene::material::bxdf::Type::Transmission)) {

			if (original_material != intersection_material) {
				ray.ior = intersection_material->ior();

				intersection.geo.revert_direction();
				auto& material_sample = intersection_material->sample(intersection.geo, wo, ior, texture_sampler, worker.id());

				material_sample.sample_evaluate(sampler, sample_result);
				if (0.f == sample_result.pdf || math::float3::identity == sample_result.reflection) {
					break;
				}

				throughput *= sample_result.reflection / sample_result.pdf;
			}

			break;
		}

		previous_sample_attenuation = material_sample.attenuation();
	}

	return throughput;
}

}}}}
