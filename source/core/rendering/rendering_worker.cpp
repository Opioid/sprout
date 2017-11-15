#include "rendering_worker.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/prop.hpp"
#include "scene/scene_intersection.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/material.hpp"
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

float4 Worker::li(scene::Ray& ray) {
	scene::Intersection intersection;
	const bool hit = intersect(ray, intersection);

	const auto volume = scene_->volume_region();

	if (volume) {
		float3 vtr;
		const float3 vli = volume_integrator_->li(ray, true, *volume, *this, vtr);

		if (hit) {
			const float4 li = surface_integrator_->li(ray, intersection, *this);
			return float4(vtr * li.xyz() + vli, li[3]);
		} else {
			return float4(vli, 1.f);
		}
	} else {
		if (hit) {
			return surface_integrator_->li(ray, intersection, *this);
		} else {
			return float4::identity();
		}
	}
}

float4 Worker::surface_li(scene::Ray& ray) {
	scene::Intersection intersection;
	const bool hit = intersect(ray, intersection);

	if (hit) {
		return surface_integrator_->li(ray, intersection, *this);
	} else {
		return float4::identity();
	}
}

float3 Worker::volume_li(const scene::Ray& ray, bool primary_ray, float3& transmittance) {
	const auto volume = scene_->volume_region();

	if (!volume) {
		transmittance = float3(1.f);
		return float3(0.f);
	}

	return volume_integrator_->li(ray, primary_ray, *volume, *this, transmittance);
}

float3 Worker::transmittance(const scene::Ray& ray) {
	const auto volume = scene_->volume_region();

	if (!volume) {
		return float3(1.f);
	}

	return volume_integrator_->transmittance(ray, *volume, *this);
}

sampler::Sampler* Worker::sampler() {
	return sampler_;
}

}
