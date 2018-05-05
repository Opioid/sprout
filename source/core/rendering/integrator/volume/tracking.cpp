#include "tracking.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene_ray.inl"
#include "scene/prop/prop_intersection.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"

#include "base/debug/assert.hpp"
#include <iostream>
#include "base/math/print.hpp"

namespace rendering::integrator::volume {

// Code for hetereogeneous transmittance from:
// https://github.com/DaWelter/ToyTrace/blob/master/atmosphere.cxx

float3 Tracking::transmittance(Ray const& ray, rnd::Generator& rng, Worker& worker) {
	SOFT_ASSERT(!worker.interface_stack().empty());

	auto const interface = worker.interface_stack().top();

	auto const& material = *interface->material();

	float const d = ray.max_t;

	if (d - ray.min_t < 0.0005f) {
		return float3(1.f);
	}

	if (material.is_heterogeneous_volume()) {
		Transformation temp;
		auto const& transformation = interface->prop->transformation_at(ray.time, temp);

		float3 const local_origin = math::transform_point(ray.origin,
														  transformation.world_to_object);

		float3 const local_dir = math::transform_vector(ray.direction,
														transformation.world_to_object);

		float const mt  = material.majorant_mu_t();
		float const imt = 1.f / mt;

		float3 w(1.f);

		// Completely arbitray limit
		uint32_t i = max_iterations_;
		for (float t = ray.min_t; i > 0; --i) {
			float const r0 = rng.random_float();
			t = t -std::log(1.f - r0) * imt;
			if (t > d) {
				return w;
			}

			float3 const local_p = local_origin + t * local_dir;

			float3 mu_a, mu_s;
			material.collision_coefficients(local_p, Sampler_filter::Nearest,
											worker, mu_a, mu_s);

			float3 const mu_t = mu_a + mu_s;

			float3 const mu_n = float3(mt) - mu_t;

			w *= imt * mu_n;
		}

		return w;
	}

	float3 mu_a, mu_s;
	material.collision_coefficients(interface->uv, Sampler_filter::Nearest, worker, mu_a, mu_s);

	float3 const mu_t = mu_a + mu_s;

	return attenuation(d, mu_t);
}

}
