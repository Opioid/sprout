#include "single_scattering.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/volume/volume.hpp"
#include "base/math/ray.inl"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"
#include "base/math/sampling/sampling.inl"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.inl"

namespace rendering { namespace integrator { namespace volume {

Single_scattering::Single_scattering(const take::Settings& take_settings,
									 rnd::Generator& rng,
									 const Settings& settings) :
	Integrator(take_settings, rng),
	settings_(settings),
	sampler_(rng) {}

void Single_scattering::prepare(const scene::Scene& /*scene*/,
								uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Single_scattering::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Single_scattering::transmittance(Worker& worker, const scene::volume::Volume& volume,
										const scene::Ray& ray) {
	float min_t;
	float max_t;
	if (!volume.aabb().intersect_p(ray, min_t, max_t)) {
		return float3(1.f);
	}

	scene::Ray tray(ray.origin, ray.direction, min_t, max_t, ray.time);

	float3 tau = volume.optical_depth(tray, settings_.step_size, rng_,
									  worker, Sampler_filter::Nearest);
	return math::exp(-tau);
}

float4 Single_scattering::li(Worker& worker, const scene::volume::Volume& volume,
							 const scene::Ray& ray, float3& transmittance) {
	float min_t;
	float max_t;
//	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
	if (!volume.aabb().intersect_p(ray, min_t, max_t)) {
		transmittance = float3(1.f);
		return float4(0.f);
	}

	float range = max_t - min_t;

	if (range < 0.0001f) {
		transmittance = float3(1.f);
		return float4(0.f);
	}

	uint32_t num_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));

	float step = range / static_cast<float>(num_samples);

	float3 w = -ray.direction;

	float3 radiance(0.f);
	float3 tr(1.f);

	float3 current = ray.point(min_t);
	float3 previous;

	min_t += rng_.random_float() * step;

	for (uint32_t i = 0; i < num_samples; ++i, min_t += step) {
		previous = current;
		current  = ray.point(min_t);

		scene::Ray tau_ray(previous, current - previous, 0.f, 1.f, ray.time);
		float3 tau = volume.optical_depth(tau_ray, settings_.step_size, rng_,
										  worker, Sampler_filter::Unknown);
		tr *= math::exp(-tau);

		// Direct light scattering
		float light_pdf;
		const auto light = worker.scene().random_light(rng_.random_float(), light_pdf);
		if (!light) {
			continue;
		}

		scene::light::Sample light_sample;
		light->sample(ray.time, current, sampler_, 0, worker,
					  Sampler_filter::Nearest, light_sample);

		if (light_sample.shape.pdf > 0.f) {
			scene::Ray shadow_ray(current, light_sample.shape.wi, 0.f,
								  light_sample.shape.t, ray.time);

			float mv = worker.masked_visibility(shadow_ray, Sampler_filter::Nearest);
			if (mv > 0.f) {
				float p = volume.phase(w, -light_sample.shape.wi);

				float3 scattering = volume.scattering(current, worker, Sampler_filter::Unknown);

				float3 l = Single_scattering::transmittance(worker, volume, shadow_ray)
							   * light_sample.radiance;

				radiance += p * mv * tr * scattering * l / (light_pdf * light_sample.shape.pdf);
			}
		}

		// Indirect light scattering
		/*
		float2 uv(rng_.random_float(), rng_.random_float());
		float3 dir = math::sample_sphere_uniform(uv);

		math::Ray scatter_ray(current, dir, 0.f, 10000.f, ray.time);

		float3 li = worker.surface_li(scatter_ray);

		float p = volume->phase(w, -dir);

		float3 l = Single_scattering::transmittance(volume, scatter_ray) * li;

		radiance += p * tr * scattering * l;
		*/
	}

	transmittance = tr;

	float3 color = step * radiance;

	return float4(color, spectrum::luminance(color));
}

size_t Single_scattering::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

Single_scattering_factory::Single_scattering_factory(const take::Settings& take_settings,
													 float step_size) :
	Factory(take_settings) {
	settings_.step_size = step_size;
}

Integrator* Single_scattering_factory::create(rnd::Generator& rng) const {
	return new Single_scattering(take_settings_, rng, settings_);
}

}}}
