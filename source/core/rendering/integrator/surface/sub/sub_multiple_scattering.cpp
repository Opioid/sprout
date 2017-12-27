#include "sub_multiple_scattering.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/bssrdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"

namespace rendering::integrator::surface::sub {

Multiple_scattering::Multiple_scattering(rnd::Generator& rng, const take::Settings& take_settings,
										 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Multiple_scattering::prepare(const Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Multiple_scattering::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Multiple_scattering::li(const Ray& ray, Intersection& intersection,
							   const Material_sample& sample, Sampler_filter filter, 
							   Worker& worker, Bxdf_sample& sample_result) {
	float3 result(0.f);
	float3 tr(sample_result.reflection / sample_result.pdf);

	const auto bssrdf = sample.bssrdf();
	const float3 scattering = bssrdf.scattering_coefficient();

	const float sigma_t = spectrum::average(bssrdf.extinction_coefficient());

	const uint32_t part = intersection.geo.part;

	Ray tray;
	tray.time  = ray.time;
	tray.depth = ray.depth + 1;
	tray.properties = ray.properties;
	tray.properties.set(Ray::Property::Recursive);
	tray.set_primary(false);

	const float ray_offset_factor = take_settings_.ray_offset_factor;

	for (uint32_t i = 0; /*i < 256*/; ++i) {
		const float ray_offset = ray_offset_factor * intersection.geo.epsilon;
		tray.origin = intersection.geo.p;
		tray.set_direction(sample_result.wi);
		tray.min_t = ray_offset;

		const float r = math::clamp(rng_.random_float(), 0.001f, 0.999f);

		tray.max_t = -std::log(r) / sigma_t;

		if (!worker.intersect(intersection.prop, tray, intersection)) {

			const float3 tau = bssrdf.optical_depth(tray.max_t);
			tr *= math::exp(-tau);

			const float average = spectrum::average(tr);
			if (average < 0.01f) {
				//	if (rendering::russian_roulette(tr, 0.5f, rng_.random_float())) {
				sample_result.pdf = 0.f;
			//	result += /*step **/ radiance;
				return result;
				//	}
			}

			// Lighting
			Ray secondary_ray = tray;

			intersection.geo.p = tray.point(tray.max_t);

			scene::prop::Intersection secondary_intersection = intersection;
			secondary_intersection.geo.part = part;
			secondary_intersection.inside_volume = true;

			const float3 local_radiance = worker.li(secondary_ray, secondary_intersection);

			const float range = tray.max_t - tray.min_t;

			result += range * tr * scattering * local_radiance;



			const float3 wo = -tray.direction;
			auto& material_sample = intersection.sample(wo, ray.time, filter, worker);

			material_sample.sample(sampler_, sample_result);
			if (0.f == sample_result.pdf) {
				break;
			}

			tr *= scattering * (sample_result.reflection / sample_result.pdf);

			continue;
		//	break;
		} else {
			const float3 tau = bssrdf.optical_depth(tray.max_t);
			tr *= math::exp(-tau);

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

		float range = tray.max_t - tray.min_t;
		if (range < 0.0001f) {
		//	sample_result.pdf = 0.f;
			break;
		}


	}

	sample_result.reflection = tr;

	return result;
}

size_t Multiple_scattering::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

Multiple_scattering_factory::Multiple_scattering_factory(const take::Settings& take_settings,
														 uint32_t num_integrators) :
	Factory(take_settings),
	integrators_(memory::allocate_aligned<Multiple_scattering>(num_integrators)) {}

Multiple_scattering_factory::~Multiple_scattering_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Multiple_scattering_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Multiple_scattering(rng, take_settings_, settings_);
}

}
