#include "tracking.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/volumetric/volumetric_octree.hpp"
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

		float3 const local_origin = transformation.world_to_object_point(ray.origin);
		float3 const local_dir    = transformation.world_to_object_vector(ray.direction);

		if (auto const tree = material.volume_octree(); tree) {
			Ray local_ray(local_origin, local_dir, ray.min_t, ray.max_t);

			float3 w(1.f);
			for (;local_ray.min_t < d;) {
				float mt;
				if (!tree->intersect(local_ray, mt)) {
					return w;
				}

				w *= track(local_ray, mt, material, Sampler_filter::Nearest, rng, worker);

				local_ray.min_t = local_ray.max_t + 0.00001f;
				local_ray.max_t = d;
			}

			return w;
		}

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

bool Tracking::track(Ray const& ray, float mt, Material const& material, Sampler_filter filter,
					 rnd::Generator& rng, Worker& worker, float& t_out, float3& w) {
	if (0.f == mt) {
		return false;
	}

	float3 lw = w;

	float const d = ray.max_t;

	for (float t = ray.min_t;;) {
		float const r0 = rng.random_float();
		t = t -std::log(1.f - r0) / mt;
		if (t > d) {
			w = lw;
			return false;
		}

		float3 const local_p = ray.point(t);

		float3 mu_a, mu_s;
		material.collision_coefficients(local_p, filter, worker, mu_a, mu_s);

		float3 const mu_t = mu_a + mu_s;

		float3 const mu_n = float3(mt) - mu_t;

		float const ms = math::average(mu_s * lw);
		float const mn = math::average(mu_n * lw);
		float const c = 1.f / (ms + mn);

		float const ps = ms * c;
		float const pn = mn * c;

		float const r1 = rng.random_float();
		if (r1 <= 1.f - pn && ps > 0.f) {
			float3 const ws = mu_s / (mt * ps);

			t_out = t;
			w = lw * ws;
			return true;
		} else {
			float3 const wn = mu_n / (mt * pn);

			SOFT_ASSERT(math::all_finite(wn));

			lw *= wn;
		}
	}
}

float3 Tracking::track(Ray const& ray, float mt, Material const& material, Sampler_filter filter,
					   rnd::Generator& rng, Worker& worker) {
	float3 w(1.f);

	if (0.f == mt) {
		return w;
	}

	float const imt = 1.f / mt;

	float const d = ray.max_t;

	for (float t = ray.min_t;;) {
		float const r0 = rng.random_float();
		t = t -std::log(1.f - r0) * imt;
		if (t > d) {
			return w;
		}

		float3 const local_p = ray.point(t);

		float3 mu_a, mu_s;
		material.collision_coefficients(local_p, filter, worker, mu_a, mu_s);

		float3 const mu_t = mu_a + mu_s;

		float3 const mu_n = float3(mt) - mu_t;

		w *= imt * mu_n;
	}

	return w;
}

}
