#include "sub_single_scattering.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/bssrdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"

#include <iostream>

namespace rendering::integrator::surface::sub {

Single_scattering::Single_scattering(rnd::Generator& rng, const take::Settings& take_settings,
									 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Single_scattering::prepare(const Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Single_scattering::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Single_scattering::li(const Ray& ray, bool /*primary_ray*/, Intersection& intersection,
							 const Material_sample& sample, Sampler_filter filter,
							 Worker& worker, Bxdf_sample& sample_result) {
	float3 result(0.f);
	float3 tr(sample_result.reflection / sample_result.pdf);

	const auto bssrdf = sample.bssrdf();
	const float3 scattering = bssrdf.scattering_coefficient();

	const uint32_t part = intersection.geo.part;

	Ray tray;
	tray.time  = ray.time;
	tray.depth = ray.depth + 1;

	const float ray_offset_factor = take_settings_.ray_offset_factor;

	for (uint32_t i = 0; /*i < 256*/; ++i) {
		const float ray_offset = ray_offset_factor * intersection.geo.epsilon;
		tray.origin = intersection.geo.p;
		tray.set_direction(sample_result.wi);
		tray.min_t = ray_offset;
		tray.max_t = scene::Ray_max_t;

		if (!worker.intersect(intersection.prop, tray, intersection)) {
			break;
		}

		float range = tray.max_t - tray.min_t;
		if (range < 0.0001f) {
		//	sample_result.pdf = 0.f;
			break;
		}

	//	range = std::max(range, 0.01f);

		const uint32_t max_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));
		const uint32_t num_samples = (0 == ray.depth && 0 == i) ? max_samples : 1;
		const float num_samples_reciprocal = 1.f / static_cast<float>(num_samples);
		const float step = range * num_samples_reciprocal;

		float3 radiance(0.f);

		const float r = rng_.random_float();
		float tau_ray_length = r * step;

	//	tau_ray_length = std::max(tau_ray_length, 0.001f);

		float min_t = tray.min_t + tau_ray_length;

		for (uint32_t j = num_samples; j > 0; --j, min_t += step) {
			const float3 tau = bssrdf.optical_depth(tau_ray_length);
			tr *= math::exp(-tau);

			const float average = spectrum::average(tr);
			if (average < 0.01f) {
			//	if (rendering::russian_roulette(tr, 0.5f, rng_.random_float())) {
					sample_result.pdf = 0.f;
					result += /*step **/ radiance;
					return result;
			//	}
			}

			// Lighting
			Ray secondary_ray = tray;
			secondary_ray.properties.set(Ray::Property::Recursive);

			scene::prop::Intersection secondary_intersection = intersection;
			secondary_intersection.geo.p = tray.point(min_t);
			secondary_intersection.geo.part = part;
			secondary_intersection.inside_volume = true;

			const float3 local_radiance = worker.li(secondary_ray, secondary_intersection).xyz();

			radiance += step * tr * scattering * local_radiance;

			tau_ray_length = step;
		}

		result += /*step **/ radiance;

		const float3 wo = -tray.direction;
		auto& material_sample = intersection.sample(wo, ray.time, filter, worker);

		material_sample.sample(sampler_, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		tr *= sample_result.reflection / sample_result.pdf;

		if (sample_result.type.test(Bxdf_type::Transmission)) {
			break;
		}
	}

	sample_result.reflection = tr;

	return result;
}

size_t Single_scattering::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

Single_scattering_factory::Single_scattering_factory(const take::Settings& take_settings,
													 uint32_t num_integrators, float step_size) :
	Factory(take_settings),
	integrators_(memory::allocate_aligned<Single_scattering>(num_integrators)),
	settings_{ step_size } {}

Single_scattering_factory::~Single_scattering_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Single_scattering_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Single_scattering(rng, take_settings_, settings_);
}

}
