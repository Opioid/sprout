#include "scene_worker.hpp"
#include "prop.hpp"
#include "scene.hpp"
#include "scene_intersection.hpp"
#include "material/sampler_cache.hpp"
#include "shape/node_stack.inl"
#include "base/math/vector4.inl"

namespace scene {

using Texture_sampler_2D = image::texture::sampler::Sampler_2D;
using Texture_sampler_3D = image::texture::sampler::Sampler_3D;

Worker::Worker() : node_stack_(128) {}

void Worker::init(uint32_t id, const Scene& scene) {
	id_ = id;
	scene_ = &scene;
}

Worker::~Worker() {}

uint32_t Worker::id() const {
	return id_;
}

bool Worker::intersect(Ray& ray, Intersection& intersection) {
	return scene_->intersect(ray, node_stack_, intersection);
}

bool Worker::intersect(const Prop* prop, Ray& ray, Intersection& intersection) {
	bool hit = prop->intersect(ray, node_stack_, intersection.geo);
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

bool Worker::visibility(const Ray& ray) {
	return !scene_->intersect_p(ray, node_stack_);
}

float Worker::masked_visibility(const Ray& ray, Sampler_filter filter) {
	return 1.f - scene_->opacity(ray, *this, filter);
}

float3 Worker::tinted_visibility(const Ray& ray, Sampler_filter filter) {
	return float3(1.f) - scene_->thin_absorption(ray, *this, filter);
}

const scene::Scene& Worker::scene() const {
	return *scene_;
}

scene::shape::Node_stack& Worker::node_stack() {
	return node_stack_;
}

const Texture_sampler_2D& Worker::sampler_2D(uint32_t key, Sampler_filter filter) const {
	return sampler_cache_.sampler_2D(key, filter);
}

const Texture_sampler_3D& Worker::sampler_3D(uint32_t key, Sampler_filter filter) const {
	return sampler_cache_.sampler_3D(key, filter);
}

}
