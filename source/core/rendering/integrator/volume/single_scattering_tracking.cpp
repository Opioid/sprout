#include "single_scattering_tracking.hpp"
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
#include "base/spectrum/rgb.hpp"

#include <iostream>
#include "math/print.hpp"

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

float3 Single_scattering_tracking::transmittance(const Ray& ray, const Volume& volume,
												 const Worker& worker) {
	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const auto& material = *volume.material(0);

	const float3 tau = material.optical_depth(transformation, volume.aabb(), ray,
											  settings_.step_size, rng_,
											  Sampler_filter::Nearest, worker);
	return math::exp(-tau);
}

float3 Single_scattering_tracking::li(const Ray& ray, const Volume& volume,
									  Worker& worker, float3& transmittance) {
	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const auto& material = *volume.material(0);

	constexpr bool spectral = true;

	if (spectral) {
		const float d = ray.max_t - ray.min_t;

		float3 w(1.f);
		float t = 0.f;

		const float mt = math::max_element(material.max_extinction());
		while (true) {
			const float r = rng_.random_float();
			t = t -std::log(1.f - r) / mt;
			if (t > d) {
				transmittance = float3(1.f);
				return float3(0.f);
			}

			const float3 p = ray.point(ray.min_t + t);

			const float3 sigma_a = material.absorption(transformation, p,
													   Sampler_filter::Undefined, worker);

			const float3 sigma_s = material.scattering(transformation, p,
													   Sampler_filter::Undefined, worker);

			const float3 extinction = sigma_a + sigma_s;

			const float msa = math::max_element(sigma_a);
			const float mss = math::max_element(sigma_s);
			const float c = 1.f / (msa + mss);

			const float pa = msa * c;
			const float ps = mss * c;

			const float r2 = rng_.random_float();
			if (r2 < pa) {
				transmittance = float3(0.f);
			//	const float3 lw = (sigma_a / (mt * pa));
			//	transmittance = 1.f - lw;
			//	return 1.f - lw;
				return float3(0.f);
			} else {
				const float3 l = estimate_direct_light(ray, p, worker);
				w *= (sigma_s / (mt * ps));
				transmittance = float3(0.f);
			//	const float3 scattering_albedo = sigma_s / extinction;

				return l;
			}
		}
	} else {
		const float d = ray.max_t - ray.min_t;

		const float3 sigma_a = material.absorption(transformation, float3::identity(),
												   Sampler_filter::Undefined, worker);

		const float3 sigma_s = material.scattering(transformation, float3::identity(),
												   Sampler_filter::Undefined, worker);

		const float3 extinction = sigma_a + sigma_s;

		const float3 scattering_albedo = sigma_s / extinction;

		const float3 tr = math::exp(-d * extinction);

		const float r = rng_.random_float();
		const float scatter_distance = -std::log(1.f - r * (1.f - spectrum::average(tr))) / spectrum::average(extinction);

		const float3 p = ray.point(ray.min_t + scatter_distance);

		const float3 l = estimate_direct_light(ray, p, worker);

		transmittance = tr;
		return l * (1.f - tr) * scattering_albedo;
	}
}

float3 Single_scattering_tracking::transmittance(const Ray& ray, const Intersection& intersection,
												 const Worker& worker) {
	const auto& prop = *intersection.prop;

	Transformation temp;
	const auto& transformation = prop.transformation_at(ray.time, temp);

	const auto& material = *intersection.material();

	if (material.is_heterogeneous_volume()) {
		const float d = ray.max_t - ray.min_t;
		const float max_extinction = spectrum::average(material.max_extinction());
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
			if (r2 < spectrum::average(extinction) / max_extinction) {
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
											  settings_.step_size, rng_,
											  Sampler_filter::Nearest, worker);
	return math::exp(-tau);
}

bool Single_scattering_tracking::integrate(Ray& ray, Intersection& intersection,
										   const Material_sample& material_sample, Worker& worker,
										   float3& li, float3& transmittance, float3& weight) {
	Transformation temp;
	const auto& transformation = intersection.prop->transformation_at(ray.time, temp);

	const auto& material = *intersection.material();

	const bool hit = worker.intersect_and_resolve_mask(ray, intersection, Sampler_filter::Nearest);
	if (!hit) {
		li = float3(0.f);
		transmittance = float3(1.f);
		weight = float3(1.f);
		return false;
	}

	const float d = ray.max_t;

	const bool multiple_scattering = true;

	if (multiple_scattering) {
		const float3 sigma_a = material.absorption(transformation, float3::identity(),
												   Sampler_filter::Undefined, worker);

		const float3 sigma_s = material.scattering(transformation, float3::identity(),
												   Sampler_filter::Undefined, worker);

		const float3 extinction = sigma_a + sigma_s;

		const float r = rng_.random_float();
		const float scatter_distance = -std::log(1.f - r) / spectrum::average(extinction);

		if (scatter_distance < d) {
			const float3 p = ray.point(scatter_distance);

			intersection.geo.p = p;
			intersection.geo.epsilon = 0.f;
			intersection.geo.subsurface = true;

			transmittance = math::exp(-scatter_distance * extinction);
			const float3 pdf = extinction * transmittance;
			const float3 scattering_albedo = sigma_s / extinction;
			weight = scattering_albedo * extinction / pdf;
		} else {
			transmittance = math::exp(-d * extinction);
			weight = 1.f / transmittance;
		}

		li = float3(0.f);
		return true;
	} else {
		if (material.is_heterogeneous_volume()) {
			constexpr bool spectral = false;

			if (spectral) {
				float3 w(1.f);
				float t = 0.f;

				const float mt = math::max_element(material.max_extinction());
				while (true) {
					const float r = rng_.random_float();
					t = t -std::log(1.f - r) / mt;
					if (t > d) {
						transmittance = float3(1.f);
						li = float3(0.f);
						weight = float3(1.f);
						return true;
					}

					const float3 p = ray.point(t);

					const float3 sigma_a = material.absorption(transformation, p,
															   Sampler_filter::Undefined, worker);

					const float3 sigma_s = material.scattering(transformation, p,
															   Sampler_filter::Undefined, worker);

					const float3 extinction = sigma_a + sigma_s;

					const float3 sigma_n = material.max_extinction() - extinction;

					const float msa = math::max_element(sigma_a);
					const float mss = math::max_element(sigma_s);
					const float msn = math::max_element(sigma_n);
					const float c = 1.f / (msa + mss + msn);

					const float pa = msa * c;
					const float ps = mss * c;
					const float pn = msn * c;

					const float r2 = rng_.random_float();
					if (r2 < pa) {
						transmittance = float3(0.f);
						li = float3(0.f);
						weight = float3(1.f);
						return true;
					} else if (r2 < 1.f - pn) {
						float3 l = estimate_direct_light(ray, p, intersection, material_sample, worker);

						li = w * l;

						transmittance = float3(0.f);
						weight = float3(1.f);
						return true;
					} else {
						w *= (sigma_n / (mt * pn));
					}
				}

			} else {
				const float max_extinction = spectrum::average(material.max_extinction());
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

					p = ray.point(t);

					const float3 sigma_a = material.absorption(transformation, p,
															   Sampler_filter::Undefined, worker);

					const float3 sigma_s = material.scattering(transformation, p,
															   Sampler_filter::Undefined, worker);

					extinction = sigma_a + sigma_s;
					const float r2 = rng_.random_float();
					if (r2 < spectrum::average(extinction) / max_extinction) {
						terminated = true;

						scattering_albedo = sigma_s / extinction;
					}
				} while (!terminated);

				if (terminated) {
					float3 l = estimate_direct_light(ray, p, intersection, material_sample, worker);

					l *= scattering_albedo;// * extinction;

					li = l;

					transmittance = float3(0.f);
					weight = float3(1.f);
				} else {
					li = float3(0.f);
					transmittance = float3(1.f);
					weight = float3(1.f);
				}
			}
		} else {
			constexpr Algorithm algorithm = Algorithm::Experiment;

			if (Algorithm::Tracking == algorithm) {
				const float3 sigma_a = material.absorption(transformation, float3::identity(),
														   Sampler_filter::Undefined, worker);

				const float3 sigma_s = material.scattering(transformation, float3::identity(),
														   Sampler_filter::Undefined, worker);

				const float3 extinction = sigma_a + sigma_s;

				const float3 scattering_albedo = sigma_s / extinction;

				transmittance = math::exp(-d * extinction);

				const float r = rng_.random_float();
				const float scatter_distance = -std::log(1.f - r * (1.f - spectrum::average(transmittance))) / spectrum::average(extinction);

				const float3 p = ray.point(scatter_distance);

				float3 l = estimate_direct_light(ray, p, intersection, material_sample, worker);

				l *= (1.f - transmittance) * scattering_albedo;

				li = l;
				weight = float3(1.f);
			} else if (Algorithm::Delta_tracking == algorithm) {
				const float max_extinction = spectrum::average(material.max_extinction());
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

					p = ray.point(t);

					const float3 sigma_a = material.absorption(transformation, p,
															   Sampler_filter::Undefined, worker);

					const float3 sigma_s = material.scattering(transformation, p,
															   Sampler_filter::Undefined, worker);

					extinction = sigma_a + sigma_s;
					const float r2 = rng_.random_float();
					if (r2 < spectrum::average(extinction) / max_extinction) {
						terminated = true;

						scattering_albedo = sigma_s / extinction;
					}
				} while (!terminated);

				if (terminated) {
					float3 l = estimate_direct_light(ray, p, intersection, material_sample, worker);

					l *= scattering_albedo * extinction;

					li = l;

					transmittance = float3(0.f);
					weight = float3(1.f);
				} else {
					transmittance = float3(1.f);
					li = float3(0.f);
					weight = float3(1.f);
				}
			} else {
				const float max_extinction = spectrum::average(material.max_extinction());
				bool terminated = false;
				float t = 0.f;

				float3 p;
				float3 scattering_albedo;

				do {
					const float r = rng_.random_float();
					t = t -std::log(1.f - r) / max_extinction;
					if (t > d) {
						break;
					}

					p = ray.point(t);

					const float3 sigma_a = material.absorption(transformation, p,
															   Sampler_filter::Undefined, worker);

					const float3 sigma_s = material.scattering(transformation, p,
															   Sampler_filter::Undefined, worker);

					const float3 extinction = sigma_a + sigma_s;
					const float r2 = rng_.random_float();
					if (r2 < spectrum::average(extinction) / max_extinction) {
						terminated = true;

						scattering_albedo = sigma_s / extinction;
					}
				} while (!terminated);

				if (terminated) {
					float3 l = estimate_direct_light(ray, p, intersection, material_sample, worker);

					l *= scattering_albedo;

					li = l;

					transmittance = float3(0.f);
					weight = float3(1.f);
				} else {
					transmittance = float3(1.f);
					li = float3(0.f);
					weight = float3(1.f);
				}
			}
		}

		return true;
	}
}

size_t Single_scattering_tracking::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

float3 Single_scattering_tracking::estimate_direct_light(const Ray& ray, const float3& position,
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

float3 Single_scattering_tracking::estimate_direct_light(const Ray& ray, const float3& position,
														 const Intersection& intersection,
														 const Material_sample& material_sample,
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

	//	const float3 tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);

		Intersection tintersection = intersection;
		tintersection.geo.subsurface = true;
		const float3 tv = worker.tinted_visibility(shadow_ray, tintersection, material_sample, Sampler_filter::Nearest);
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
