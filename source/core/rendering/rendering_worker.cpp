#include "rendering_worker.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/material.hpp"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.inl"
#include "base/math/vector4.inl"
#include "base/math/sampling/sample_distribution.hpp"
#include "base/memory/align.hpp"

#include "base/debug/assert.hpp"

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
	scene::prop::Intersection intersection;
	const bool hit = intersect_and_resolve_mask(ray, intersection, Sampler_filter::Undefined);

	float3 vtr(1.f);
	const float3 vli = volume_li(ray, true, vtr);

	if (hit) {
		const float4 li = surface_integrator_->li(ray, intersection, *this);
		return float4(vtr * li.xyz() + vli, li[3]);
	} else {
		return float4(vli, 1.f);
	}
}

float3 Worker::volume_li(const Ray& ray, bool primary_ray, float3& transmittance) {
	float3 tr(1.f);
	float3 radiance(0.f);

//	if (ray.properties.test(Ray::Property::Within_volume)) {
//		transmittance = Worker::transmittance(ray);
//		return radiance;
//	}

	Ray tray = ray;

	for (; tray.min_t < tray.max_t;) {
		float epsilon;
		const auto volume = scene_->closest_volume_segment(tray, node_stack_, epsilon);
		if (!volume) {
			break;
		}

		// Otherwise too small to handle meaningfully, but we still want to continue raymarching
		if (tray.max_t - tray.min_t > 0.0005f) {
			float3 temp;
			radiance += tr * volume_integrator_->li(tray, primary_ray, *volume, *this, temp);
			tr *= temp;
		}

		SOFT_ASSERT(tray.max_t + epsilon - tray.min_t > 0.0001f);

		tray.min_t = tray.max_t + epsilon;
		tray.max_t = ray.max_t;
	}

	transmittance = tr;
	return radiance;
}

float3 Worker::transmittance(const Ray& ray) {
	float3 transmittance(1.f);

//	if (ray.properties.test(Ray::Property::Within_volume)) {
//		return transmittance;
//	}

	Ray tray = ray;

	for (; tray.min_t < tray.max_t;) {
		float epsilon;
		const auto volume = scene_->closest_volume_segment(tray, node_stack_, epsilon);
		if (!volume) {
			break;
		}

		// Otherwise too small to handle meaningfully, but we still want to continue raymarching
		if (tray.max_t - tray.min_t > 0.0005f) {
			transmittance *= volume_integrator_->transmittance(tray, *volume, *this);
		}

		SOFT_ASSERT(tray.max_t + epsilon - tray.min_t > 0.0001f);

		tray.min_t = tray.max_t + epsilon;
		tray.max_t = ray.max_t;
	}

	return transmittance;
}

sampler::Sampler* Worker::sampler() {
	return sampler_;
}

}
