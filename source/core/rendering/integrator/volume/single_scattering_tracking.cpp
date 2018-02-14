#include "single_scattering_tracking.hpp"
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
#include "base/spectrum/rgb.hpp"

#include <iostream>
#include "math/print.hpp"

namespace rendering::integrator::volume {

Single_scattering_tracking::Single_scattering_tracking(rnd::Generator& rng, const take::Settings& take_settings,
									 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Single_scattering_tracking::prepare(const Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Single_scattering_tracking::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Single_scattering_tracking::transmittance(const Ray& ray, const Volume& volume,
										const Worker& worker) {
	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const auto& material = *volume.material(0);

	const float3 tau = material.optical_depth(transformation, volume.aabb(), ray,
											  settings_.step_size, rng_,
											  Sampler_filter::Nearest, worker);
	return math::exp(-tau);
}

float3 Single_scattering_tracking::li(const Ray& ray, const Volume& volume,
							 Worker& worker, float3& transmittance) {
	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);


	float3 radiance(0.f);


	const auto& material = *volume.material(0);

	const float3 sigma_a = material.absorption(transformation, float3::identity(),
											   Sampler_filter::Undefined, worker);

	const float3 sigma_s = material.scattering(transformation, float3::identity(),
											   Sampler_filter::Undefined, worker);

	const float3 extinction = sigma_a + sigma_s;

	const float3 scattering_albedo = sigma_s / extinction;


	const float3 tau = material.optical_depth(transformation, volume.aabb(), ray,
											  settings_.step_size, rng_,
											  Sampler_filter::Undefined, worker);

	const float3 tr = math::exp(-tau);


	const float r = rng_.random_float();
	const float scatter_distance = -std::log(1.f - r * (1.f - spectrum::average(tr))) / spectrum::average(extinction);

	const float range = ray.max_t - ray.min_t;

	// Lighting
	const float3 p = ray.point(ray.min_t + scatter_distance);

	const float3 local_radiance = estimate_direct_light(ray, p, worker);

	const float3 scatter_tr = math::exp(-scatter_distance * extinction);

	radiance += range * scatter_tr * extinction * scattering_albedo * local_radiance;
//	radiance += scatter_tr * sigma_s * local_radiance;

	transmittance = tr;

	return radiance;
}

size_t Single_scattering_tracking::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

float3 Single_scattering_tracking::estimate_direct_light(const Ray& ray, const float3& position,
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

Single_scattering_tracking_factory::Single_scattering_tracking_factory(const take::Settings& take_settings,
													 uint32_t num_integrators, float step_size, 
													 float step_probability,
													 bool indirect_lighting) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Single_scattering_tracking>(num_integrators)),
	settings_{step_size, step_probability, !indirect_lighting} {}

Single_scattering_tracking_factory::~Single_scattering_tracking_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Single_scattering_tracking_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Single_scattering_tracking(rng, take_settings_, settings_);
}

}
