#include "sub_single_scattering.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_intersection.inl"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bssrdf.hpp"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace surface { namespace sub {

Single_scattering::Single_scattering(rnd::Generator& rng, const take::Settings& take_settings,
									 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Single_scattering::prepare(const Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Single_scattering::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Single_scattering::li(Worker& worker, const Ray& ray, const Intersection& intersection) {
	float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	Ray tray(intersection.geo.p, ray.direction, ray_offset, scene::Ray_max_t);
	Intersection tintersection;
	if (!worker.intersect(intersection.prop, tray, tintersection)) {
		return float3(0.f);
	}

	float range = tray.max_t - tray.min_t;

	if (range < 0.0001f) {
		return float3(0.f);
	}

	const auto& bssrdf = intersection.bssrdf(worker);

	const uint32_t num_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));

	const float step = range / static_cast<float>(num_samples);

//	float3 w = -ray.direction;

	float3 radiance(0.f);
	float3 tr(1.f);

	float min_t = tray.min_t;
	float tau_ray_length = rng_.random_float() * step;

	min_t += tau_ray_length;

	for (uint32_t i = 0; i < num_samples; ++i, min_t += step) {
		float3 tau = bssrdf.optical_depth(tau_ray_length);
		tr *= math::exp(-tau);

		tau_ray_length = step;

		// Direct light scattering
		float light_pdf;
		const auto light = worker.scene().random_light(rng_.random_float(), light_pdf);
		if (!light) {
			continue;
		}

		float3 current = tray.point(min_t);

		scene::light::Sample light_sample;
		light->sample(ray.time, current, sampler_, 0, worker,
					  Sampler_filter::Nearest, light_sample);

		if (light_sample.shape.pdf > 0.f) {
			Ray shadow_ray(current, light_sample.shape.wi, 0.f,
						   light_sample.shape.t, ray.time);

			if (!worker.intersect(intersection.prop, shadow_ray, tintersection)) {
				continue;
			}

			float prop_length = shadow_ray.length();

			ray_offset = take_settings_.ray_offset_factor * tintersection.geo.epsilon;
			shadow_ray.min_t = shadow_ray.max_t + ray_offset;
			shadow_ray.max_t = light_sample.shape.t - ray_offset;

			float mv = worker.masked_visibility(shadow_ray, Sampler_filter::Nearest);
			if (mv > 0.f) {
			//	float p = volume.phase(w, -light_sample.shape.wi);
				float p = 1.f / (4.f * math::Pi);

				float3 scattering = bssrdf.scattering();

				tau = bssrdf.optical_depth(prop_length);
				float3 transmittance = math::exp(-tau);

				float3 l = transmittance * light_sample.radiance;

				radiance += p * mv * tr * scattering * l / (light_pdf * light_sample.shape.pdf);
			}
		}
	}

	float3 color = step * radiance;
	return color;
}

size_t Single_scattering::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

Single_scattering_factory::Single_scattering_factory(const take::Settings& take_settings,
													 uint32_t num_integrators, float step_size) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Single_scattering>(num_integrators)),
	settings_{ step_size } {}

Single_scattering_factory::~Single_scattering_factory() {
	memory::destroy_aligned(integrators_, num_integrators_);
}

Integrator* Single_scattering_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Single_scattering(rng, take_settings_, settings_);
}


}}}}
