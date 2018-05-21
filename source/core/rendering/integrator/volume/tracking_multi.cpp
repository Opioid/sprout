#include "tracking_multi.hpp"
#include "tracking.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/entity/composed_transformation.inl"
#include "scene/material/volumetric/volumetric_octree.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/shape/shape.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

#include "base/debug/assert.hpp"

namespace rendering::integrator::volume {

Tracking_multi::Tracking_multi(rnd::Generator& rng, take::Settings const& take_settings) :
	Integrator(rng, take_settings) {}

void Tracking_multi::prepare(Scene const& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Tracking_multi::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}
/*
static inline void max_probabilities(float mt,
									 float3 const& mu_a,
									 float3 const& mu_s,
									 float3 const& mu_n,
									 float& pa, float& ps, float& pn,
									 float3& wa, float3& ws, float3& wn) {
	float const ma = math::max_component(mu_a);
	float const ms = math::max_component(mu_s);
	float const mn = math::max_component(mu_n);
	float const c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (mu_a / (mt * pa));
	ws = (mu_s / (mt * ps));
	wn = (mu_n / (mt * pn));
}

static inline void max_history_probabilities(float mt,
											 float3 const& mu_a,
											 float3 const& mu_s,
											 float3 const& mu_n,
											 float3 const& w,
											 float& pa, float& ps, float& pn,
											 float3& wa, float3& ws, float3& wn) {
	float const ma = math::max_component(mu_a * w);
	float const ms = math::max_component(mu_s * w);
	float const mn = math::max_component(mu_n * w);
	float const c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (mu_a / (mt * pa));
	ws = (mu_s / (mt * ps));
	wn = (mu_n / (mt * pn));
}

static inline void max_history_probabilities(float mt,
											 float3 const& mu_a,
											 float3 const& mu_s,
											 float3 const& mu_n,
											 float3 const& w,
											 float& pn, float3& wn) {
	float const ma = math::max_component(mu_a * w);
	float const ms = math::max_component(mu_s * w);
	float const mn = math::max_component(mu_n * w);
	float const c = 1.f / (ma + ms + mn);

	pn = mn * c;

	wn = (mu_n / (mt * pn));
}

static inline void avg_probabilities(float mt,
									 float3 const& mu_a,
									 float3 const& mu_s,
									 float3 const& mu_n,
									 float& pa, float& ps, float& pn,
									 float3& wa, float3& ws, float3& wn) {
	float const ma = math::average(mu_a);
	float const ms = math::average(mu_s);
	float const mn = math::average(mu_n);
	float const c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (mu_a / (mt * pa));
	ws = (mu_s / (mt * ps));
	wn = (mu_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt,
											 float3 const& mu_a,
											 float3 const& mu_s,
											 float3 const& mu_n,
											 float3 const& w,
											 float& pa, float& ps, float& pn,
											 float3& wa, float3& ws, float3& wn) {
	float const ma = 0.f;//math::average(mu_a * w);
	float const ms = math::average(mu_s * w);
	float const mn = math::average(mu_n * w);
	float const c = 1.f / (ma + ms + mn);

	pa = ma * c;
	ps = ms * c;
	pn = mn * c;

	wa = (mu_a / (mt * pa));
	ws = (mu_s / (mt * ps));
	wn = (mu_n / (mt * pn));
}

static inline void avg_history_probabilities(float mt,
											 float3 const& mu_s,
											 float3 const& mu_n,
											 float3 const& w,
											 float& ps, float& pn,
											 float3& ws, float3& wn) {
	float const ms = math::average(mu_s * w);
	float const mn = math::average(mu_n * w);
	float const c = 1.f / (ms + mn);

	ps = ms * c;
	pn = mn * c;

	ws = (mu_s / (mt * ps));
	wn = (mu_n / (mt * pn));
}
*/
float3 Tracking_multi::transmittance(Ray const& ray, Worker& worker) {
	return Tracking::transmittance(ray, rng_, worker);
}

bool Tracking_multi::integrate(Ray& ray, Intersection& intersection, Sampler_filter filter,
							   Worker& worker, float3& li, float3& transmittance) {
	if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
		li = float3(0.f);
		transmittance = float3(1.f);
	//	weight = float3(1.f);
		return false;
	}

	float const d = ray.max_t;

	if (d - ray.min_t < 0.0005f) {
		li = float3(0.f);
		transmittance = float3(1.f);
	//	weight = float3(1.f);
		return true;
	}

	SOFT_ASSERT(!worker.interface_stack().empty());

	auto const interface = worker.interface_stack().top();

	auto const& material = *interface->material();

	if (!material.is_scattering_volume()) {
		// Basically the "glass" case
		float3 const mu_a = material.absorption_coefficient(interface->uv, filter, worker);

		li = float3(0.f);
		transmittance = attenuation(d, mu_a);
	//	weight = float3(1.f);
		return true;
	}

	if (material.is_heterogeneous_volume()) {
		Transformation temp;
		auto const& transformation = interface->prop->transformation_at(ray.time, temp);

		float3 const local_origin = transformation.world_to_object_point(ray.origin);
		float3 const local_dir    = transformation.world_to_object_vector(ray.direction);

		if (auto const tree = material.volume_octree(); tree) {
			math::Ray local_ray(local_origin, local_dir, ray.min_t, ray.max_t);

			float3 w(1.f);
			for (; local_ray.min_t < d;) {
				if (float mt; tree->intersect_f(local_ray, mt)) {
					if (float t; Tracking::track(local_ray, mt, material, filter,
												 rng_, worker, t, w)) {
						intersection.prop = interface->prop;
						intersection.geo.p = ray.point(t);
						intersection.geo.uv = interface->uv;
						intersection.geo.part = interface->part;
						intersection.geo.subsurface = true;

						li = float3(0.f);
						transmittance = w;
						return true;
					}
				}

				local_ray.min_t = local_ray.max_t + 0.00001f;
				local_ray.max_t = d;
			}
			
		/*	for (; local_ray.min_t < d;) {
				float mt;
				if (!tree->intersect_f(local_ray, mt)) {
					break;
				}

				if (float t; Tracking::track(local_ray, mt, material, filter,
					rng_, worker, t, w)) {
					intersection.prop = interface->prop;
					intersection.geo.p = ray.point(t);
					intersection.geo.uv = interface->uv;
					intersection.geo.part = interface->part;
					intersection.geo.subsurface = true;

					li = float3(0.f);
					transmittance = w;
					return true;
				}

				local_ray.min_t = local_ray.max_t + 0.00001f;
				local_ray.max_t = d;
			}*/
			
			li = float3(0.f);
			transmittance = w;
			return true;
		}

		float const mt = material.majorant_mu_t();

		float3 w(1.f);

		uint32_t i = Tracking::max_iterations_;
		for (float t = ray.min_t;; --i) {
		//	SOFT_ASSERT(i < 1024*1024);

			float const r0 = rng_.random_float();
			t = t -std::log(1.f - r0) / mt;
			if (t > d || 0 == i) {
				li = float3(0.f);
				transmittance = w;
			//	weight = float3(1.f);
				return true;
			}

			float3 const local_p = local_origin + t * local_dir;

			auto const mu = material.collision_coefficients(local_p, filter, worker);

			float3 const mu_t = mu.a + mu.s;

			float3 const mu_n = float3(mt) - mu_t;

//			float ps, pn;
//			float3 ws, wn;
//			avg_history_probabilities(mt, mu_s, mu_n, w, ps, pn, ws, wn);

			float const ms = math::average(mu.s * w);
			float const mn = math::average(mu_n * w);
			float const c = 1.f / (ms + mn);

			float const ps = ms * c;
			float const pn = mn * c;

			float const r1 = rng_.random_float();
			if (r1 <= 1.f - pn && ps > 0.f) {
				intersection.prop = interface->prop;
				intersection.geo.p = ray.point(t);
				intersection.geo.uv = interface->uv;
				intersection.geo.part = interface->part;
				intersection.geo.subsurface = true;

				float3 const ws = mu.s / (mt * ps);

				SOFT_ASSERT(math::all_finite(ws));

				li = float3(0.f);
			//	transmittance = float3(1.f);
			//	weight = w * ws;
				transmittance = w * ws;
				return true;
			} else {
				float3 const wn = mu_n / (mt * pn);

				SOFT_ASSERT(math::all_finite(wn));

				w *= wn;
			}
		}
	} else {
		auto const mu = material.collision_coefficients(interface->uv, filter, worker);

		float3 const mu_t = mu.a + mu.s;

		float const mt  = math::max_component(mu_t);
		float const imt = 1.f / mt;

		float3 const mu_n = float3(mt) - mu_t;

		float3 w(1.f);

		for (float t = ray.min_t;;) {
			float const r0 = rng_.random_float();
			t = t -std::log(1.f - r0) * imt;
			if (t > d) {
				li = float3(0.f);
				transmittance = w;
			//	weight = float3(1.f);
				return true;
			}

	//		float ps, pn;
	//		float3 ws, wn;
	//		avg_history_probabilities(mt, mu_s, mu_n, w, ps, pn, ws, wn);

			float const ms = math::average(mu.s * w);
			float const mn = math::average(mu_n * w);
			float const c = 1.f / (ms + mn);

			float const ps = ms * c;
			float const pn = mn * c;

			float const r1 = rng_.random_float();
			if (r1 <= 1.f - pn && ps > 0.f) {
				intersection.prop = interface->prop;
				intersection.geo.p = ray.point(t);
				intersection.geo.uv = interface->uv;
				intersection.geo.part = interface->part;
				intersection.geo.subsurface = true;

				float3 const ws = mu.s / (mt * ps);

				li = float3(0.f);
			//	transmittance = float3(1.f);
			//	weight = w * ws;
				transmittance = w * ws;
				return true;
			} else {
				float3 const wn = mu_n / (mt * pn);

				SOFT_ASSERT(math::all_finite(wn));

				w *= wn;
			}
		}
	}
}

size_t Tracking_multi::num_bytes() const {
	return sizeof(*this);
}

Tracking_multi_factory::Tracking_multi_factory(take::Settings const& take_settings,
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
