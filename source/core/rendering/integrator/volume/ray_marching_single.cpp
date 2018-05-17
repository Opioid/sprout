#include "ray_marching_single.hpp"
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

Ray_marching_single::Ray_marching_single(rnd::Generator& rng, take::Settings const& take_settings,
										 Settings const& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Ray_marching_single::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Ray_marching_single::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

/*
float3 Ray_marching_single::li(Ray const& ray, const Volume& volume,
							 Worker& worker, float3& transmittance) {
	if (ray.properties.test(Ray::Property::Recursive)) {
		transmittance = Ray_marching_single::transmittance(ray, volume, worker);
		return float3::identity();
	}

	float min_t = ray.min_t;
	float const range = ray.max_t - min_t;

	Transformation temp;
	auto const& transformation = volume.transformation_at(ray.time, temp);

//	float const step_size = -std::log(settings_.step_probability) / spectrum::average(sigma);

	uint32_t const max_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));
	uint32_t const num_samples = ray.is_primary() ? max_samples : 1;

	float const step = range / static_cast<float>(num_samples);

	float3 radiance(0.f);
	float3 tr(1.f);

	float3 const start = ray.point(min_t);

	float const r = rng_.random_float();

	min_t += r * step;

	float3 const next = ray.point(min_t);
	Ray tau_ray(start, next - start, 0.f, 1.f, 0, ray.time);

	float3 const tau_ray_direction     = ray.point(min_t + step) - next;
	float3 const inv_tau_ray_direction = math::reciprocal(tau_ray_direction);

	auto const& material = *volume.material(0);

	for (uint32_t i = num_samples; i > 0; --i, min_t += step) {
		// This happens sometimes when the range is very small compared to the world coordinates.
		if (float3::identity() == tau_ray.direction) {
			tau_ray.origin = ray.point(min_t + step);
			tau_ray.direction = tau_ray_direction;
			tau_ray.inv_direction = inv_tau_ray_direction;
			continue;
		}

		float3 const tau = material.optical_depth(transformation, volume.aabb(), tau_ray,
												  settings_.step_size, rng_,
												  Sampler_filter::Undefined, worker);

		tr *= math::exp(-tau);

		float3 const current = ray.point(min_t);
		tau_ray.origin = current;
		// This stays the same during the loop,
		// but we need a different value in the first iteration.
		// Would be nicer to restructure the loop.
		tau_ray.direction = tau_ray_direction;
		tau_ray.inv_direction = inv_tau_ray_direction;

		// Lighting
		float3 const local_radiance = estimate_direct_light(ray, current, worker);

		float3 mu_a, scattering;
		material.collision_coefficients(transformation, current,
							Sampler_filter::Undefined, worker, mu_a, scattering);

		radiance += tr * scattering * local_radiance;
	}

	tau_ray.set_direction(ray.point(ray.max_t) - tau_ray.origin);
	float3 const tau = material.optical_depth(transformation, volume.aabb(), tau_ray,
											  settings_.step_size, rng_,
											  Sampler_filter::Undefined, worker);

	tr *= math::exp(-tau);

	transmittance = tr;

	float3 const color = step * radiance;

	return color;
}
*/
float3 Ray_marching_single::transmittance(Ray const& ray, Worker& worker) {
	return Tracking::transmittance(ray, rng_, worker);
}

bool Ray_marching_single::integrate(Ray& /*ray*/, Intersection& /*intersection*/,
									Sampler_filter /*filter*/, Worker& /*worker*/,
									float3& /*li*/, float3& /*transmittance*/) {
	return false;
}

size_t Ray_marching_single::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

float3 Ray_marching_single::estimate_direct_light(Ray const& ray, f_float3 position,
												  Worker& worker) {
	float3 result = float3::identity();

	Ray shadow_ray;
	shadow_ray.origin = position;
	shadow_ray.min_t  = 0.f;
	shadow_ray.depth  = ray.depth + 1;
	shadow_ray.time   = ray.time;

	auto const light = worker.scene().random_light(rng_.random_float());

	scene::light::Sample light_sample;
	if (light.ref.sample(position, float3(0.f, 0.f, 1.f), ray.time,
						 true, sampler_, 0, Sampler_filter::Nearest, worker, light_sample)) {
		shadow_ray.set_direction(light_sample.shape.wi);
		float const offset = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
		shadow_ray.max_t = light_sample.shape.t - offset;

		float3 const tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);

		if (math::any_greater_zero(tv)) {
			float3 const tr = worker.transmittance(shadow_ray);

			float const phase = 1.f / (4.f * math::Pi);

			result += (tv * tr) * (phase * light_sample.radiance)
					/ (light.pdf * light_sample.shape.pdf);
		}
	}

	return result;
}

Ray_marching_single_factory::Ray_marching_single_factory(take::Settings const& take_settings,
														 uint32_t num_integrators, float step_size,
														 float step_probability) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Ray_marching_single>(num_integrators)),
	settings_{step_size, step_probability} {}

Ray_marching_single_factory::~Ray_marching_single_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Ray_marching_single_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Ray_marching_single(rng, take_settings_, settings_);
}

}
