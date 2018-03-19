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

#include "base/debug/assert.hpp"

namespace rendering::integrator::surface::sub {

Multiple_scattering::Multiple_scattering(rnd::Generator& rng, const take::Settings& take_settings/*,
										 const Settings& settings*/) :
	Integrator(rng, take_settings),
	// settings_(settings),
	sampler_(rng),
	material_samplers_{rng, rng} {}

void Multiple_scattering::prepare(const Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);

	for (auto& s : material_samplers_) {
		s.resize(num_samples_per_pixel, 1, 1, 2);
	}
}

void Multiple_scattering::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	sampler_.resume_pixel(sample, scramble);

	for (auto& s : material_samplers_) {
		s.resume_pixel(sample, scramble);
	}
}

float3 Multiple_scattering::li(const Ray& ray, Intersection& intersection,
							   const Material_sample& sample, Sampler_filter filter, 
							   Worker& worker, Bxdf_sample& sample_result) {
	float3 result(0.f);
	float3 tr(sample_result.reflection / sample_result.pdf);

	const auto bssrdf = sample.bssrdf();
	const float3 scattering = bssrdf.scattering_coefficient();
	const float average_scattering = math::average(scattering);
	const float3 adjusted_scattering = scattering / average_scattering;

	const uint32_t part = intersection.geo.part;
	const float2 uv = intersection.geo.uv;

	Ray tray;
	tray.depth = ray.depth + 1;
	tray.time  = ray.time;
	tray.wavelength = ray.wavelength;
	tray.properties = ray.properties;
	tray.properties.set(Ray::Property::Recursive);
	tray.set_primary(false);

	const float ray_offset_factor = take_settings_.ray_offset_factor;

	bool scattered = false;

	for (uint32_t i = 0; /*i < 256*/; ++i) {
		auto& sampler = material_sampler(ray.depth, i);

		tray.origin = intersection.geo.p;
		tray.set_direction(sample_result.wi);
		tray.min_t = ray_offset_factor * intersection.geo.epsilon;

		const float r = std::max(sampler.generate_sample_1D(1), 0.0000001f);
		tray.max_t = -std::log(r) / average_scattering;

		const bool hit = worker.intersect(intersection.prop, tray, intersection);

		tr *= bssrdf.transmittance(tray.max_t);

		const float average = math::average(tr);
		if (average < 0.1f) {
			if (rendering::russian_roulette(tr, 0.5f, rng_.random_float())) {
				sample_result.pdf = 0.f;
				return result;
			}
		}

		if (!hit) {
			scattered = true;
			// Scattering
			Ray secondary_ray = tray;

			intersection.geo.p = tray.point(tray.max_t);
			intersection.geo.epsilon = 0.f;

			scene::prop::Intersection secondary_intersection = intersection;
			secondary_intersection.geo.uv = uv;
			secondary_intersection.geo.part = part;
			secondary_intersection.geo.subsurface = true;

			// Prepare the next scattering event...
			const float3 wo = -tray.direction;
			auto& material_sample = secondary_intersection.sample(wo, ray, filter,
																  sampler, worker);

			material_sample.sample(sampler, sample_result);

			// ...before gathering the lighting at the current point
			const float3 local_radiance = worker.li(secondary_ray, secondary_intersection);

//			const float range = tray.max_t;
//			result += range * tr * scattering * local_radiance;

			result += tr * adjusted_scattering * local_radiance;

			// This should never happen for volumetric samples
			SOFT_ASSERT(sample_result.pdf > 0.f);

			tr *= sample_result.reflection / sample_result.pdf;
		} else {
			const float3 wo = -tray.direction;
			auto& material_sample = intersection.sample(wo, ray, filter, sampler, worker);

			material_sample.sample(sampler, sample_result);
			if (0.f == sample_result.pdf) {
				return result;
			}

			tr *= sample_result.reflection / sample_result.pdf;

			if (sample_result.type.test(Bxdf_type::Transmission)) {
				break;
			}
		}
	}

	if (scattered) {
		sample_result.pdf = 0.f;
	}

	sample_result.reflection = tr;

	return result;
}

size_t Multiple_scattering::num_bytes() const {
	size_t sampler_bytes = 0;

	for (const auto& s : material_samplers_) {
		sampler_bytes += s.num_bytes();
	}

	return sizeof(*this) + sampler_.num_bytes() + sampler_bytes;
}

sampler::Sampler& Multiple_scattering::material_sampler(uint32_t bounce, uint32_t iteration) {
	if (0 == bounce && Num_material_samplers > iteration) {
		return material_samplers_[iteration];
	}

	return sampler_;
}

Multiple_scattering_factory::Multiple_scattering_factory(const take::Settings& take_settings,
														 uint32_t num_integrators) :
	Factory(take_settings),
	integrators_(memory::allocate_aligned<Multiple_scattering>(num_integrators)) {}

Multiple_scattering_factory::~Multiple_scattering_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Multiple_scattering_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Multiple_scattering(rng, take_settings_/*, settings_*/);
}

}
