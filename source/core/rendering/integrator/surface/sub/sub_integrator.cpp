#include "sub_integrator.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/bssrdf.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "base/math/sampling/sampling.hpp"
#include "base/random/generator.inl"

#include "base/debug/assert.hpp"

namespace rendering::integrator::surface::sub {

Integrator::Integrator(rnd::Generator& rng, const take::Settings& settings) :
	integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

float3 Integrator::estimate_light(const float3& position, const Prop* prop,
								  const scene::material::BSSRDF& bssrdf,
								  float time, uint32_t depth,
								  sampler::Sampler& sampler, Worker& worker) {
	const float3 scattering = bssrdf.scattering();

	float3 radiance = estimate_direct_light(position, prop, bssrdf, time, depth, sampler, worker);

	SOFT_ASSERT(math::all_finite_and_positive(radiance));

	radiance += estimate_indirect_light(position, prop, bssrdf, time, depth, sampler, worker);

	SOFT_ASSERT(math::all_finite_and_positive(radiance));

	return scattering * radiance;
}

float3 Integrator::estimate_direct_light(const float3& position, const Prop* prop,
										 const scene::material::BSSRDF& bssrdf,
										 float time, uint32_t depth,
										 sampler::Sampler& sampler, Worker& worker) {
	// Direct light scattering
	const auto light = worker.scene().random_light(rng_.random_float());

	scene::light::Sample light_sample;
	if (!light.ref.sample(position, time, sampler, 0, Sampler_filter::Nearest,
						  worker, light_sample)) {
		return float3(0.f);
	}

	Ray shadow_ray(position, light_sample.shape.wi, 0.f, light_sample.shape.t, time, ++depth);

	float prop_length = 0.f;

	float epsilon;
	if (worker.intersect(prop, shadow_ray, epsilon)) {
		prop_length = shadow_ray.max_t;//shadow_ray.length();

		const float ray_offset = take_settings_.ray_offset_factor * epsilon;
		shadow_ray.min_t = shadow_ray.max_t + ray_offset;
		shadow_ray.max_t = light_sample.shape.t - ray_offset;
	}

	const float3 tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);
	if (math::any_greater_zero(tv)) {
		// this is the transmittance from "outside",
		// potentially caused by participating media volumes in the scene
		const float3 tr = worker.transmittance(shadow_ray);

		//	float p = volume.phase(w, -light_sample.shape.wi);
		constexpr float phase = 1.f / (4.f * math::Pi);

		const float3 tau = bssrdf.optical_depth(prop_length);
		const float3 transmittance = math::exp(-tau);
		const float3 l = transmittance * light_sample.radiance;

		return ((phase * tv) * (tr * l)) / (light.pdf * light_sample.shape.pdf);
	}

	return float3(0.f);
}

float3 Integrator::estimate_indirect_light(const float3& position, const Prop* prop,
										   const scene::material::BSSRDF& bssrdf,
										   float time, uint32_t depth,
										   sampler::Sampler& /*sampler*/, Worker& worker) {
	const float2 uv(rng_.random_float(), rng_.random_float());
	const float3 dir = math::sample_sphere_uniform(uv);

	// const float phase = volume.phase(w, -dir);
//	constexpr float phase = 1.f / (4.f * math::Pi);

	Ray secondary_ray(position, dir, 0.f, scene::Ray_max_t, time,
					  depth + 1, Ray::Property::Recursive);

	float prop_length = 0.f;
	float epsilon;
	if (worker.intersect(prop, secondary_ray, epsilon)) {
		prop_length = secondary_ray.max_t;

		const float ray_offset = take_settings_.ray_offset_factor * epsilon;
		secondary_ray.min_t = secondary_ray.max_t + ray_offset;
		secondary_ray.max_t = scene::Ray_max_t;
	}

	const float3 tau = bssrdf.optical_depth(prop_length);
	const float3 transmittance = math::exp(-tau);

	SOFT_ASSERT(math::all_finite_and_positive(transmittance));

	const float3 li = worker.li(secondary_ray).xyz();

	SOFT_ASSERT(math::all_finite_and_positive(li));

	return /*phase **/ (transmittance * li);
}

Factory::Factory(const take::Settings& settings) :
	take_settings_(settings) {}

Factory::~Factory() {}

}
