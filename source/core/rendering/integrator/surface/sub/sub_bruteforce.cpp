#include "sub_bruteforce.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_intersection.inl"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bssrdf.hpp"
#include "base/math/vector4.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/random/generator.inl"

#include <iostream>

namespace rendering { namespace integrator { namespace surface { namespace sub {

Bruteforce::Bruteforce(rnd::Generator& rng, const take::Settings& take_settings,
					   const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Bruteforce::prepare(const Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Bruteforce::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Bruteforce::li(Worker& worker, const Ray& ray, const Intersection& intersection) {
	float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	Ray tray(intersection.geo.p, ray.direction, ray_offset, scene::Ray_max_t);
	Intersection tintersection;
	if (!worker.intersect(intersection.prop, tray, tintersection)) {
		return float3(0.f);
	}

	const float range = tray.max_t - tray.min_t;

	if (range < 0.0001f) {
		return float3(0.f);
	}

	const auto& bssrdf = intersection.bssrdf(worker);

	const uint32_t num_samples = 16;//static_cast<uint32_t>(std::ceil(range / settings_.step_size));

	const float step = range / static_cast<float>(num_samples);

//	float3 w = -ray.direction;

	float3 radiance(0.f);
	float3 tr(1.f);

	float min_t = tray.min_t;
	float pdf = rng_.random_float();
	float tau_ray_length = pdf * step;

	min_t += tau_ray_length;

	float3 current = tray.point(min_t);

	uint32_t i = 0;
	for (i = 0; i < num_samples; ++i, min_t += step) {
		const float3 tau = bssrdf.optical_depth(tau_ray_length);
		tr *= math::exp(-tau);

		// Direct light scattering
		radiance += tr * estimate_direct_light(current, intersection.prop, bssrdf,
											   ray.time, worker);

		tray.origin = current;
		const float2 uv(rng_.random_float(), rng_.random_float());
		tray.set_direction(math::sample_sphere_uniform(uv));
		tray.min_t = ray_offset;
		pdf = rng_.random_float();
		tau_ray_length = pdf * step;
		tray.max_t = tau_ray_length;
		if (!worker.intersect(intersection.prop, tray, tintersection)) {
			current = tray.point(tray.max_t);
		} else {
			break;
		}
	}

	float3 color = /*step **/ (1.f / static_cast<float>(i + 1)) * radiance;
	return color;
}

float3 Bruteforce::li(Worker& worker, Ray& ray, Intersection& intersection,
					  sampler::Sampler& sampler, Sampler_filter filter,
					  Bxdf_result& sample_result) {
	return float3(0.f);
}

size_t Bruteforce::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

float3 Bruteforce::estimate_direct_light(const float3& position, const scene::Prop* prop,
										 const scene::material::BSSRDF& bssrdf, float time,
										 Worker& worker) {
	// Direct light scattering
	float light_pdf;
	const auto light = worker.scene().random_light(rng_.random_float(), light_pdf);
	if (!light) {
		return float3(0.f);
	}

	scene::light::Sample light_sample;
	light->sample(time, position, sampler_, 0, worker, Sampler_filter::Nearest, light_sample);

	if (light_sample.shape.pdf > 0.f) {
		Ray shadow_ray(position, light_sample.shape.wi, 0.f, light_sample.shape.t, time);

		Intersection intersection;
		if (!worker.intersect(prop, shadow_ray, intersection)) {
			return float3(0.f);
		}

		const float prop_length = shadow_ray.length();

		const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		shadow_ray.min_t = shadow_ray.max_t + ray_offset;
		shadow_ray.max_t = light_sample.shape.t - ray_offset;

		const float mv = worker.masked_visibility(shadow_ray, Sampler_filter::Nearest);
		if (mv > 0.f) {
			//	float p = volume.phase(w, -light_sample.shape.wi);
			const float p = 1.f / (4.f * math::Pi);

			const float3 scattering = bssrdf.scattering();

			const float3 tau = bssrdf.optical_depth(prop_length);
			const float3 transmittance = math::exp(-tau);
			const float3 l = transmittance * light_sample.radiance;

			return p * mv * scattering * l / (light_pdf * light_sample.shape.pdf);
		}
	}

	return float3(0.f);
}

Bruteforce_factory::Bruteforce_factory(const take::Settings& take_settings,
									   uint32_t num_integrators, float step_size) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Bruteforce>(num_integrators)) {
	settings_.step_size = step_size;
}

Bruteforce_factory::~Bruteforce_factory() {
	memory::destroy_aligned(integrators_, num_integrators_);
}

Integrator* Bruteforce_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Bruteforce(rng, take_settings_, settings_);
}


}}}}
