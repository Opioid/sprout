#include "ao.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene_intersection.inl"
#include "scene/scene_ray.inl"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "take/take_settings.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

AO::AO(const take::Settings& take_settings,
	   rnd::Generator& rng,
	   const Settings& settings) :
	Integrator(take_settings, rng),
	settings_(settings),
	sampler_(rng) {}

void AO::prepare(const scene::Scene& /*scene*/,
				 uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, settings_.num_samples, 1, 1);
}

void AO::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	sampler_.resume_pixel(sample, scramble);
}

float4 AO::li(Worker& worker, scene::Ray& ray, bool /*volume*/,
			  scene::Intersection& intersection) {
	scene::Ray occlusion_ray;
	occlusion_ray.origin = intersection.geo.p;
	occlusion_ray.min_t	 = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	occlusion_ray.max_t	 = settings_.radius;
	occlusion_ray.time   = ray.time;

	float result = 0.f;

	float3 wo = -ray.direction;
	auto& material_sample = intersection.sample(worker, wo, ray.time, Sampler_filter::Unknown);

	for (uint32_t i = 0; i < settings_.num_samples; ++i) {
		float2 sample = sampler_.generate_sample_2D();
		float3 hs = math::sample_hemisphere_cosine(sample);
//		float3 ws = intersection.geo.tangent_to_world(hs);
		float3 ws = material_sample.base_layer().tangent_to_world(hs);

		occlusion_ray.set_direction(ws);

		if (worker.visibility(occlusion_ray)) {
			result += settings_.num_samples_reciprocal;
		}
	}

	return float4(result, result, result, 1.f);
}

size_t AO::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

AO_factory::AO_factory(const take::Settings& settings, uint32_t num_samples, float radius) :
	Factory(settings) {
	settings_.num_samples = num_samples;
	settings_.num_samples_reciprocal = 1.f / static_cast<float>(settings_.num_samples);
	settings_.radius = radius;
}

Integrator* AO_factory::create(rnd::Generator& rng) const {
	return new AO(take_settings_, rng, settings_);
}

}}}
