#include "sub_bruteforce.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_intersection.inl"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bssrdf.hpp"
#include "base/math/vector4.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

#include <iostream>

namespace rendering { namespace integrator { namespace surface { namespace sub {

Bruteforce::Bruteforce(rnd::Generator& rng, const take::Settings& take_settings,
					   const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Bruteforce::prepare(const Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Bruteforce::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Bruteforce::li(Worker& worker, const Ray& ray, const Intersection& intersection,
					  const Material_sample& material_sample) {
	float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
	Ray tray(intersection.geo.p, ray.direction, ray_offset, scene::Ray_max_t);
	Intersection tintersection;
	if (!worker.intersect(intersection.prop, tray, tintersection)) {
		return float3(0.f);
	}

	const float range = tray.max_t - tray.min_t;

	if (range < 0.0001f) {
		return float3(0.f);
	}

	const auto& bssrdf = intersection.bssrdf(worker);

	const uint32_t num_samples = 1;//static_cast<uint32_t>(std::ceil(range / settings_.step_size));

	const float step = range / static_cast<float>(num_samples);

	float3 radiance(0.f);
	float3 tr(1.f);

	const float3 scatter = bssrdf.scattering();
	const float average_scatter = (1.f / 3.f) * (scatter[0] + scatter[1] + scatter[2]);

	float tau_ray_length = rng_.random_float() * step;
	float3 current = tray.point(tau_ray_length);

	for (uint32_t i = 0;; ++i) {
		const float3 tau = bssrdf.optical_depth(tau_ray_length);
		tr *= math::exp(-tau);

		// Direct light scattering
		radiance += tr * estimate_direct_light(current, intersection.prop, bssrdf,
											   ray.time, sampler_, worker);

	//	if (i >= num_samples) {
			break;
	//	}

		tray.origin = current;
		const float2 uv(rng_.random_float(), rng_.random_float());
		tray.set_direction(math::sample_sphere_uniform(uv));
		tray.min_t = ray_offset;
		tray.max_t = -std::log(rng_.random_float()) / average_scatter;

		if (!worker.intersect(intersection.prop, tray, tintersection)) {
			current = tray.point(tray.max_t);
			tau_ray_length = tray.max_t;
		} else {
			current = tintersection.geo.p;
			tau_ray_length = tray.max_t;
			break;
		}
	}

	return radiance;
}

float3 Bruteforce::li(Worker& worker, Ray& ray, Intersection& intersection,
					  Sampler_filter filter, Bxdf_result& sample_result) {
	return float3(0.f);
}

size_t Bruteforce::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

Bruteforce_factory::Bruteforce_factory(const take::Settings& take_settings,
									   uint32_t num_integrators, float step_size) :
	Factory(take_settings),
	integrators_(memory::allocate_aligned<Bruteforce>(num_integrators)) {
	settings_.step_size = step_size;
}

Bruteforce_factory::~Bruteforce_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Bruteforce_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Bruteforce(rng, take_settings_, settings_);
}


}}}}
