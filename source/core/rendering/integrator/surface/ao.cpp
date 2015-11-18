#include "ao.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "take/take_settings.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Ao::Ao(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings) :
	Surface_integrator(take_settings, rng), settings_(settings), sampler_(rng, settings.num_samples) {}

void Ao::start_new_pixel(uint32_t num_samples) {
	sampler_.restart(num_samples);
}

math::float4 Ao::li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) {
	math::Oray occlusion_ray;
	occlusion_ray.time = ray.time;
	occlusion_ray.origin = intersection.geo.p;
	occlusion_ray.min_t = take_settings_.ray_offset_modifier * intersection.geo.epsilon;
	occlusion_ray.max_t = settings_.radius;

	float result = 0.f;

	auto material = intersection.material();

	math::float3 wo = -ray.direction;
	auto& material_sample = material->sample(intersection.geo, wo, settings_.sampler_linear, worker.id());

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
	Surface_integrator_factory(settings) {
	settings_.num_samples = num_samples;
	settings_.num_samples_reciprocal = 1.f / static_cast<float>(settings_.num_samples);
	settings_.radius = radius;
}

Surface_integrator* Ao_factory::create(math::random::Generator& rng) const {
	return new Ao(take_settings_, rng, settings_);
}

}
