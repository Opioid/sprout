#include "sub_integrator.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/bssrdf.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace surface { namespace sub {

Integrator::Integrator(rnd::Generator& rng, const take::Settings& settings) :
	integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

float3 Integrator::estimate_direct_light(const float3& position, const scene::Prop* prop,
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

	Intersection intersection;
	if (!worker.intersect(prop, shadow_ray, intersection)) {
		return float3(0.f);
	}

	// Travel distance inside prop
	const float prop_length = shadow_ray.max_t;//shadow_ray.length();

	const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	shadow_ray.min_t = shadow_ray.max_t + ray_offset;
	shadow_ray.max_t = light_sample.shape.t - ray_offset;

	const float3 tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);
	if (math::any_greater_zero(tv)) {
		// this is the transmittance from "outside",
		// potentially caused by participating media volumes in the scene
		const float3 t = worker.transmittance(shadow_ray);

		//	float p = volume.phase(w, -light_sample.shape.wi);
		constexpr float p = 1.f / (4.f * math::Pi);

		const float3 scattering = bssrdf.scattering();

		const float3 tau = bssrdf.optical_depth(prop_length);
		const float3 transmittance = math::exp(-tau);
		const float3 l = transmittance * light_sample.radiance;

		return ((p * tv * t) * (scattering * l)) / (light.pdf * light_sample.shape.pdf);
	}

	return float3(0.f);
}

Factory::Factory(const take::Settings& settings) :
	take_settings_(settings) {}

Factory::~Factory() {}

}}}}

