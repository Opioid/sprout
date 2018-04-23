#include "tracking_single.hpp"
#include "tracking.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/shape/shape.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

#include <iostream>
#include "math/print.hpp"

#include "base/debug/assert.hpp"

namespace rendering::integrator::volume {

Tracking_single::Tracking_single(rnd::Generator& rng, const take::Settings& take_settings) :
	Integrator(rng, take_settings),
	sampler_(rng) {}

void Tracking_single::prepare(const Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Tracking_single::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

static inline void max_probabilities(float mt,
									 const float3& mu_a,
									 const float3& mu_s,
									 const float3& mu_n,
									 float& pa, float& ps, float& pn,
									 float3& wa, float3& ws, float3& wn) {
	const float ma = math::max_component(mu_a);
	const float ms = math::max_component(mu_s);
	const float mn = math::max_component(mu_n);
	const float c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (mu_a / (mt * pa));
	ws = (mu_s / (mt * ps));
	wn = (mu_n / (mt * pn));
}

static inline void max_history_probabilities(float mt,
											 const float3& mu_a,
											 const float3& mu_s,
											 const float3& mu_n,
											 const float3& w,
											 float& pa, float& ps, float& pn,
											 float3& wa, float3& ws, float3& wn) {
	const float ma = math::max_component(mu_a * w);
	const float ms = math::max_component(mu_s * w);
	const float mn = math::max_component(mu_n * w);
	const float c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (mu_a / (mt * pa));
	ws = (mu_s / (mt * ps));
	wn = (mu_n / (mt * pn));
}

static inline void max_history_probabilities(float mt,
											 const float3& mu_a,
											 const float3& mu_s,
											 const float3& mu_n,
											 const float3& w,
											 float& pn, float3& wn) {
	const float ma = math::max_component(mu_a * w);
	const float ms = math::max_component(mu_s * w);
	const float mn = math::max_component(mu_n * w);
	const float c = 1.f / (ma + ms + mn);

	pn = mn * c;

	wn = (mu_n / (mt * pn));
}

static inline void avg_probabilities(float mt,
									 const float3& mu_a,
									 const float3& mu_s,
									 const float3& mu_n,
									 float& pa, float& ps, float& pn,
									 float3& wa, float3& ws, float3& wn) {
	const float ma = math::average(mu_a);
	const float ms = math::average(mu_s);
	const float mn = math::average(mu_n);
	const float c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (mu_a / (mt * pa));
	ws = (mu_s / (mt * ps));
	wn = (mu_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt,
											 const float3& mu_a,
											 const float3& mu_s,
											 const float3& mu_n,
											 const float3& w,
											 float& pa, float& ps, float& pn,
											 float3& wa, float3& ws, float3& wn) {
	const float ma = 0.f;//math::average(mu_a * w);
	const float ms = math::average(mu_s * w);
	const float mn = math::average(mu_n * w);
	const float c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (mu_a / (mt * pa));
	ws = (mu_s / (mt * ps));
	wn = (mu_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt,
											 const float3& mu_s,
											 const float3& mu_n,
											 const float3& w,
											 float& ps, float& pn,
											 float3& ws, float3& wn) {
	const float ms = math::average(mu_s * w);
	const float mn = math::average(mu_n * w);
	const float c = 1.f / (ms + mn);

	ps = ms * c;
	pn = mn * c;

	ws = (mu_s / (mt * ps));
	wn = (mu_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt,
											 const float3& mu_s,
											 const float3& mu_n,
											 const float3& w,
											 float& pn,
											 float3& wn) {
	const float ms = math::average(mu_s * w);
	const float mn = math::average(mu_n * w);
	const float c = 1.f / (ms + mn);

	pn = mn * c;

	wn = (mu_n / (mt * pn));
}

float3 Tracking_single::transmittance(const Ray& ray, Worker& worker) {
	return Tracking::transmittance(ray, rng_, worker);
}

bool Tracking_single::integrate(Ray& ray, Intersection& intersection,
								Sampler_filter filter, Worker& worker,
								float3& li, float3& transmittance) {
//	weight = float3(1.f);

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

		const float mt = material.majorant_mu_t();
		while (true) {
			const float r = rng_.random_float();
			t = t -std::log(1.f - r) / mt;
			if (t > d) {
				transmittance = w;
				li = float3(0.f);
				return true;
			}

			const float3 p = ray.point(ray.min_t + t);

			float3 mu_a, mu_s;
			material.collision_coefficients(transformation, p, filter, worker, mu_a, mu_s);

			const float3 mu_t = mu_a + mu_s;

			const float3 mu_n = float3(mt) - mu_t;

			float ps, pn;
			float3 ws, wn;
			//avg_probabilities(mt, mu_a, mu_s, mu_n, pa, ps, pn, wa, ws, wn);

			avg_history_probabilities(mt, mu_s, mu_n, w, ps, pn, ws, wn);

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
		float3 mu_a, mu_s;
		material.collision_coefficients(float2(0.f), filter, worker, mu_a, mu_s);

		const float3 extinction = mu_a + mu_s;

		const float3 scattering_albedo = mu_s / extinction;

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

size_t Tracking_single::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

float3 Tracking_single::direct_light(const Ray& ray, const float3& position, Worker& worker) {
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

float3 Tracking_single::direct_light(const Ray& ray, const float3& position,
									 const Intersection& intersection, Worker& worker) {
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

Tracking_single_factory::Tracking_single_factory(const take::Settings& take_settings,
												 uint32_t num_integrators) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Tracking_single>(num_integrators)) {}

Tracking_single_factory::~Tracking_single_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Tracking_single_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Tracking_single(rng, take_settings_);
}

}
