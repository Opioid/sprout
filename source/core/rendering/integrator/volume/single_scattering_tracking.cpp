#include "single_scattering_tracking.hpp"
#include "tracking.hpp"
#include "rendering/rendering_worker.hpp"
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

Single_scattering_tracking::Single_scattering_tracking(rnd::Generator& rng, const take::Settings& take_settings,
									 const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Single_scattering_tracking::prepare(const Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Single_scattering_tracking::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

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

float3 Single_scattering_tracking::transmittance(const Ray& ray, const Volume& volume,
												 const Worker& worker) {
	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const auto& material = *volume.material(0);

	const bool use_heterogeneous_algorithm = material.is_heterogeneous_volume();

	if (use_heterogeneous_algorithm) {
/*		const float d = ray.max_t - ray.min_t;
		const float max_extinction = math::average(material.majorant_sigma_t());
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

		const float mt = material.majorant_sigma_t();
		for (;;) {
			const float r = rng_.random_float();
			t = t -std::log(1.f - r) / mt;
			if (t > d) {
				return w;
			}

			const float3 p = ray.point(ray.min_t + t);

			float3 sigma_a, sigma_s;
			material.extinction(transformation, p,
								Sampler_filter::Undefined, worker, sigma_a, sigma_s);

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
											  settings_.step_size, rng_,
											  Sampler_filter::Nearest, worker);
	return math::exp(-tau);
}

float3 Single_scattering_tracking::li(const Ray& ray, const Volume& volume,
									  Worker& worker, float3& transmittance) {
	enum class Homogeneous_algorithm {
		Closed_form,
		PBRT3,
		Stuff
	};

	enum class Heterogeneous_algorithm {
		Delta_tracking,
		Spectral_tracking
	};

	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const auto& material = *volume.material(0);

	const bool use_heterogeneous_algorithm = true;//material.is_heterogeneous_volume();

	if (use_heterogeneous_algorithm) {
		constexpr Heterogeneous_algorithm algorithm = Heterogeneous_algorithm::Spectral_tracking;

		if (Heterogeneous_algorithm::Spectral_tracking == algorithm) {
			const float d = ray.max_t - ray.min_t;

			float3 w(1.f);
			float t = 0.f;

			const float mt = material.majorant_sigma_t();
			while (true) {
				const float r = rng_.random_float();
				t = t -std::log(1.f - r) / mt;
				if (t > d) {
					transmittance = w;
					return float3(0.f);
				}

				const float3 p = ray.point(ray.min_t + t);

				float3 sigma_a, sigma_s;
				material.extinction(transformation, p,
									Sampler_filter::Undefined, worker, sigma_a, sigma_s);

				const float3 sigma_t = sigma_a + sigma_s;

				const float3 sigma_n = float3(mt) - sigma_t;

				float ps, pn;
				float3 ws, wn;
				//avg_probabilities(mt, sigma_a, sigma_s, sigma_n, pa, ps, pn, wa, ws, wn);

				avg_history_probabilities(mt, sigma_s, sigma_n, w, ps, pn, ws, wn);

				const float r2 = rng_.random_float();
				if (r2 < 1.f - pn) {
					transmittance = float3(0.f);
					SOFT_ASSERT(math::all_finite(ws));
					return w * ws * direct_light(ray, p, worker);
				} else {
					SOFT_ASSERT(math::all_finite(wn));
					w *= wn;
				}
			}
		} else if (Heterogeneous_algorithm::Delta_tracking == algorithm) {
			const float d = ray.max_t - ray.min_t;

			const float max_extinction = material.majorant_sigma_t();
			bool terminated = false;
			float t = 0.f;

			float3 p;
			float3 extinction;
			float3 scattering_albedo;

			do {
				const float r = rng_.random_float();
				t = t -std::log(1.f - r) / max_extinction;
				if (t > d) {
					break;
				}

				p = ray.point(ray.min_t + t);

				float3 sigma_a, sigma_s;
				material.extinction(transformation, p,
									Sampler_filter::Undefined, worker, sigma_a, sigma_s);

				extinction = sigma_a + sigma_s;
				const float r2 = rng_.random_float();
				if (r2 < math::average(extinction) / max_extinction) {
					terminated = true;

					scattering_albedo = sigma_s / extinction;
				}
			} while (!terminated);

			if (terminated) {
				float3 l = direct_light(ray, p, worker);

				l *= scattering_albedo;

				transmittance = float3(0.f);
				return l;
			} else {
				transmittance = float3(1.f);
				return float3(0.f);
			}
		}
	} else {
		constexpr Homogeneous_algorithm algorithm = Homogeneous_algorithm::Stuff;

		if (Homogeneous_algorithm::Closed_form == algorithm) {
			const float d = ray.max_t - ray.min_t;

			float3 sigma_a, sigma_s;
			material.extinction(float2(0.f),
								Sampler_filter::Undefined, worker, sigma_a, sigma_s);

			const float3 extinction = sigma_a + sigma_s;

			const float3 scattering_albedo = sigma_s / extinction;

			const float3 tr = math::exp(-d * extinction);

			const float r = rng_.random_float();
			const float scatter_distance = -std::log(1.f - r * (1.f - math::average(tr))) / math::average(extinction);

			const float3 p = ray.point(ray.min_t + scatter_distance);

			const float3 l = direct_light(ray, p, worker);

			transmittance = tr;
			return l * (1.f - tr) * scattering_albedo;
		} else if (Homogeneous_algorithm::PBRT3 == algorithm) {
			const float d = ray.max_t - ray.min_t;

			const float r = rng_.random_float();

			const uint32_t channel = std::min(static_cast<uint32_t>(r * 3.f), 2u);

//			Float dist = -std::log(1 - sampler.Get1D()) / sigma_t[channel];
//			Float t = std::min(dist / ray.d.Length(), ray.tMax);
//			bool sampledMedium = t < ray.tMax;
//			if (sampledMedium)
//				*mi = MediumInteraction(ray(t), -ray.d, ray.time, this,
//										ARENA_ALLOC(arena, HenyeyGreenstein)(g));

//			// Compute the transmittance and sampling density
//			Spectrum Tr = Exp(-sigma_t * std::min(t, MaxFloat) * ray.d.Length());

//			// Return weighting factor for scattering from homogeneous medium
//			Spectrum density = sampledMedium ? (sigma_t * Tr) : Tr;
//			Float pdf = 0;
//			for (int i = 0; i < Spectrum::nSamples; ++i) pdf += density[i];
//			pdf *= 1 / (Float)Spectrum::nSamples;
//			if (pdf == 0) {
//				CHECK(Tr.IsBlack());
//				pdf = 1;
//			}
//			return sampledMedium ? (Tr * sigma_s / pdf) : (Tr / pdf);

			float3 sigma_a, sigma_s;
			material.extinction(float2(0.f),
								Sampler_filter::Undefined, worker, sigma_a, sigma_s);

			const float3 sigma_t = sigma_a + sigma_s;

			const float r2 = rng_.random_float();
			const float dist = -std::log(1.f - r2) / sigma_t[channel];

			const float t = std::min(dist, d);

			const bool scatter = t < d;

			float3 l;
			if (scatter) {
				const float3 p = ray.point(ray.min_t + t);

				l = direct_light(ray, p, worker);
			}

			const float3 tr = math::exp(-t * sigma_t);

			const float3 density = scatter ? sigma_t * tr : tr;

			const float pdf = math::average(density);

			transmittance = scatter ? ((tr * sigma_s) / pdf) : tr / pdf;

			const float3 scattering_albedo = sigma_s / sigma_t;

			return l * (1.f - transmittance) * scattering_albedo;
		} else {
			const float d = ray.max_t - ray.min_t;

			float3 sigma_a, sigma_s;
			material.extinction(float2(0.f),
								Sampler_filter::Undefined, worker, sigma_a, sigma_s);

			const float3 sigma_t = sigma_a + sigma_s;

			const float3 scattering_albedo = sigma_s / sigma_t;

			const float3 tr = math::exp(-d * sigma_t);

			const float r = rng_.random_float();

			const float max_tr = math::max_component(tr);
			const float mt = math::max_component(sigma_t);

			const float3 sigma_n = float3(mt) - sigma_t;

			float ps, pn;
			float3 ws, wn;
			//avg_probabilities(mt, sigma_a, sigma_s, sigma_n, pa, ps, pn, wa, ws, wn);

			float3 w(1.f);
			avg_history_probabilities(mt, sigma_s, sigma_n, w, ps, pn, ws, wn);


			const float scatter_distance = -std::log(1.f - r * (1.f - max_tr)) / mt;

			const float3 p = ray.point(ray.min_t + scatter_distance);

			const float3 l = direct_light(ray, p, worker);

			transmittance = tr;
			return l * (1.f - tr) * ws * scattering_albedo;
		}
	}
}

float3 Single_scattering_tracking::transmittance(const Ray& ray, const Worker& worker) {
	return Tracking::transmittance(ray, rng_, worker);
}

bool Single_scattering_tracking::integrate(Ray& ray, Intersection& intersection,
										   Sampler_filter filter, Worker& worker,
										   float3& li, float3& transmittance, float3& weight) {
	weight = float3(1.f);

	Transformation temp;
	const auto& transformation = intersection.prop->transformation_at(ray.time, temp);

	const auto& material = *intersection.material();

	const bool hit = worker.intersect_and_resolve_mask(ray, intersection, filter);
	if (!hit) {
		li = float3(0.f);
		transmittance = float3(1.f);
		return false;
	}

	const float d = ray.max_t;

	constexpr bool use_heterogeneous_algorithm = true;

	if (use_heterogeneous_algorithm) {
		float3 w(1.f);
		float t = 0.f;

		const float mt = material.majorant_sigma_t();
		while (true) {
			const float r = rng_.random_float();
			t = t -std::log(1.f - r) / mt;
			if (t > d) {
				transmittance = w;
				li = float3(0.f);
				return true;
			}

			const float3 p = ray.point(ray.min_t + t);

			float3 sigma_a, sigma_s;
			material.extinction(transformation, p, filter, worker, sigma_a, sigma_s);

			const float3 sigma_t = sigma_a + sigma_s;

			const float3 sigma_n = float3(mt) - sigma_t;

			float ps, pn;
			float3 ws, wn;
			//avg_probabilities(mt, sigma_a, sigma_s, sigma_n, pa, ps, pn, wa, ws, wn);

			avg_history_probabilities(mt, sigma_s, sigma_n, w, ps, pn, ws, wn);

			const float r2 = rng_.random_float();
			if (r2 <= 1.f - pn) {
				transmittance = float3(0.f);
				SOFT_ASSERT(math::all_finite(ws));
				li = w * ws * direct_light(ray, p, intersection, worker);
				return true;
			} else {
				SOFT_ASSERT(math::all_finite(wn));

//				if (!math::all_finite(wn)) {
//					std::cout << "problem" << std::endl;
//				}

				w *= wn;

				if (math::average(w) == 0.f) {
					transmittance = w;
					li = float3(0.f);
					return true;
				}
			}
		}
	} else {
		float3 sigma_a, sigma_s;
		material.extinction(float2(0.f), filter, worker, sigma_a, sigma_s);

		const float3 extinction = sigma_a + sigma_s;

		const float3 scattering_albedo = sigma_s / extinction;

		transmittance = math::exp(-d * extinction);

		const float r = rng_.random_float();
		const float scatter_distance = -std::log(1.f - r * (1.f - math::average(transmittance))) / math::average(extinction);

		const float3 p = ray.point(scatter_distance);

		float3 l = direct_light(ray, p, intersection, worker);

		l *= (1.f - transmittance) * scattering_albedo;

		li = l;
	}

	return true;
}

size_t Single_scattering_tracking::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

float3 Single_scattering_tracking::direct_light(const Ray& ray, const float3& position,
												Worker& worker) {
	float3 result = float3::identity();

	Ray shadow_ray;
	shadow_ray.origin = position;
	shadow_ray.min_t  = 0.f;
	shadow_ray.depth  = ray.depth + 1;
	shadow_ray.time   = ray.time;

	const auto light = worker.scene().random_light(rng_.random_float());

	scene::light::Sample light_sample;
	if (light.ref.sample(position, float3(0.f, 0.f, 1.f), ray.time,
						 true, sampler_, 0, Sampler_filter::Nearest, worker, light_sample)) {
		shadow_ray.set_direction(light_sample.shape.wi);
		const float offset = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
		shadow_ray.max_t = light_sample.shape.t - offset;

		const float3 tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);
		if (math::any_greater_zero(tv)) {
			const float3 tr = worker.transmittance(shadow_ray);

			const float phase = 1.f / (4.f * math::Pi);

			result += (tv * tr) * (phase * light_sample.radiance)
					/ (light.pdf * light_sample.shape.pdf);
		}
	}

	return result;
}

float3 Single_scattering_tracking::direct_light(const Ray& ray, const float3& position,
												const Intersection& intersection,
												Worker& worker) {
	float3 result = float3::identity();

	Ray shadow_ray;
	shadow_ray.origin = position;
	shadow_ray.min_t  = 0.f;
	shadow_ray.depth  = ray.depth + 1;
	shadow_ray.time   = ray.time;

	const auto light = worker.scene().random_light(rng_.random_float());

	scene::light::Sample light_sample;
	if (light.ref.sample(position, ray.time, sampler_, 0,
						 Sampler_filter::Nearest, worker, light_sample)) {
		shadow_ray.set_direction(light_sample.shape.wi);
		const float offset = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
		shadow_ray.max_t = light_sample.shape.t - offset;

	//	const float3 tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);

		Intersection tintersection = intersection;
		tintersection.geo.subsurface = true;
		const float3 tv = worker.tinted_visibility(shadow_ray, tintersection,
												   Sampler_filter::Nearest);
		if (math::any_greater_zero(tv)) {
			const float3 tr = worker.transmittance(shadow_ray);

			const float phase = 1.f / (4.f * math::Pi);

			result += (tv * tr) * (phase * light_sample.radiance)
					/ (light.pdf * light_sample.shape.pdf);
		}
	}

	return result;
}

Single_scattering_tracking_factory::Single_scattering_tracking_factory(const take::Settings& take_settings,
													 uint32_t num_integrators, float step_size, 
													 float step_probability,
													 bool indirect_lighting) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Single_scattering_tracking>(num_integrators)),
	settings_{step_size, step_probability, !indirect_lighting} {}

Single_scattering_tracking_factory::~Single_scattering_tracking_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Single_scattering_tracking_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Single_scattering_tracking(rng, take_settings_, settings_);
}

}
