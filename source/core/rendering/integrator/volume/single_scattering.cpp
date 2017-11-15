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

float3 Single_scattering::transmittance(const Ray& ray, const Volume& volume,
										const Worker& worker) {
	float min_t;
	float max_t;
	if (!volume.aabb().intersect_p(ray, min_t, max_t)) {
		return float3(1.f);
	}

	const Ray tray(ray.origin, ray.direction, min_t, max_t, ray.time);

	const float3 tau = volume.optical_depth(tray, settings_.step_size, rng_,
											Sampler_filter::Nearest, worker);
	return math::exp(-tau);
}

float3 Single_scattering::li(const Ray& ray, bool primary_ray, const Volume& volume,
							 const Worker& worker, float3& transmittance) {
	float min_t;
	float max_t;
	if (!volume.aabb().intersect_p(ray, min_t, max_t)) {
		transmittance = float3(1.f);
		return float3(0.f);
	}

	const float range = max_t - min_t;

	if (range < 0.0001f) {
		transmittance = float3(1.f);
		return float3(0.f);
	}

	const uint32_t max_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));
	const uint32_t num_samples = primary_ray ? max_samples : 1;

	const float step = range / static_cast<float>(num_samples);

	const float3 w = -ray.direction;

	float3 radiance(0.f);
	float3 tr(1.f);

	float3 current = ray.point(min_t);
	float3 previous;

	const float r = std::max(rng_.random_float(), 0.0001f);

	min_t += r * step;

	const float3 next = ray.point(min_t);
	Ray tau_ray(current, next - current, 0.f, 1.f, ray.time);

	const float3 tau_ray_direction     = ray.point(min_t + step) - next;
	const float3 inv_tau_ray_direction = math::reciprocal(tau_ray_direction);

	for (uint32_t i = num_samples; i > 0; --i, min_t += step) {
		// This happens sometimes when the range is very small compared to the world coordinates.
		if (float3::identity() == tau_ray.direction) {
			tau_ray.origin = current;
			tau_ray.direction = tau_ray_direction;
			tau_ray.inv_direction = inv_tau_ray_direction;
			continue;
		}

		previous = current;
		current  = ray.point(min_t);

		const float3 tau = volume.optical_depth(tau_ray, settings_.step_size, rng_,
												Sampler_filter::Undefined, worker);
		tr *= math::exp(-tau);

		tau_ray.origin = previous;
		// This stays the same during the loop,
		// but we need a different value in the first iteration.
		// Would be nicer to restructure the loop.
		tau_ray.direction = tau_ray_direction;
		tau_ray.inv_direction = inv_tau_ray_direction;

		// Direct light scattering
		radiance += tr * estimate_direct_light(w, current, ray.time, volume, worker);
	}

	// indirect lighting
//	const float3 sp = ray.point(min_t + 0.5f * range);

//	worker.li();

	transmittance = tr;

	const float3 color = step * radiance;

	return color;
}

size_t Single_scattering::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

float3 Single_scattering::estimate_direct_light(const float3& w, const float3& p, float time,
												const Volume& volume, const Worker& worker) {
	const uint32_t num_samples = settings_.light_sampling.num_samples;

	float3 result(0.f);

	if (Light_sampling::Strategy::Single == settings_.light_sampling.strategy) {
		for (uint32_t i = num_samples; i > 0; --i) {
			const auto light = worker.scene().random_light(rng_.random_float());

			result += evaluate_light(light.ref, light.pdf, w, p, time, 0, volume, worker);
		}

		result /= static_cast<float>(num_samples);
	} else {
		const auto& lights = worker.scene().lights();

		const float light_weight = static_cast<float>(num_samples);

		for (uint32_t l = 0, len = static_cast<uint32_t>(lights.size()); l < len; ++l) {
			const auto& light = *lights[l];

			for (uint32_t i = num_samples; i > 0; --i) {
				result += evaluate_light(light, light_weight, w, p, time, l, volume, worker);
			}
		}
	}

	return result;
}

float3 Single_scattering::evaluate_light(const Light& light, float light_weight,
										 const float3& w, const float3& p,
										 float time, uint32_t sampler_dimension,
										 const Volume& volume, const Worker& worker) {
	constexpr float epsilon = 5e-5f;

	scene::light::Sample light_sample;
	if (!light.sample(p, time, sampler_, sampler_dimension, Sampler_filter::Nearest,
					  worker, light_sample)) {
		return float3::identity();
	}

	const Ray shadow_ray(p, light_sample.shape.wi, 0.f,
						 light_sample.shape.t - epsilon, time);

	const float3 tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);
	if (math::any_greater_zero(tv)) {
		const float phase = volume.phase(w, -light_sample.shape.wi);

		const float3 scattering = volume.scattering(p, Sampler_filter::Undefined, worker);

		const float3 tr = Single_scattering::transmittance(shadow_ray, volume, worker);

		const float3 l = tr * light_sample.radiance;

		return (phase * tv) * (scattering * l) / (light_weight * light_sample.shape.pdf);
	}

	return float3(0.f);
}

Single_scattering_factory::Single_scattering_factory(const take::Settings& take_settings,
													 uint32_t num_integrators,
													 float step_size,
													 Light_sampling light_sampling) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Single_scattering>(num_integrators)),
	settings_{step_size, light_sampling} {}

Single_scattering_factory::~Single_scattering_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Single_scattering_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Single_scattering(rng, take_settings_, settings_);
}

}}}
