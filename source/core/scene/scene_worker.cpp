#include "scene_worker.hpp"
#include "scene.hpp"
#include "prop/prop.hpp"
#include "prop/prop_intersection.hpp"
#include "shape/node_stack.inl"

namespace scene {

Worker::Worker() : node_stack_(128) {}

void Worker::init(uint32_t id, const scene::Scene& scene) {
	id_ = id;
	scene_ = &scene;
}

uint32_t Worker::id() const {
	return id_;
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
