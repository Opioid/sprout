#include "single_scattering.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/volume/volume.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace rendering { namespace integrator { namespace volume {

Single_scattering::Single_scattering(rnd::Generator& rng, const take::Settings& take_settings,
									 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Single_scattering::prepare(const Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Single_scattering::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Single_scattering::transmittance(Worker& worker, const Ray& ray, const Volume& volume) {
	float min_t;
	float max_t;
	if (!volume.aabb().intersect_p(ray, min_t, max_t)) {
		return float3(1.f);
	}

	const Ray tray(ray.origin, ray.direction, min_t, max_t, ray.time);

	const float3 tau = volume.optical_depth(tray, settings_.step_size, rng_,
											worker, Sampler_filter::Nearest);
	return math::exp(-tau);
}

float4 Single_scattering::li(Worker& worker, const Ray& ray, bool primary_ray,
							 const Volume& volume, float3& transmittance) {
	float min_t;
	float max_t;
	if (!volume.aabb().intersect_p(ray, min_t, max_t)) {
		transmittance = float3(1.f);
		return float4(0.f);
	}

	const float range = max_t - min_t;

	if (range < 0.0001f) {
		transmittance = float3(1.f);
		return float4(0.f);
	}

	const uint32_t num_samples = primary_ray ?
			 static_cast<uint32_t>(std::ceil(range / settings_.step_size)) : 1;

	const float step = range / static_cast<float>(num_samples);

	constexpr float epsilon = 5e-5f;

	float3 w = -ray.direction;

	float3 radiance(0.f);
	float3 tr(1.f);

	float3 current = ray.point(min_t);
	float3 previous;

	min_t += rng_.random_float() * step;

	const float3 next = ray.point(min_t);
	Ray tau_ray(current, next - current, 0.f, 1.f, ray.time);

	const float3 tau_ray_direction     = ray.point(min_t + step) - next;
	const float3 inv_tau_ray_direction = math::reciprocal(tau_ray_direction);

	for (uint32_t i = 0; i < num_samples; ++i, min_t += step) {
		previous = current;
		current  = ray.point(min_t);

//		Ray tau_ray(previous, current - previous, 0.f, 1.f, ray.time);

		const float3 tau = volume.optical_depth(tau_ray, settings_.step_size, rng_,
												worker, Sampler_filter::Unknown);
		tr *= math::exp(-tau);

		tau_ray.origin = previous;
		// This stays the same during the loop,
		// but we need a different value in the first iteration.
		// Would be nicer to restructure the loop.
		tau_ray.direction = tau_ray_direction;
		tau_ray.inv_direction = inv_tau_ray_direction;

		// Direct light scattering
		float light_pdf;
		const auto light = worker.scene().random_light(rng_.random_float(), light_pdf);

		scene::light::Sample light_sample;
		light->sample(current, ray.time, sampler_, 0, worker,
					  Sampler_filter::Nearest, light_sample);

		if (light_sample.shape.pdf > 0.f) {
			const Ray shadow_ray(current, light_sample.shape.wi, 0.f,
								 light_sample.shape.t - epsilon, ray.time);

			const float3 tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);
			if (math::any_greater_zero(tv)) {
				const float p = volume.phase(w, -light_sample.shape.wi);

				const float3 scattering = volume.scattering(current, worker,
															Sampler_filter::Unknown);

				const float3 l = Single_scattering::transmittance(worker, shadow_ray, volume)
							   * light_sample.radiance;

				radiance += (p * tv * tr) * (scattering * l) / (light_pdf * light_sample.shape.pdf);
			}
		}
	}

	transmittance = tr;

	const float3 color = step * radiance;

	return float4(color, spectrum::luminance(color));
}

size_t Single_scattering::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

Single_scattering_factory::Single_scattering_factory(const take::Settings& take_settings,
													 uint32_t num_integrators,
													 float step_size) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Single_scattering>(num_integrators)),
	settings_{ step_size } {}

Single_scattering_factory::~Single_scattering_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Single_scattering_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Single_scattering(rng, take_settings_, settings_);
}

}}}
