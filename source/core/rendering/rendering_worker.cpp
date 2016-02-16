#include "rendering_worker.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/shape/node_stack.inl"
#include "scene/material/material.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Worker::Worker() : surface_integrator_(nullptr), volume_integrator_(nullptr), sampler_(nullptr), node_stack_(128) {}

Worker::~Worker() {
	delete sampler_;
	delete volume_integrator_;
	delete surface_integrator_;
}

void Worker::init(uint32_t id, const math::random::Generator& rng,
				  integrator::surface::Integrator_factory& surface_integrator_factory,
				  integrator::volume::Integrator_factory& volume_integrator_factory,
				  sampler::Sampler& sampler, const scene::Scene& scene) {
	id_ = id;
	rng_ = rng;
	surface_integrator_ = surface_integrator_factory.create(rng_);
	volume_integrator_  = volume_integrator_factory.create(rng_);
	sampler_ = sampler.clone();
	scene_ = &scene;
}

uint32_t Worker::id() const {
	return id_;
}

math::float4 Worker::li(scene::Ray& ray) {
	scene::Intersection intersection;
	bool hit = intersect(ray, intersection);

	auto volume = scene_->volume_region();

	if (volume) {
		math::float3 vtr;
		math::float4 vli = volume_integrator_->li(*this, volume, ray, vtr);

		if (hit) {
			math::float4 li = surface_integrator_->li(*this, ray, false, intersection);
			return math::float4(vtr * li.xyz(), li.w) + vli;
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

math::float3 Worker::surface_li(scene::Ray& ray) {
	scene::Intersection intersection;
	bool hit = intersect(ray, intersection);

	if (hit) {
		scene::Ray tray = ray;
		return math::float3(surface_integrator_->li(*this, tray, false, intersection).xyz());
	} else {
		return math::float3(0.f, 0.f, 0.f);
	}
}

math::float4 Worker::volume_li(const scene::Ray& ray, math::float3& transmittance) {
	auto volume = scene_->volume_region();

	if (!volume) {
		transmittance = math::float3(1.f, 1.f, 1.f);
		return math::float4_identity;
	}

	return volume_integrator_->li(*this, volume, ray, transmittance);
}

math::float3 Worker::transmittance(const scene::Ray& ray) {
	auto volume = scene_->volume_region();

	if (!volume) {
		return math::float3(1.f, 1.f, 1.f);
	}

	return volume_integrator_->transmittance(*this, volume, ray);
}

bool Worker::intersect(scene::Ray& ray, scene::Intersection& intersection) {
	return scene_->intersect(ray, node_stack_, intersection);
}

bool Worker::intersect(const scene::Prop* prop, scene::Ray& ray, scene::Intersection& intersection) {
	bool hit = prop->intersect(ray, node_stack_, intersection.geo);
	if (hit) {
		intersection.prop = prop;
	}

	return hit;
}

bool Worker::visibility(const scene::Ray& ray) {
	return !scene_->intersect_p(ray, node_stack_);
}

float Worker::masked_visibility(const scene::Ray& ray, const image::texture::sampler::Sampler_2D& sampler) {
	return 1.f - scene_->opacity(ray, node_stack_, sampler);
}

const scene::Scene& Worker::scene() const {
	return *scene_;
}

scene::shape::Node_stack& Worker::node_stack() {
	return node_stack_;
}

}
