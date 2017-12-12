#include "single_scattering.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/shape/shape.hpp"
#include "scene/volume/volume.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

namespace rendering::integrator::volume {

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
	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const auto& material = *volume.material(0);

	const float3 tau = material.optical_depth(transformation, volume.aabb(), ray,
											  settings_.step_size, rng_,
											  Sampler_filter::Nearest, worker);
	return math::exp(-tau);
}

float3 Single_scattering::li(const Ray& ray, bool primary_ray, const Volume& volume,
							 Worker& worker, float3& transmittance) {
	if (ray.properties.test(Ray::Property::Within_volume)) {
		transmittance = Single_scattering::transmittance(ray, volume, worker);
		return float3::identity();
	}

	float min_t = ray.min_t;
	const float range = ray.max_t - min_t;

	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const uint32_t max_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));
	const uint32_t num_samples = primary_ray ? max_samples : 1;

	const float step = range / static_cast<float>(num_samples);

	const float3 w = -ray.direction;

	float3 radiance(0.f);
	float3 tr(1.f);

	const float3 start = ray.point(min_t);

	const float r = std::max(rng_.random_float(), 0.0001f);

	min_t += r * step;

	const float3 next = ray.point(min_t);
	Ray tau_ray(start, next - start, 0.f, 1.f, ray.time);

	const float3 tau_ray_direction     = ray.point(min_t + step) - next;
	const float3 inv_tau_ray_direction = math::reciprocal(tau_ray_direction);

	const auto& material = *volume.material(0);

	for (uint32_t i = num_samples; i > 0; --i, min_t += step) {
		// This happens sometimes when the range is very small compared to the world coordinates.
		if (float3::identity() == tau_ray.direction) {
			tau_ray.origin = ray.point(min_t + step);
			tau_ray.direction = tau_ray_direction;
			tau_ray.inv_direction = inv_tau_ray_direction;
			continue;
		}

		const float3 tau = material.optical_depth(transformation, volume.aabb(), tau_ray,
												  settings_.step_size, rng_,
												  Sampler_filter::Undefined, worker);

		tr *= math::exp(-tau);

		const float3 current = ray.point(min_t);
		tau_ray.origin = current;
		// This stays the same during the loop,
		// but we need a different value in the first iteration.
		// Would be nicer to restructure the loop.
		tau_ray.direction = tau_ray_direction;
		tau_ray.inv_direction = inv_tau_ray_direction;

		// Direct incoming light
/*		float3 local_radiance = estimate_direct_light(w, current, ray.time, material, worker);

	//	if (ray.depth < settings_.max_indirect_bounces) {
			// Indirect incoming light
			local_radiance += estimate_indirect_light(w, current, ray, material, worker);
	//	}

		const float3 scattering = material.scattering(transformation, current,
													  Sampler_filter::Undefined, worker);

		radiance += tr * scattering * local_radiance;
		*/

//		if (ray.depth < settings_.max_indirect_bounces) {
			Ray secondary_ray = ray;
			secondary_ray.properties.set(Ray::Property::Within_volume);

//			if (ray.depth > 1) {
//				secondary_ray.properties.set(Ray::Property::Direct_only);
//			}

			scene::prop::Intersection secondary_intersection;
			secondary_intersection.geo.p = current;
			secondary_intersection.geo.part = 0;
			secondary_intersection.geo.epsilon = 0.0005f;
			secondary_intersection.prop = &volume;

			const float3 local_radiance = worker.li(secondary_ray, secondary_intersection).xyz();

			const float3 scattering = material.scattering(transformation, current,
														  Sampler_filter::Undefined, worker);

			radiance += tr * scattering * local_radiance;
//		}

	}

	transmittance = tr;

	const float3 color = step * radiance;

	return color;
}

size_t Single_scattering::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

float3 Single_scattering::estimate_direct_light(const float3& w, const float3& p, float time,
												const Material& material, Worker& worker) {
	const uint32_t num_samples = settings_.light_sampling.num_samples;

	float3 result(0.f);

	if (Light_sampling::Strategy::Single == settings_.light_sampling.strategy) {
		for (uint32_t i = num_samples; i > 0; --i) {
			const auto light = worker.scene().random_light(rng_.random_float());

			result += evaluate_light(light.ref, light.pdf, w, p, time, 0, material, worker);
		}

		result /= static_cast<float>(num_samples);
	} else {
		const auto& lights = worker.scene().lights();

		const float light_weight = static_cast<float>(num_samples);

		for (uint32_t l = 0, len = static_cast<uint32_t>(lights.size()); l < len; ++l) {
			const auto& light = *lights[l];

			for (uint32_t i = num_samples; i > 0; --i) {
				result += evaluate_light(light, light_weight, w, p, time, l, material, worker);
			}
		}
	}

	return result;
}

float3 Single_scattering::evaluate_light(const Light& light, float light_weight,
										 const float3& w, const float3& p,
										 float time, uint32_t sampler_dimension,
										 const Material& material, Worker& worker) {
	constexpr float epsilon = 5e-5f;

	scene::light::Sample light_sample;
	if (!light.sample(p, time, sampler_, sampler_dimension, Sampler_filter::Nearest,
					  worker, light_sample)) {
		return float3::identity();
	}

	const Ray shadow_ray(p, light_sample.shape.wi, 0.f, light_sample.shape.t - epsilon, time);

	const float3 tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);
	if (math::any_greater_zero(tv)) {
		const float phase = material.phase(w, -light_sample.shape.wi);

		const float3 tr = worker.transmittance(shadow_ray);

		const float3 l = tr * light_sample.radiance;

		return (phase * tv * l) / (light_weight * light_sample.shape.pdf);
	}

	return float3(0.f);
}

float3 Single_scattering::estimate_indirect_light(const float3& w, const float3& p,
												  const Ray& history, const Material& material,
												  Worker& worker) {
	const float2 uv(rng_.random_float(), rng_.random_float());
	const float3 dir = math::sample_sphere_uniform(uv);

//	const float phase = material.phase(w, -dir);

	Ray secondary_ray(p, dir, 0.f, scene::Ray_max_t, history.time,
					  history.depth + 1, Ray::Property::Within_volume);

	const float3 li = worker.li(secondary_ray).xyz();

	return /*phase **/ li;
}

Single_scattering_factory::Single_scattering_factory(const take::Settings& take_settings,
													 uint32_t num_integrators, float step_size,
													 uint32_t max_indirect_bounces,
													 Light_sampling light_sampling) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Single_scattering>(num_integrators)),
	settings_{step_size, max_indirect_bounces, light_sampling} {}

Single_scattering_factory::~Single_scattering_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Single_scattering_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Single_scattering(rng, take_settings_, settings_);
}

}
