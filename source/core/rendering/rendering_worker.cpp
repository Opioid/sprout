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
#include "base/random/generator.inl"

namespace rendering {

Worker::~Worker() {
	memory::safe_destruct(sampler_);
	memory::safe_destruct(volume_integrator_);
	memory::safe_destruct(surface_integrator_);
}

void Worker::init(uint32_t id, const scene::Scene& scene, uint32_t max_sample_size,
				  integrator::surface::Factory& surface_integrator_factory,
				  integrator::volume::Factory& volume_integrator_factory,
				  sampler::Factory& sampler_factory) {
	scene::Worker::init(id, scene, max_sample_size);

	rng_ = rnd::Generator(0, id);

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
	const bool hit = intersect(ray, intersection);

	if (hit) {
		return surface_integrator_->li(ray, intersection, *this);
	} else {
		float3 vtr;
		return float4(volume_li(ray, true, vtr), 1.f);
	}
}

float4 Worker::surface_li(Ray& ray) {
	scene::prop::Intersection intersection;
	const bool hit = intersect(ray, intersection);

	if (hit) {
		return surface_integrator_->li(ray, intersection, *this);
	} else {
		return float4::identity();
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

		float3 temp;
		radiance += tr * volume_integrator_->li(tray, primary_ray, *volume, *this, temp);
		tr *= temp;

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

		transmittance *= volume_integrator_->transmittance(tray, *volume, *this);

		tray.min_t = tray.max_t + epsilon;
		tray.max_t = ray.max_t;
	}

	return transmittance;
}

sampler::Sampler* Worker::sampler() {
	return sampler_;
}

}
