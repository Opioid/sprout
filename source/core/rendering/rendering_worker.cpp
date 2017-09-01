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

Worker::Worker() :
	surface_integrator_(nullptr),
	volume_integrator_(nullptr),
	sampler_(nullptr) {}

Worker::~Worker() {
	memory::safe_destruct(sampler_);
	memory::safe_destruct(volume_integrator_);
	memory::safe_destruct(surface_integrator_);
}

void Worker::init(uint32_t id, const scene::Scene& scene,
				  uint32_t max_sample_size, const rnd::Generator& rng,
				  integrator::surface::Factory& surface_integrator_factory,
				  integrator::volume::Factory& volume_integrator_factory,
				  sampler::Factory& sampler_factory) {
	scene::Worker::init(id, scene, max_sample_size);

	rng_ = rng;

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
		const float4 vli = volume_integrator_->li(*this, ray, true, *volume, vtr);

		if (hit) {
			const float4 li = surface_integrator_->li(*this, ray, intersection);
			return float4(vtr * li.xyz(), li[3]) + vli;
		} else {
			return vli;
		}
	} else {
		if (hit) {
			return surface_integrator_->li(*this, ray, intersection);
		} else {
			return float4::identity();
		}
	}
}

float4 Worker::volume_li(const scene::Ray& ray, bool primary_ray, float3& transmittance) {
	const auto volume = scene_->volume_region();

	if (!volume) {
		transmittance = float3(1.f);
		return float4(0.f);
	}

	return volume_integrator_->li(*this, ray, primary_ray, *volume, transmittance);
}

float3 Worker::transmittance(const scene::Ray& ray) {
	const auto volume = scene_->volume_region();

	if (!volume) {
		return float3(1.f);
	}

	return volume_integrator_->transmittance(*this, ray, *volume);
}

sampler::Sampler* Worker::sampler() {
	return sampler_;
}

}
