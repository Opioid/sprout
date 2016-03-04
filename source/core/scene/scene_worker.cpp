#include "scene_worker.hpp"
#include "scene.hpp"
#include "material/sampler_cache.hpp"
#include "prop/prop.hpp"
#include "prop/prop_intersection.hpp"
#include "shape/node_stack.inl"
#include "base/math/vector.inl"

namespace scene {

Worker::Worker() : node_stack_(128) {}

void Worker::init(uint32_t id, const Scene& scene) {
	id_ = id;
	scene_ = &scene;
}

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

	return hit;
}

bool Worker::visibility(const Ray& ray) {
	return !scene_->intersect_p(ray, node_stack_);
}

float Worker::masked_visibility(const Ray& ray, material::Sampler_settings::Filter filter) {
	return 1.f - scene_->opacity(ray, *this, filter);
}

const scene::Scene& Worker::scene() const {
	return *scene_;
}

scene::shape::Node_stack& Worker::node_stack() {
	return node_stack_;
}

const image::texture::sampler::Sampler_2D&
Worker::sampler(uint32_t key, material::Sampler_settings::Filter filter) const {
	return sampler_cache_.sampler(key, filter);
}

}
