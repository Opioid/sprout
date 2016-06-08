#include "single_scattering.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/volume/volume.hpp"
#include "base/spectrum/rgb.inl"
#include "base/math/ray.inl"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"
#include "base/math/random/generator.inl"
#include "base/math/sampling/sampling.inl"

namespace rendering { namespace integrator { namespace volume {

Single_scattering::Single_scattering(const take::Settings& take_settings,
									 math::random::Generator& rng,
									 const Settings& settings) :
	Integrator(take_settings, rng), settings_(settings), sampler_(rng, 1) {}

math::float3 Single_scattering::transmittance(Worker& worker, const scene::volume::Volume* volume,
											  const scene::Ray& ray) {
	float min_t;
	float max_t;
	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
		return math::float3(1.f, 1.f, 1.f);
	}

	scene::Ray tray(ray.origin, ray.direction, min_t, max_t, ray.time);

	math::float3 tau = volume->optical_depth(tray);
	return math::exp(-tau);
}

math::float4 Single_scattering::li(Worker& worker, const scene::volume::Volume* volume,
								   const scene::Ray& ray, math::float3& transmittance) {
	float min_t;
	float max_t;
	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
		transmittance = math::float3(1.f, 1.f, 1.f);
		return math::float4_identity;
	}

//	min_t = ray.min_t;

//	float atmosphere_y = worker.scene().aabb().max().y;
//	max_t = (atmosphere_y - ray.origin.y) * ray.reciprocal_direction.y;

//	if (max_t < 0.f || max_t > ray.max_t) {
//		max_t = ray.max_t;
//	}

	float range = max_t - min_t;

	if (range < 0.0001f) {
		transmittance = math::float3(1.f, 1.f, 1.f);
		return math::float4_identity;
	}

	uint32_t num_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));

	float step = range / static_cast<float>(num_samples);

	math::float3 w = -ray.direction;

	math::float3 radiance = math::float3_identity;

	math::float3 scattering = volume->scattering();

	math::float3 tr(1.f, 1.f, 1.f);

	math::float3 current = ray.point(min_t);
	math::float3 previous;

	min_t += rng_.random_float() * step;

	for (uint32_t i = 0; i < num_samples; ++i, min_t += step) {
		previous = current;
		current  = ray.point(min_t);

		scene::Ray tau_ray(previous, current - previous, 0.f, 1.f, ray.time);
		math::float3 tau = volume->optical_depth(tau_ray);
		tr *= math::exp(-tau);

		// Direct light scattering
		float light_pdf;
		const auto light = worker.scene().montecarlo_light(rng_.random_float(), light_pdf);
		if (!light) {
			continue;
		}

		scene::light::Sample light_sample;
		light->sample(ray.time, current, sampler_, worker, Sampler_filter::Nearest, light_sample);

		if (light_sample.shape.pdf > 0.f) {
			scene::Ray shadow_ray(current, light_sample.shape.wi, 0.f,
								  light_sample.shape.t, ray.time);

			float mv = worker.masked_visibility(shadow_ray, Sampler_filter::Nearest);
			if (mv > 0.f) {
				float p = volume->phase(w, -light_sample.shape.wi);

				math::float3 l = Single_scattering::transmittance(worker, volume, shadow_ray)
							   * light_sample.radiance;

				radiance += p * mv * tr * scattering * l / (light_pdf * light_sample.shape.pdf);
			}
		}

		// Indirect light scattering
		/*
		math::float2 uv(rng_.random_float(), rng_.random_float());
		math::float3 dir = math::sample_sphere_uniform(uv);

		math::Oray scatter_ray(current, dir, 0.f, 10000.f, ray.time);

		math::float3 li = worker.surface_li(scatter_ray);

		float p = volume->phase(w, -dir);

		math::float3 l = Single_scattering::transmittance(volume, scatter_ray) * li;

		radiance += p * tr * scattering * l;
		*/
	}

	transmittance = tr;

	math::float3 color = step * radiance;

	return math::float4(color, spectrum::luminance(color));
}

Single_scattering_factory::Single_scattering_factory(const take::Settings& take_settings,
													 float step_size) :
	Integrator_factory(take_settings) {
	settings_.step_size = step_size;
}

Integrator* Single_scattering_factory::create(math::random::Generator& rng) const {
	return new Single_scattering(take_settings_, rng, settings_);
}

}}}
