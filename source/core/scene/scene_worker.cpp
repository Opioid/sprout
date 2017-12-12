#include "scene_worker.hpp"
#include "scene.hpp"
#include "scene_constants.hpp"
#include "scene_ray.inl"
#include "prop/prop.hpp"
#include "prop/prop_intersection.inl"
#include "material/sampler_cache.hpp"
#include "shape/node_stack.inl"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"

namespace scene {

using Texture_sampler_2D = image::texture::sampler::Sampler_2D;
using Texture_sampler_3D = image::texture::sampler::Sampler_3D;

Worker::Worker() : node_stack_(128 + 16) {}

void Worker::init(uint32_t id, const take::Settings& settings,
				  const Scene& scene, uint32_t max_sample_size) {
	id_ = id;
	rng_ = rnd::Generator(0, id);
	settings_ = settings;
	scene_ = &scene;
	sample_cache_.init(max_sample_size);
}

Worker::~Worker() {}

uint32_t Worker::id() const {
	return id_;
}

bool Worker::intersect(Ray& ray, prop::Intersection& intersection) const {
	return scene_->intersect(ray, node_stack_, intersection);
}

bool Worker::intersect(const prop::Prop* prop, Ray& ray, prop::Intersection& intersection) const {
	const bool hit = prop->intersect(ray, node_stack_, intersection.geo);
	if (hit) {
		intersection.prop = prop;
	}
	// The following is problematic for our resolving of transmission in closed shapes.
	// It can happen that we don't get an intersection in corner cases,
	// and the calling code (somewhat reasonably) does not expect prop suddenly becoming null.
	// Probably there is a more elegant way of solving this
//	else {
//		intersection.prop = nullptr;
//	}

	return hit;
}

bool Worker::resolve_mask(Ray& ray, prop::Intersection& intersection, Sampler_filter filter) {
	const float ray_offset_factor = settings_.ray_offset_factor;

	const float start_min_t = ray.min_t;

	float opacity = intersection.opacity(ray.time, filter, *this);

	while (opacity < 1.f) {
		if (opacity > 0.f && opacity > rng_.random_float()) {
			ray.min_t = start_min_t;
			return true;
		}

		// Slide along ray until opaque surface is found
		ray.min_t = ray.max_t + ray_offset_factor * intersection.geo.epsilon;
		ray.max_t = scene::Ray_max_t;
		if (!intersect(ray, intersection)) {
			ray.min_t = start_min_t;
			return false;
		}

		opacity = intersection.opacity(ray.time, filter, *this);
	}

	ray.min_t = start_min_t;
	return true;
}

bool Worker::intersect_and_resolve_mask(Ray& ray, prop::Intersection& intersection,
										Sampler_filter filter) {
	if (!intersect(ray, intersection)) {
		return false;
	}

	return resolve_mask(ray, intersection, filter);
}

bool Worker::visibility(const Ray& ray) const {
	return !scene_->intersect_p(ray, node_stack_);
}

float Worker::masked_visibility(const Ray& ray, Sampler_filter filter) const {
	return 1.f - scene_->opacity(ray, filter, *this);
}

float3 Worker::tinted_visibility(const Ray& ray, Sampler_filter filter) const {
	return float3(1.f) - scene_->thin_absorption(ray, filter, *this);
}

const scene::Scene& Worker::scene() const {
	return *scene_;
}

scene::shape::Node_stack& Worker::node_stack() const {
	return node_stack_;
}

material::Sample_cache& Worker::sample_cache() const {
	return sample_cache_;
}

const Texture_sampler_2D& Worker::sampler_2D(uint32_t key, Sampler_filter filter) const {
	return sampler_cache_.sampler_2D(key, filter);
}

const Texture_sampler_3D& Worker::sampler_3D(uint32_t key, Sampler_filter filter) const {
	return sampler_cache_.sampler_3D(key, filter);
}

}
