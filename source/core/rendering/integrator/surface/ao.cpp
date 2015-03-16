#include "ao.hpp"
#include "rendering/worker.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Ao::Ao(uint32_t id,  const math::random::Generator& rng, const Settings& settings) : Surface_integrator(id, rng), settings_(settings) {}

math::float3 Ao::li(const Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) {
	math::Oray occlusion_ray;
	occlusion_ray.origin = intersection.geo.p;
	occlusion_ray.min_t = intersection.geo.epsilon;
	occlusion_ray.max_t = settings_.radius;

	float result = 0.f;

	for (uint32_t i = 0; i < settings_.num_samples; ++i) {
		math::float2 sample(rng_.random_float(), rng_.random_float());
		math::float3 hs = math::sample_hemisphere_cosine(sample);
		math::float3 ws = intersection.geo.tangent_to_world(hs);

		occlusion_ray.set_direction(ws);

		if (worker.visibility(occlusion_ray)) {
			result += settings_.num_samples_reciprocal;
		}
	}

	return math::float3(result, result, result);
}

Ao_factory::Ao_factory(uint32_t num_samples, float radius) {
	settings_.num_samples = num_samples;
	settings_.num_samples_reciprocal = 1.f / static_cast<float>(settings_.num_samples);
	settings_.radius = radius;
}

Surface_integrator* Ao_factory::create(uint32_t id, const math::random::Generator& rng) const {
	return new Ao(id, rng, settings_);
}

}
