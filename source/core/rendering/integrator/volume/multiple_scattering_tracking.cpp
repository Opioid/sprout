#include "multiple_scattering_tracking.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/shape/shape.hpp"
#include "scene/volume/volume.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

#include <iostream>
#include "math/print.hpp"

#include "base/debug/assert.hpp"

namespace rendering::integrator::volume {

enum class Algorithm {
	Tracking,
	Delta_tracking,
	Experiment
};

Multiple_scattering_tracking::Multiple_scattering_tracking(rnd::Generator& rng, const take::Settings& take_settings,
									 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Multiple_scattering_tracking::prepare(const Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Multiple_scattering_tracking::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

static inline void max_probabilities(float mt,
									 const float3& sigma_a,
									 const float3& sigma_s,
									 const float3& sigma_n,
									 float& pa, float& ps, float& pn,
									 float3& wa, float3& ws, float3& wn) {
	const float ma = math::max_component(sigma_a);
	const float ms = math::max_component(sigma_s);
	const float mn = math::max_component(sigma_n);
	const float c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (sigma_a / (mt * pa));
	ws = (sigma_s / (mt * ps));
	wn = (sigma_n / (mt * pn));
}

static inline void max_history_probabilities(float mt,
											 const float3& sigma_a,
											 const float3& sigma_s,
											 const float3& sigma_n,
											 const float3& w,
											 float& pa, float& ps, float& pn,
											 float3& wa, float3& ws, float3& wn) {
	const float ma = math::max_component(sigma_a * w);
	const float ms = math::max_component(sigma_s * w);
	const float mn = math::max_component(sigma_n * w);
	const float c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (sigma_a / (mt * pa));
	ws = (sigma_s / (mt * ps));
	wn = (sigma_n / (mt * pn));
}

static inline void max_history_probabilities(float mt,
											 const float3& sigma_a,
											 const float3& sigma_s,
											 const float3& sigma_n,
											 const float3& w,
											 float& pn, float3& wn) {
	const float ma = math::max_component(sigma_a * w);
	const float ms = math::max_component(sigma_s * w);
	const float mn = math::max_component(sigma_n * w);
	const float c = 1.f / (ma + ms + mn);

	pn = mn * c;

	wn = (sigma_n / (mt * pn));
}

static inline void avg_probabilities(float mt,
									 const float3& sigma_a,
									 const float3& sigma_s,
									 const float3& sigma_n,
									 float& pa, float& ps, float& pn,
									 float3& wa, float3& ws, float3& wn) {
	const float ma = math::average(sigma_a);
	const float ms = math::average(sigma_s);
	const float mn = math::average(sigma_n);
	const float c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (sigma_a / (mt * pa));
	ws = (sigma_s / (mt * ps));
	wn = (sigma_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt,
											 const float3& sigma_a,
											 const float3& sigma_s,
											 const float3& sigma_n,
											 const float3& w,
											 float& pa, float& ps, float& pn,
											 float3& wa, float3& ws, float3& wn) {
	const float ma = 0.f;//math::average(sigma_a * w);
	const float ms = math::average(sigma_s * w);
	const float mn = math::average(sigma_n * w);
	const float c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (sigma_a / (mt * pa));
	ws = (sigma_s / (mt * ps));
	wn = (sigma_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt,
											 const float3& sigma_s,
											 const float3& sigma_n,
											 const float3& w,
											 float& ps, float& pn,
											 float3& ws, float3& wn) {
	const float ms = math::average(sigma_s * w);
	const float mn = math::average(sigma_n * w);
	const float c = 1.f / (ms + mn);

	ps = ms * c;
	pn = mn * c;

	ws = (sigma_s / (mt * ps));
	wn = (sigma_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt,
											 const float3& sigma_s,
											 const float3& sigma_n,
											 const float3& w,
											 float& pn,
											 float3& wn) {
	const float ms = math::average(sigma_s * w);
	const float mn = math::average(sigma_n * w);
	const float c = 1.f / (ms + mn);

	pn = mn * c;

	wn = (sigma_n / (mt * pn));
}

float3 Multiple_scattering_tracking::transmittance(const Ray& ray, const Volume& volume,
												 const Worker& worker) {
	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const auto& material = *volume.material(0);

	const bool use_heterogeneous_algorithm = material.is_heterogeneous_volume();

	if (use_heterogeneous_algorithm) {
/*		const float d = ray.max_t - ray.min_t;
		const float max_extinction = math::average(material.max_extinction());
		bool terminated = false;
		float t = 0.f;

		do {
			const float r = rng_.random_float();
			t = t -std::log(1.f - r) / max_extinction;
			if (t > d) {
				break;
			}

			const float3 p = ray.point(ray.min_t + t);

			const float3 sigma_a = material.absorption(transformation, p,
													   Sampler_filter::Undefined, worker);

			const float3 sigma_s = material.scattering(transformation, p,
													   Sampler_filter::Undefined, worker);

			const float3 extinction = sigma_a + sigma_s;

			const float r2 = rng_.random_float();
			if (r2 < math::average(extinction) / max_extinction) {
				terminated = true;
			}
		} while (!terminated);

		if (terminated) {
			return float3(0.f);
		} else {
			return float3(1.f);
		}
		*/

		const float d = ray.max_t - ray.min_t;

		float3 w(1.f);
		float t = 0.f;

		const float mt = math::max_component(material.max_extinction(float2(0.f),
																	 Sampler_filter::Undefined,
																	 worker));
		for (;;) {
			const float r = rng_.random_float();
			t = t -std::log(1.f - r) / mt;
			if (t > d) {
				return w;
			}

			const float3 p = ray.point(ray.min_t + t);

			const float3 sigma_a = material.absorption(transformation, p, float2(0.f),
													   Sampler_filter::Undefined, worker);

			const float3 sigma_s = material.scattering(transformation, p, float2(0.f),
													   Sampler_filter::Undefined, worker);

			const float3 sigma_t = sigma_a + sigma_s;

			const float3 sigma_n = float3(mt) - sigma_t;

			float pn;
			float3 wn;
			//avg_probabilities(mt, sigma_a, sigma_s, sigma_n, pa, ps, pn, wa, ws, wn);

			avg_history_probabilities(mt, sigma_s, sigma_n, w, pn, wn);

			const float r2 = rng_.random_float();
			if (r2 < 1.f - pn) {
				return float3(0.f);
			} else {
				SOFT_ASSERT(math::all_finite(wn));
				w *= wn;
			}
		}
	}

	const float3 tau = material.optical_depth(transformation, volume.aabb(), ray,
											  1.f, rng_,
											  Sampler_filter::Nearest, worker);
	return math::exp(-tau);
}

float3 Multiple_scattering_tracking::li(const Ray& ray, const Volume& volume,
									  Worker& worker, float3& transmittance) {
	std::cout << "not implemented" << std::endl;
	transmittance = float3(1.f);
	return float3(0.f);
}

float3 Multiple_scattering_tracking::transmittance(const Ray& ray, const Intersection& intersection,
												 const Worker& worker) {
	const auto& prop = *intersection.prop;

	Transformation temp;
	const auto& transformation = prop.transformation_at(ray.time, temp);

	const auto& material = *intersection.material();

	if (material.is_heterogeneous_volume()) {
		const float d = ray.max_t - ray.min_t;

		const float3 me = material.max_extinction(intersection.geo.uv, Sampler_filter::Nearest,
												  worker);
		const float mt = math::average(me);
		bool terminated = false;
		float t = 0.f;

		do {
			const float r0 = rng_.random_float();
			t = t -std::log(1.f - r0) / mt;
			if (t > d) {
				break;
			}

			const float3 p = ray.point(ray.min_t + t);

			float3 sigma_a;
			float3 sigma_s;
			material.extinction(transformation, p, intersection.geo.uv,
								Sampler_filter::Nearest, worker, sigma_a, sigma_s);

			const float3 extinction = sigma_a + sigma_s;

			const float r1 = rng_.random_float();
			if (r1 < math::average(extinction) / mt) {
				terminated = true;
			}
		} while (!terminated);

		if (terminated) {
			return float3(0.f);
		} else {
			return float3(1.f);
		}
	}

	const float3 tau = material.optical_depth(transformation, prop.aabb(), ray,
											  1.f, rng_,
											  Sampler_filter::Nearest, worker);
	return math::exp(-tau);
}

bool Multiple_scattering_tracking::integrate(Ray& ray, Intersection& intersection, Worker& worker,
										   float3& li, float3& transmittance, float3& weight) {
	const float2 initial_uv = intersection.geo.uv;

	weight = float3(1.f);

	Transformation temp;
	const auto& transformation = intersection.prop->transformation_at(ray.time, temp);

	// We rely on the material stack being not empty
	const auto& material = *worker.material_stack().top();

	if (!worker.intersect_and_resolve_mask(ray, intersection, Sampler_filter::Nearest)) {
		li = float3(0.f);
		transmittance = float3(1.f);
		return false;
	}

	const float d = ray.max_t;

	if (!material.is_scattering_volume()) {
		// Basically the "glass" case
		const float3 sigma_a = material.absorption(transformation, float3(0.f), initial_uv,
												   Sampler_filter::Nearest, worker);

		li = float3(0.f);
		transmittance = attenuation(d, sigma_a);
		return true;
	}

	constexpr bool use_heterogeneous_algorithm = true;

	if (use_heterogeneous_algorithm) {
		float3 w(1.f);
		float t = 0.f;

		const float3 me = material.max_extinction(initial_uv, Sampler_filter::Undefined, worker);
		const float mt = math::max_component(me);

		while (true) {
			const float r = rng_.random_float();
			t = t -std::log(1.f - r) / mt;
			if (t > d) {
				transmittance = w;
				li = float3(0.f);
				return true;
			}

			const float3 p = ray.point(t);

			float3 sigma_a;
			float3 sigma_s;
			material.extinction(transformation, p, initial_uv, Sampler_filter::Undefined, worker,
								sigma_a, sigma_s);

			const float3 sigma_t = sigma_a + sigma_s;

			const float3 sigma_n = float3(mt) - sigma_t;

			float ps, pn;
			float3 ws, wn;
			//avg_probabilities(mt, sigma_a, sigma_s, sigma_n, pa, ps, pn, wa, ws, wn);

			avg_history_probabilities(mt, sigma_s, sigma_n, w, ps, pn, ws, wn);

			const float r2 = rng_.random_float();
			if (r2 <= 1.f - pn) {
				intersection.geo.p = p;
				intersection.geo.uv = initial_uv;
				intersection.geo.epsilon = 0.f;
				intersection.geo.subsurface = true;

				transmittance = float3(1.f);
				weight = w * ws;

				li = float3(0.f);

				return true;
			} else {
				SOFT_ASSERT(math::all_finite(wn));

				w *= wn;
			}
		}
	} else {
		li = float3(0.f);
		transmittance = float3(1.f);
		weight = float3(1.f);
		std::cout << "not implemented" << std::endl;
	}

	return true;
}

size_t Multiple_scattering_tracking::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

Multiple_scattering_tracking_factory::Multiple_scattering_tracking_factory(const take::Settings& take_settings,
													 uint32_t num_integrators) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Multiple_scattering_tracking>(num_integrators)),
	settings_{} {}

Multiple_scattering_tracking_factory::~Multiple_scattering_tracking_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Multiple_scattering_tracking_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Multiple_scattering_tracking(rng, take_settings_, settings_);
}

}
