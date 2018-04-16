#include "rendering_worker.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/material.hpp"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.inl"
#include "base/math/vector4.inl"
#include "base/math/sampling/sample_distribution.hpp"
#include "base/memory/align.hpp"
#include "base/spectrum/rgb.hpp"

#include "base/debug/assert.hpp"

#include <iostream>

namespace rendering {

Worker::~Worker() {
	memory::safe_destruct(sampler_);
	memory::safe_destruct(volume_integrator_);
	memory::safe_destruct(surface_integrator_);
}

void Worker::init(uint32_t id, const take::Settings& settings,
				  const scene::Scene& scene, uint32_t max_sample_size,
				  integrator::surface::Factory& surface_integrator_factory,
				  integrator::volume::Factory& volume_integrator_factory,
				  sampler::Factory& sampler_factory) {
	scene::Worker::init(id, settings, scene, max_sample_size);

	surface_integrator_ = surface_integrator_factory.create(id, rng_);
	volume_integrator_  = volume_integrator_factory.create(id, rng_);
	sampler_			= sampler_factory.create(id, rng_);
}

void Worker::prepare(uint32_t num_samples_per_pixel) {
	surface_integrator_->prepare(*scene_, num_samples_per_pixel);
	volume_integrator_->prepare(*scene_, num_samples_per_pixel);
	sampler_->resize(num_samples_per_pixel, 1, 2, 1);
}

float4 Worker::li(Ray& ray) {
	interface_stack_.clear();

	scene::prop::Intersection intersection;
	const bool hit = intersect_and_resolve_mask(ray, intersection, Sampler_filter::Undefined);

	float3 vtr(1.f);
	const float3 vli = volume_li(ray, vtr);

	SOFT_ASSERT(math::all_finite_and_positive(vli));

	if (hit) {
		const float3 li = surface_integrator_->li(ray, intersection, *this);

		SOFT_ASSERT(math::all_finite_and_positive(li));

		return float4(vtr * li + vli, 1.f);
	} else {
		return float4(vli, spectrum::luminance(vli));
	}
}

float3 Worker::li(Ray& ray, Intersection& intersection) {
	return surface_integrator_->li(ray, intersection, *this);
}

float3 Worker::volume_li(const Ray& ray, float3& transmittance) {
	float3 tr(1.f);
	float3 radiance(0.f);

	Ray tray = ray;

	for (; tray.min_t < tray.max_t;) {
		float epsilon;
		const auto volume = scene_->closest_volume_segment(tray, node_stack_, epsilon);
		if (!volume || tray.max_t >= scene::Almost_ray_max_t_minus_epsilon) {
			// By convention don't integrate infinite volumes,
			// as the result should be pre-computed in the surrounding infinite shape alredy.
			break;
		}

		// Otherwise too small to handle meaningfully, but we still want to continue raymarching
		if (tray.max_t - tray.min_t > 0.0005f) {
			float3 temp;
			radiance += tr * volume_integrator_->li(tray, *volume, *this, temp);
			tr *= temp;
		}

		SOFT_ASSERT(tray.max_t + epsilon - tray.min_t > 0.0001f);

		tray.min_t = tray.max_t + epsilon;
		tray.max_t = ray.max_t;
	}

	transmittance = tr;
	return radiance;
}

bool Worker::volume(Ray& ray, Intersection& intersection, Sampler_filter filter,
					float3& li, float3& transmittance, float3& weight) {
	return volume_integrator_->integrate(ray, intersection, filter,
										 *this, li, transmittance, weight);
}

float3 Worker::transmittance(const Ray& ray) const {
	float3 transmittance(1.f);

	interface_stack_temp_ = interface_stack_;

	// This is the typical SSS case:
	// A medium is on the stack but we already considered it during shadow calculation,
	// igonoring the IoR. Therefore remove the medium from the stack.
	if (!interface_stack_.empty() && interface_stack_.top()->material()->ior() > 1.f) {
		interface_stack_.pop();
	}

	const float ray_max_t = ray.max_t;

	Ray tray = ray;

	Intersection intersection;

	for (;;) {
		const bool hit = scene_->intersect_volume(tray, node_stack_, intersection);

		if (!interface_stack_.empty()) {
			const float3 tr = volume_integrator_->transmittance(tray, *this);
			transmittance *= math::saturate(tr);
		}

		if (!hit) {
			break;
		}

		if (intersection.same_hemisphere(ray.direction)) {
			interface_stack_.pop();
		} else {
			interface_stack_.push(intersection);
		}

		tray.min_t = tray.max_t + intersection.geo.epsilon * settings_.ray_offset_factor;
		tray.max_t = ray_max_t;
	}

	interface_stack_ = interface_stack_temp_;

	return transmittance;
}


float3 Worker::tinted_visibility(const Ray& ray, Sampler_filter filter) const {
	return float3(1.f) - scene_->thin_absorption(ray, filter, *this);
}

float3 Worker::tinted_visibility(Ray& ray, const Intersection& intersection,
								 Sampler_filter filter) {
	if (intersection.geo.subsurface && intersection.material()->ior() > 1.f) {
		const float ray_max_t = ray.max_t;

		float epsilon;
		if (intersect(intersection.prop, ray, epsilon)) {
			const float3 tr = volume_integrator_->transmittance(ray, *this);

			SOFT_ASSERT(math::all_finite_and_positive(tr));

			ray.min_t = ray.max_t + epsilon * settings_.ray_offset_factor;
			ray.max_t = ray_max_t;

			return tr * tinted_visibility(ray, filter);;
		}
	}

	return tinted_visibility(ray, filter);
}

sampler::Sampler* Worker::sampler() {
	return sampler_;
}

}
