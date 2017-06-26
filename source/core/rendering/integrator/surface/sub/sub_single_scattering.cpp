#include "sub_single_scattering.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_intersection.inl"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/bssrdf.hpp"
#include "scene/material/material_sample.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace surface { namespace sub {

Single_scattering::Single_scattering(rnd::Generator& rng, const take::Settings& take_settings,
									 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Single_scattering::prepare(const Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Single_scattering::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Single_scattering::li(Worker& worker, const Ray& ray, Intersection& intersection,
							 Sampler_filter filter, Bxdf_result& sample_result) {
	if (0.f == sample_result.pdf) {
		return float3(0.f);
	}

	const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	Ray tray(intersection.geo.p, sample_result.wi,
			 ray_offset, scene::Ray_max_t, ray.time, ray.depth);
	if (!worker.intersect(intersection.prop, tray, intersection)) {
		return float3(0.f);
	}

	const float range = tray.max_t - tray.min_t;
	if (range < 0.0001f) {
		return float3(0.f);
	}

	const auto& bssrdf = intersection.bssrdf(worker);

	const uint32_t num_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));
	const float num_samples_reciprocal = 1.f / static_cast<float>(num_samples);
	const float step = range * num_samples_reciprocal;

	float3 radiance(0.f);
	float3 tr(sample_result.reflection / sample_result.pdf);

	float tau_ray_length = rng_.random_float() * step;

	float min_t = tray.min_t + tau_ray_length;

	for (uint32_t i = num_samples; i > 0; --i, min_t += step) {
		const float3 tau = bssrdf.optical_depth(tau_ray_length);
		tr *= math::exp(-tau);

		tau_ray_length = step;

		const float3 current = tray.point(min_t);

		// Direct light scattering
		radiance += tr * estimate_direct_light(current, intersection.prop, bssrdf,
											   ray.time, sampler_, worker);
	}

	const float3 wo = -tray.direction;
	auto& material_sample_2 = intersection.sample(worker, wo, ray.time, filter);

	material_sample_2.sample_sss(sampler_, sample_result);
	sample_result.reflection *= tr;

	return step * radiance;
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

}}}}
