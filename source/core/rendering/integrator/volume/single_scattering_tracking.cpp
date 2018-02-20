#include "single_scattering_tracking.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/prop/prop_intersection.inl"
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

enum class Algorithm {
	PBRT,
	Algorithm_1,
	Listing_10,
	Experiment
};

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

	const float d = ray.max_t - ray.min_t;

	transmittance = math::exp(-d * extinction);

	constexpr Algorithm algorithm = Algorithm::Listing_10;
/*
	if (Algorithm::PBRT == algorithm) {
		// PBRT style raymarching
	} else if (Algorithm::Algorithm_1 == algorithm) {
		const float r = rng_.random_float();
		const float scatter_distance = -std::log(1.f - r) / spectrum::average(extinction);

		if (scatter_distance > d) {
			// boundary
			return float3(0.f);
		}

		const float r2 = rng_.random_float();
		if (r2 < spectrum::average(sigma_a) / spectrum::average(extinction)) {
			// absorption/emission
			return float3(0.f);
		}

		const float3 p = ray.point(ray.min_t + scatter_distance);

		const float3 l = estimate_direct_light(ray, p, worker);

		radiance += l;
	} else if (Algorithm::Listing_10 == algorithm) {
		const float r = rng_.random_float();
		const float scatter_distance = -std::log(1.f - r * (1.f - spectrum::average(transmittance))) / spectrum::average(extinction);

	//	const float3 tr = math::exp(-scatter_distance * extinction);

		const float3 p = ray.point(ray.min_t + scatter_distance);

		float3 l = estimate_direct_light(ray, p, worker);

	//	l *= (1.f - transmittance) / (extinction * tr);

	//	l *= extinction * scattering_albedo * tr;

		l *= (1.f - transmittance) * scattering_albedo;

		radiance += l;
	} else if (Algorithm::Experiment == algorithm) {
		const float r = rng_.random_float();
		const float scatter_distance = r * d;

		const float3 tr = math::exp(-scatter_distance * extinction);

		const float3 p = ray.point(ray.min_t + scatter_distance);

		float3 l = estimate_direct_light(ray, p, worker);

		l *= d * extinction * scattering_albedo * tr;

		radiance += l;
	}
*/
	return radiance;
}

bool Single_scattering_tracking::integrate(Ray& ray, Intersection& intersection,
										   const Material_sample& material_sample,
										   Worker& worker, float3& li, float3& transmittance) {
	Transformation temp;
	const auto& transformation = intersection.prop->transformation_at(ray.time, temp);

	const auto material = intersection.material();

	const float3 sigma_a = material->absorption(transformation, float3::identity(),
											   Sampler_filter::Undefined, worker);

	const float3 sigma_s = material->scattering(transformation, float3::identity(),
											   Sampler_filter::Undefined, worker);

	const float3 extinction = sigma_a + sigma_s;

	const float3 scattering_albedo = sigma_s / extinction;

	const bool hit = worker.intersect_and_resolve_mask(ray, intersection, Sampler_filter::Nearest);
	if (!hit) {
		li = float3(0.f);
		transmittance = float3(1.f);
		return false;
	}

	const float d = ray.max_t;

	const bool multiple_scattering = true;

	if (multiple_scattering) {
		const float r = rng_.random_float();
		const float scatter_distance = -std::log(1.f - r) / spectrum::average(extinction);

		if (scatter_distance < d) {
			const float3 p = ray.point(scatter_distance);

			intersection.geo.p = p;
			intersection.geo.epsilon = 0.f;
			intersection.geo.subsurface = true;

			transmittance = math::exp(-scatter_distance * extinction);
		} else {
			transmittance = math::exp(-d * extinction);
		}

		li = float3(0.f);
		return true;
	} else {
		transmittance = math::exp(-d * extinction);

		const float r = rng_.random_float();
		const float scatter_distance = -std::log(1.f - r * (1.f - spectrum::average(transmittance))) / spectrum::average(extinction);

		const float3 p = ray.point(scatter_distance);

		float3 l = estimate_direct_light(ray, p, intersection, material_sample, worker);

		l *= (1.f - transmittance) * scattering_albedo;

		li = l;

		return true;
	}
}

size_t Single_scattering_tracking::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

float3 Single_scattering_tracking::estimate_direct_light(const Ray& ray, const float3& position,
														 const Intersection& intersection,
														 const Material_sample& material_sample,
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

	//	const float3 tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);

		Intersection tintersection = intersection;
		tintersection.geo.subsurface = true;
		const float3 tv = worker.tinted_visibility(shadow_ray, tintersection, material_sample, Sampler_filter::Nearest);
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
