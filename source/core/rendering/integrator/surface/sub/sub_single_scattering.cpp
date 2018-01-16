#include "sub_single_scattering.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/bssrdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"

namespace rendering::integrator::surface::sub {

Single_scattering::Single_scattering(rnd::Generator& rng, const take::Settings& take_settings,
									 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Single_scattering::prepare(const Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Single_scattering::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Single_scattering::li(const Ray& ray, Intersection& intersection, 
							 const Material_sample& sample, Sampler_filter filter, 
							 Worker& worker, Bxdf_sample& sample_result) {
	float3 result(0.f);
	float3 tr(sample_result.reflection / sample_result.pdf);

	const uint32_t part = intersection.geo.part;

	const auto bssrdf = sample.bssrdf();
	const float3 scattering = bssrdf.scattering_coefficient();

	const float sigma_t = spectrum::average(bssrdf.extinction_coefficient());
	const float step_size = -std::log(settings_.step_probability) / sigma_t;

	Ray tray;
	tray.time  = ray.time;
	tray.depth = ray.depth + 1;
	tray.properties = ray.properties;
	tray.properties.set(Ray::Property::Recursive);
	tray.set_primary(false);

	const float ray_offset_factor = take_settings_.ray_offset_factor;

	for (uint32_t i = 0; /*i < 256*/; ++i) {
		tray.origin = intersection.geo.p;
		tray.set_direction(sample_result.wi);
		tray.min_t = ray_offset_factor * intersection.geo.epsilon;;
		tray.max_t = scene::Ray_max_t;

		if (!worker.intersect(intersection.prop, tray, intersection)) {
			break;
		}

		const float range = tray.max_t;

		const float max_samples = std::ceil(range / step_size);
		const float num_samples = (ray.is_primary() && 0 == i) ? max_samples : 1.f;
	
		const float step = range / num_samples;

		float3 radiance(0.f);

		const float r = rng_.random_float();
		const float tau_ray_length = r * step;

		float3 attenuation = math::exp(-bssrdf.optical_depth(tau_ray_length));

		float min_t = tray.min_t + tau_ray_length;

		for (uint32_t j = static_cast<uint32_t>(num_samples); j > 0; --j, min_t += step) {
			if (num_samples - 1 == j) {
				attenuation = math::exp(-bssrdf.optical_depth(step));
			}

			tr *= attenuation;

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

			scene::prop::Intersection secondary_intersection = intersection;
			secondary_intersection.geo.p = tray.point(min_t);
			secondary_intersection.geo.part = part;
			secondary_intersection.geo.epsilon = 0.f;
			secondary_intersection.geo.inside_volume = true;

			const float3 local_radiance = worker.li(secondary_ray, secondary_intersection);

			radiance += (step * tr) * (scattering * local_radiance);
		}

		result += /*step **/ radiance;

		const float3 wo = -tray.direction;
		auto& material_sample = intersection.sample(wo, ray, filter, sampler_, worker);

		material_sample.sample(sampler_, sample_result);
		if (0.f == sample_result.pdf) {
			return result;
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
													 uint32_t num_integrators,
													 float step_probability) :
	Factory(take_settings),
	integrators_(memory::allocate_aligned<Single_scattering>(num_integrators)),
	settings_{step_probability} {}

Single_scattering_factory::~Single_scattering_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Single_scattering_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Single_scattering(rng, take_settings_, settings_);
}

}
