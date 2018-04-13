#include "tracking.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene_ray.inl"
#include "scene/prop/prop_intersection.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"

#include "base/debug/assert.hpp"

namespace rendering::integrator::volume {

static inline void avg_history_probabilities(float mt,
											 const float3& sigma_s,
											 const float3& sigma_n,
											 const float3& w,
											 float& pn,
											 float3& wn) {
	const float ms = math::average(sigma_s * w);
	const float mn = math::average(sigma_n * w);
	const float c = 1.f / (ms + mn);

	pn = mn * c;

	wn = (sigma_n / (mt * pn));
}

float3 Tracking::transmittance(const Ray& ray, const Intersection& intersection,
							   rnd::Generator& rng, const Worker& worker) {
	const float d = ray.max_t - ray.min_t;

	const auto& material = *intersection.material();

	if (material.is_heterogeneous_volume()) {
		Transformation temp;
		const auto& transformation = intersection.prop->transformation_at(ray.time, temp);

		const float mt = material.max_extinction();

		float3 w(1.f);

		for (float t = 0.f;;) {
			const float r0 = rng.random_float();
			t = t -std::log(1.f - r0) / mt;
			if (t > d) {
				return w;
			}

			const float3 p = ray.point(ray.min_t + t);

			float3 sigma_a, sigma_s;
			material.extinction(transformation, p, Sampler_filter::Nearest,
								worker, sigma_a, sigma_s);

			const float3 sigma_t = sigma_a + sigma_s;

			const float3 sigma_n = float3(mt) - sigma_t;

			float pn;
			float3 wn;
			avg_history_probabilities(mt, sigma_s, sigma_n, w, pn, wn);

			const float r1 = rng.random_float();
			if (r1 <= 1.f - pn) {
				return float3(0.f);
			} else {
				SOFT_ASSERT(math::all_finite(wn));
				w *= wn;
			}
		}
	}

	float3 sigma_a, sigma_s;
	material.extinction(intersection.geo.uv, Sampler_filter::Nearest,
						worker, sigma_a, sigma_s);

	const float3 extinction = sigma_a + sigma_s;

	return attenuation(d, extinction);
}

}
