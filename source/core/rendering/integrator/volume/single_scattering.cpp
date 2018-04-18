#include "single_scattering.hpp"
#include "tracking.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light_sample.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/shape/shape.hpp"
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

/*
float3 Single_scattering::li(const Ray& ray, const Volume& volume,
							 Worker& worker, float3& transmittance) {
	if (ray.properties.test(Ray::Property::Recursive)) {
		transmittance = Single_scattering::transmittance(ray, volume, worker);
		return float3::identity();
	}

	float min_t = ray.min_t;
	const float range = ray.max_t - min_t;

	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

//	const float step_size = -std::log(settings_.step_probability) / spectrum::average(sigma);

	const uint32_t max_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));
	const uint32_t num_samples = ray.is_primary() ? max_samples : 1;

	const float step = range / static_cast<float>(num_samples);

	float3 radiance(0.f);
	float3 tr(1.f);

	const float3 start = ray.point(min_t);

	const float r = rng_.random_float();

	min_t += r * step;

	const float3 next = ray.point(min_t);
	Ray tau_ray(start, next - start, 0.f, 1.f, 0, ray.time);

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

		// Lighting
		const float3 local_radiance = estimate_direct_light(ray, current, worker);

		float3 sigma_a, scattering;
		material.extinction(transformation, current,
							Sampler_filter::Undefined, worker, sigma_a, scattering);

		radiance += tr * scattering * local_radiance;
	}

	tau_ray.set_direction(ray.point(ray.max_t) - tau_ray.origin);
	const float3 tau = material.optical_depth(transformation, volume.aabb(), tau_ray,
											  settings_.step_size, rng_,
											  Sampler_filter::Undefined, worker);

	tr *= math::exp(-tau);

	transmittance = tr;

	const float3 color = step * radiance;

	return color;
}
*/
float3 Single_scattering::transmittance(const Ray& ray, Worker& worker) {
	return Tracking::transmittance(ray, rng_, worker);
}

bool Single_scattering::integrate(Ray& /*ray*/, Intersection& /*intersection*/,
								  Sampler_filter /*filter*/, Worker& /*worker*/,
								  float3& /*li*/, float3& /*transmittance*/, float3& /*weight*/) {
	return false;
}

size_t Single_scattering::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

float3 Single_scattering::estimate_direct_light(const Ray& ray, const float3& position,
												Worker& worker) {
	float3 result = float3::identity();

	Ray shadow_ray;
	shadow_ray.origin = position;
	shadow_ray.min_t  = 0.f;
	shadow_ray.depth  = ray.depth + 1;
	shadow_ray.time   = ray.time;

	const auto light = worker.scene().random_light(rng_.random_float());

	scene::light::Sample light_sample;
	if (light.ref.sample(position, float3(0.f, 0.f, 1.f), ray.time,
						 true, sampler_, 0, Sampler_filter::Nearest, worker, light_sample)) {
		shadow_ray.set_direction(light_sample.shape.wi);
		const float offset = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
		shadow_ray.max_t = light_sample.shape.t - offset;

		const float3 tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);

		if (math::any_greater_zero(tv)) {
			const float3 tr = worker.transmittance(shadow_ray);

			const float phase = 1.f / (4.f * math::Pi);

			result += (tv * tr) * (phase * light_sample.radiance)
					/ (light.pdf * light_sample.shape.pdf);
		}
	}

	return result;
}

Single_scattering_factory::Single_scattering_factory(const take::Settings& take_settings,
													 uint32_t num_integrators, float step_size, 
													 float step_probability,
													 bool indirect_lighting) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Single_scattering>(num_integrators)),
	settings_{step_size, step_probability, !indirect_lighting} {}

Single_scattering_factory::~Single_scattering_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Single_scattering_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Single_scattering(rng, take_settings_, settings_);
}

}
