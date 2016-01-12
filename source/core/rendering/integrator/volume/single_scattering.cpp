#include "single_scattering.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/volume/volume.hpp"
#include "base/math/ray.inl"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"
#include "base/math/random/generator.inl"

#include <iostream>

namespace rendering { namespace integrator { namespace volume {

Single_scattering::Single_scattering(const take::Settings& take_settings,
									 math::random::Generator& rng,
									 const Settings& settings) :
	Integrator(take_settings, rng), settings_(settings), sampler_(rng, 1) {}

math::float3 Single_scattering::transmittance(const scene::volume::Volume* volume, const math::Oray& ray) {
	math::float3 tau = volume->optical_depth(ray);
	return math::exp(-tau);
}

math::float3 Single_scattering::li(Worker& worker, const scene::volume::Volume* volume, const math::Oray& ray,
								   math::float3& transmittance) {
	float min_t;
	float max_t;
/*	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
		transmittance = math::float3(1.f, 1.f, 1.f);
		return math::float3::identity;
	}
*/
	min_t = ray.min_t;

	float atmosphere_y = worker.scene().aabb().max().y;
	max_t = (atmosphere_y - ray.origin.y) * ray.reciprocal_direction.y;

	if (max_t < 0.f || max_t > ray.max_t) {
		max_t = ray.max_t;
	}


	math::float3 emission = math::float3::identity;

	math::float3 scattering = volume->scattering();

	float step_size = 0.5f;

	uint32_t num_samples = static_cast<uint32_t>(std::ceil((max_t - min_t) / step_size));
	float step = (max_t - min_t) / static_cast<float>(num_samples);

	math::float3 tr(1.f, 1.f, 1.f);

	math::float3 current = ray.point(min_t);
	math::float3 previous;

	min_t += rng_.random_float() * step;

	for (uint32_t i = 0; i < num_samples; ++i, min_t += step) {
		previous = current;
		current  = ray.point(min_t);

		math::Oray tau_ray;
		tau_ray.origin = previous;
		tau_ray.set_direction(current - previous);
		tau_ray.min_t = 0.f;
		tau_ray.max_t = 1.f;

		math::float3 tau = volume->optical_depth(tau_ray);
		tr *= math::exp(-tau);

		float light_pdf;
		const scene::light::Light* light = worker.scene().montecarlo_light(rng_.random_float(), light_pdf);
		if (!light) {
			continue;
		}

		scene::light::Sample light_sample;
		light->sample(0.f,
					  current, math::float3::identity, true,
					  settings_.sampler_nearest, sampler_, worker.node_stack(), light_sample);

		if (light_sample.shape.pdf > 0.f) {
			math::Oray shadow_ray;
			shadow_ray.origin = current;
			shadow_ray.set_direction(light_sample.shape.wi);
			shadow_ray.min_t = 0.f;
			shadow_ray.max_t = light_sample.shape.t;

			float mv = worker.masked_visibility(shadow_ray, settings_.sampler_nearest);
			if (mv > 0.f) {
				float p = volume->phase(-math::normalized(ray.direction), -light_sample.shape.wi);
				emission += p * mv * tr * scattering * light_sample.energy / (light_pdf * light_sample.shape.pdf);
			} else {
			//	std::cout << "shadow" << std::endl;
			}
		}

	}

	transmittance = tr;

	return step * emission;
}

Single_scattering_factory::Single_scattering_factory(const take::Settings& settings) :
	Integrator_factory(settings) {}

Integrator* Single_scattering_factory::create(math::random::Generator& rng) const {
	return new Single_scattering(take_settings_, rng, settings_);
}

}}}
