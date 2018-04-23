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

float3 Tracking::transmittance(const Ray& ray, rnd::Generator& rng, Worker& worker) {
	SOFT_ASSERT(!worker.interface_stack().empty());

	const auto interface = worker.interface_stack().top();

	const auto& material = *interface->material();

	const float d = ray.max_t - ray.min_t;

	if (material.is_heterogeneous_volume()) {
		Transformation temp;
		const auto& transformation = interface->prop->transformation_at(ray.time, temp);

		const float mt  = material.majorant_mu_t();
		const float imt = 1.f / mt;

		float3 w(1.f);

		for (float t = 0.f;;) {
			const float r0 = rng.random_float();
			t = t -std::log(1.f - r0) * imt;
			if (t > d) {
				return w;
			}

			const float3 p = ray.point(ray.min_t + t);

			float3 mu_a, mu_s;
			material.collision_coefficients(transformation, p, Sampler_filter::Nearest,
								worker, mu_a, mu_s);

			const float3 mu_t = mu_a + mu_s;

			const float3 mu_n = float3(mt) - mu_t;

			w *= imt * mu_n;
		}
	}

	float3 mu_a, mu_s;
	material.collision_coefficients(interface->uv, Sampler_filter::Nearest, worker, mu_a, mu_s);

	const float3 mu_t = mu_a + mu_s;

	return attenuation(d, mu_t);
}

}
