#include "tracking_multi.hpp"
#include "tracking.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/shape/shape.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

#include "base/debug/assert.hpp"

namespace rendering::integrator::volume {


Tracking_multi::Tracking_multi(rnd::Generator& rng, const take::Settings& take_settings) :
	Integrator(rng, take_settings) {}

void Tracking_multi::prepare(const Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Tracking_multi::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

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

float3 Tracking_multi::transmittance(const Ray& ray, Worker& worker) {
	return Tracking::transmittance(ray, rng_, worker);
}

bool Tracking_multi::integrate(Ray& ray, Intersection& intersection,
							   Sampler_filter filter, Worker& worker,
							   float3& li, float3& transmittance) {
	if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
		li = float3(0.f);
		transmittance = float3(1.f);
	//	weight = float3(1.f);
		return false;
	}

	const float d = ray.max_t - ray.min_t;

	if (d < 0.0005f) {
		li = float3(0.f);
		transmittance = float3(1.f);
	//	weight = float3(1.f);
		return true;
	}

	SOFT_ASSERT(!worker.interface_stack().empty());

	const auto interface = worker.interface_stack().top();

	const auto& material = *interface->material();

	if (!material.is_scattering_volume()) {
		// Basically the "glass" case
		const float3 mu_a = material.absorption_coefficient(interface->uv, filter, worker);

		li = float3(0.f);
		transmittance = attenuation(d, mu_a);
	//	weight = float3(1.f);
		return true;
	}

	if (material.is_heterogeneous_volume()) {
		Transformation temp;
		const auto& transformation = interface->prop->transformation_at(ray.time, temp);

		const float mt = material.majorant_mu_t();

		float3 w(1.f);

		for (float t = 0.f;;) {
			const float r0 = rng_.random_float();
			t = t -std::log(1.f - r0) / mt;
			if (t > d) {
				li = float3(0.f);
				transmittance = w;
			//	weight = float3(1.f);
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

			const float r1 = rng_.random_float();
			if (r1 <= 1.f - pn && ps > 0.f) {
				SOFT_ASSERT(math::all_finite(ws));

				intersection.prop = interface->prop;
				intersection.geo.p = p;
				intersection.geo.uv = interface->uv;
				intersection.geo.part = interface->part;
				intersection.geo.subsurface = true;

				li = float3(0.f);
			//	transmittance = float3(1.f);
			//	weight = w * ws;
				transmittance = w * ws;
				return true;
			} else {
				SOFT_ASSERT(math::all_finite(wn));

				w *= wn;
			}
		}
	} else {
		float3 mu_a, mu_s;
		material.collision_coefficients(interface->uv, filter, worker, mu_a, mu_s);

		const float3 mu_t = mu_a + mu_s;

		const float mt = math::max_component(mu_t);

		const float3 mu_n = float3(mt) - mu_t;

		float3 w(1.f);

		for (float t = 0.f;;) {
			const float r0 = rng_.random_float();
			t = t -std::log(1.f - r0) / mt;
			if (t > d) {
				li = float3(0.f);
				transmittance = w;
			//	weight = float3(1.f);
				return true;
			}

			float ps, pn;
			float3 ws, wn;
			//avg_probabilities(mt, mu_a, mu_s, mu_n, pa, ps, pn, wa, ws, wn);

			avg_history_probabilities(mt, mu_s, mu_n, w, ps, pn, ws, wn);

			const float r1 = rng_.random_float();
			if (r1 <= 1.f - pn && ps > 0.f) {
				intersection.prop = interface->prop;
				intersection.geo.p = ray.point(ray.min_t + t);
				intersection.geo.uv = interface->uv;
				intersection.geo.part = interface->part;
				intersection.geo.subsurface = true;

				li = float3(0.f);
			//	transmittance = float3(1.f);
			//	weight = w * ws;
				transmittance = w * ws;
				return true;
			} else {
				SOFT_ASSERT(math::all_finite(wn));

				w *= wn;
			}
		}
	}

	return true;
}

size_t Tracking_multi::num_bytes() const {
	return sizeof(*this);
}

Tracking_multi_factory::Tracking_multi_factory(const take::Settings& take_settings,
											   uint32_t num_integrators) :
	Factory(take_settings, num_integrators),
	integrators_(memory::allocate_aligned<Tracking_multi>(num_integrators)) {}

Tracking_multi_factory::~Tracking_multi_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Tracking_multi_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Tracking_multi(rng, take_settings_);
}

}
