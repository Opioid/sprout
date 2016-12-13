#include "rendering_worker.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/prop.hpp"
#include "scene/scene_intersection.hpp"
#include "scene/material/material.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/random/generator.inl"

namespace rendering {

Worker::Worker() :
	surface_integrator_(nullptr),
	volume_integrator_(nullptr),
	sampler_(nullptr) {}

Worker::~Worker() {
	delete sampler_;
	delete volume_integrator_;
	delete surface_integrator_;
}

void Worker::init(uint32_t id, const scene::Scene& scene,
				  const rnd::Generator& rng,
				  integrator::surface::Factory& surface_integrator_factory,
				  integrator::volume::Factory& volume_integrator_factory,
				  sampler::Factory& sampler_factory) {
	scene::Worker::init(id, scene);

	rng_ = rng;

	surface_integrator_ = surface_integrator_factory.create(rng_);
	volume_integrator_  = volume_integrator_factory.create(rng_);
	sampler_ = sampler_factory.create(rng_);
}

void Worker::prepare(uint32_t num_samples_per_pixel) {
	surface_integrator_->prepare(*scene_, num_samples_per_pixel);
	volume_integrator_->prepare(*scene_, num_samples_per_pixel);
	sampler_->resize(num_samples_per_pixel, 1, 2, 1);
}

float4 Worker::li(scene::Ray& ray) {
	scene::Intersection intersection;
	bool hit = intersect(ray, intersection);

	auto volume = scene_->volume_region();

	if (volume) {
		float3 vtr;
		float4 vli = volume_integrator_->li(*this, *volume, ray, vtr);

		if (hit) {
			float4 li = surface_integrator_->li(*this, ray, false, intersection);
			return float4(vtr * li.xyz, li.w) + vli;
		} else {
			return vli;
		}
	} else {
		if (hit) {
			return surface_integrator_->li(*this, ray, false, intersection);
		} else {
			return math::float4_identity;
		}
	}
}

float3 Worker::surface_li(scene::Ray& ray) {
	scene::Intersection intersection;
	bool hit = intersect(ray, intersection);

	if (hit) {
		scene::Ray tray = ray;
		return surface_integrator_->li(*this, tray, false, intersection).xyz;
	} else {
		return float3(0.f);
	}
}

float4 Worker::volume_li(const scene::Ray& ray, float3& transmittance) {
	auto volume = scene_->volume_region();

	if (!volume) {
		transmittance = float3(1.f);
		return float4(0.f);
	}

	return volume_integrator_->li(*this, *volume, ray, transmittance);
}

float3 Worker::transmittance(const scene::Ray& ray) {
	auto volume = scene_->volume_region();

	if (!volume) {
		return float3(1.f);
	}

	return volume_integrator_->transmittance(*this, *volume, ray);
}

sampler::Sampler* Worker::sampler() {
	return sampler_;
}

size_t Worker::num_bytes() const {
	return scene::Worker::num_bytes() +
		   surface_integrator_->num_bytes() +
		   volume_integrator_->num_bytes() +
		   sampler_->num_bytes();
}

}
