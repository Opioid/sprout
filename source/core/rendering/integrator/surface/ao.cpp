#include "ao.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene_ray.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "take/take_settings.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Ao::Ao(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings) :
	Integrator(take_settings, rng), settings_(settings), sampler_(rng, settings.num_samples) {}

void Ao::start_new_pixel(uint32_t num_samples) {
	sampler_.restart_and_seed(num_samples);
}

math::float4 Ao::li(Worker& worker, scene::Ray& ray, bool /*volume*/, scene::Intersection& intersection) {
	scene::Ray occlusion_ray;
	occlusion_ray.origin = intersection.geo.p;
	occlusion_ray.min_t	 = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	occlusion_ray.max_t	 = settings_.radius;
	occlusion_ray.time   = ray.time;

	float result = 0.f;

	math::float3 wo = -ray.direction;
	auto& material_sample = intersection.sample(worker, wo, ray.time, scene::material::Sampler_settings::Filter::Unknown);

	for (uint32_t i = 0; i < settings_.num_samples; ++i) {
		math::float2 sample = sampler_.generate_sample_2D();
		math::float3 hs = math::sample_hemisphere_cosine(sample);
//		math::float3 ws = intersection.geo.tangent_to_world(hs);
		math::float3 ws = material_sample.tangent_to_world(hs);

		occlusion_ray.set_direction(ws);

		if (worker.visibility(occlusion_ray)) {
			result += settings_.num_samples_reciprocal;
		}
	}

	return math::float4(result, result, result, 1.f);
}

Ao_factory::Ao_factory(const take::Settings& settings, uint32_t num_samples, float radius) :
	Integrator_factory(settings) {
	settings_.num_samples = num_samples;
	settings_.num_samples_reciprocal = 1.f / static_cast<float>(settings_.num_samples);
	settings_.radius = radius;
}

Integrator* Ao_factory::create(math::random::Generator& rng) const {
	return new Ao(take_settings_, rng, settings_);
}

}}}
