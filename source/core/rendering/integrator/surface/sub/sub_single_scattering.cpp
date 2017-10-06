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
							 const Material_sample& sample, Sampler_filter filter,
							 Bxdf_result& sample_result) {
	float3 result(0.f);
	float3 tr(sample_result.reflection / sample_result.pdf);

	Ray tray;
	tray.time  = ray.time;
	tray.depth = ray.depth;

	const float ray_offset_factor = take_settings_.ray_offset_factor;

	for (uint32_t i = 0; /*i < 1*/; ++i) {
		const float ray_offset = ray_offset_factor * intersection.geo.epsilon;
		tray.origin = intersection.geo.p;
		tray.set_direction(sample_result.wi);
		tray.min_t = ray_offset;
		tray.max_t = scene::Ray_max_t;

		if (!worker.intersect(intersection.prop, tray, intersection)) {
			break;
		}

		const float range = tray.max_t - tray.min_t;
		if (range < 0.0001f) {
			break;
		}

		const auto& bssrdf = sample.bssrdf(worker);

		const uint32_t max_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));
		const uint32_t num_samples = 0 == i ? max_samples : 1;
		const float num_samples_reciprocal = 1.f / static_cast<float>(num_samples);
		const float step = range * num_samples_reciprocal;

		float3 radiance(0.f);

		float tau_ray_length = rng_.random_float() * step;

		float min_t = tray.min_t + tau_ray_length;

		for (uint32_t j = num_samples; j > 0; --j, min_t += step) {
			const float3 tau = bssrdf.optical_depth(tau_ray_length);
			tr *= math::exp(-tau);

			tau_ray_length = step;

			const float3 current = tray.point(min_t);

			// Direct light scattering
			radiance += tr * estimate_direct_light(current, intersection.prop, bssrdf,
												   ray.time, ray.depth, sampler_, worker);
		}

		result += step * radiance;

		const float3 wo = -tray.direction;
		auto& material_sample = intersection.sample(wo, ray.time, filter, worker);

		material_sample.sample(sampler_, sample_result);

		if (0.f == sample_result.pdf) {
			break;
		}

		tr *= sample_result.reflection / sample_result.pdf;

//		if (math::all_lesser(tr, 1.0f)) {
//			break;
//		}

		if (sample_result.type.test(Bxdf_type::Transmission)) {
			break;
		} /*else {
			sample_result.pdf = 0.f;
		}*/
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

}}}}
