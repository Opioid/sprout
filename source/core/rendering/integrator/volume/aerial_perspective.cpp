#include "aerial_perspective.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.hpp"
#include "scene/shape/shape.hpp"
#include "scene/volume/volume.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

namespace rendering::integrator::volume {

Aerial_perspective::Aerial_perspective(rnd::Generator& rng, const take::Settings& take_settings,
									   const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Aerial_perspective::prepare(const Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Aerial_perspective::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Aerial_perspective::transmittance(const Ray& /*ray*/, const Volume& /*volume*/,
										 const Worker& /*worker*/) {
//	Transformation temp;
//	const auto& transformation = volume.transformation_at(ray.time, temp);

//	const auto& material = *volume.material(0);

//	const float3 tau = material.optical_depth(transformation, volume.aabb(), ray,
//											  settings_.step_size, rng_,
//											  Sampler_filter::Nearest, worker);
//	return math::exp(-tau);

	return float3(1.f);
}

float3 Aerial_perspective::li(const Ray& ray, bool primary_ray, const Volume& volume,
							  Worker& worker, float3& transmittance) {
	if (ray.properties.test(Ray::Property::Recursive)) {
		transmittance = Aerial_perspective::transmittance(ray, volume, worker);
		return float3::identity();
	}

	if (ray.max_t >= scene::Almost_ray_max_t) {
		transmittance = float3(1.f);
		return float3::identity();
	}

	if (settings_.disable_shadows) {
		return integrate_without_shadows(ray, primary_ray, volume, worker, transmittance);
	}

	return integrate_with_shadows(ray, primary_ray, volume, worker, transmittance);
}

size_t Aerial_perspective::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

float3 Aerial_perspective::integrate_with_shadows(const Ray& ray, bool primary_ray,
												  const Volume& volume, Worker& worker,
												  float3& transmittance) {
	float min_t = ray.min_t;
	const float range = ray.max_t - min_t;

	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const uint32_t max_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));
	const uint32_t num_samples = primary_ray ? max_samples : 1;

	const float step = range / static_cast<float>(num_samples);

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

		// Lighting
		Ray secondary_ray = ray;
		secondary_ray.properties.set(Ray::Property::Recursive);
		secondary_ray.depth = 0xFFFFFFFE;

		scene::prop::Intersection secondary_intersection;
		secondary_intersection.prop = &volume;
		secondary_intersection.geo.p = current;
		secondary_intersection.geo.part = 0;
		secondary_intersection.geo.epsilon = 0.0005f;

		const float3 local_radiance = worker.li(secondary_ray, secondary_intersection).xyz();

		const float3 scattering = material.scattering(transformation, current,
													  Sampler_filter::Undefined, worker);

		radiance += tr * scattering * local_radiance;
	}

	transmittance = tr;

	const float3 color = step * radiance;

	return color;
}

float3 Aerial_perspective::integrate_without_shadows(const Ray& ray, bool primary_ray,
													 const Volume& volume, Worker& worker,
													 float3& transmittance) {
	float min_t = ray.min_t;
	const float range = ray.max_t - min_t;

	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const uint32_t max_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));
	const uint32_t num_samples = primary_ray ? max_samples : 1;

	const float step = range / static_cast<float>(num_samples);

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

		auto& material_sample = sample(-ray.direction, ray.time, material,
									   Sampler_filter::Undefined, worker);

		float3 local_radiance(0.f);

		// Lighting
		const uint32_t num_light_samples = 1;
		for (uint32_t i = num_light_samples; i > 0; --i) {
			const auto light = worker.scene().random_light(rng_.random_float());

			scene::light::Sample light_sample;
			if (light.ref.sample(start, float3::identity(), ray.time, true, sampler_, 0,
								 Sampler_filter::Nearest, worker, light_sample)) {

				const auto bxdf = material_sample.evaluate(light_sample.shape.wi);

				local_radiance += (light_sample.radiance * bxdf.reflection)
								/ (light.pdf * light_sample.shape.pdf);
			}
		}

		const float3 scattering = material.scattering(transformation, current,
													  Sampler_filter::Undefined, worker);

		radiance += tr * scattering * local_radiance;
	}

	transmittance = tr;

	const float3 color = step * radiance;

	return color;
}

const scene::material::Sample& Aerial_perspective::sample(const float3& wo, float time,
														  const Material& material,
														  Sampler_filter filter,
														  Worker& worker) {
	scene::Renderstate rs;

	rs.time = time;

	return material.sample(wo, rs, filter, worker);
}

Aerial_perspective_factory::Aerial_perspective_factory(const take::Settings& take_settings,
													   uint32_t num_integrators, float step_size,
													   bool shadows) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Aerial_perspective>(num_integrators)),
	settings_{step_size, !shadows} {}

Aerial_perspective_factory::~Aerial_perspective_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Aerial_perspective_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Aerial_perspective(rng, take_settings_, settings_);
}

}

