#include "scene.hpp"
#include "surrounding/surrounding.hpp"
#include "prop/prop.hpp"
#include "prop/prop_intersection.hpp"
#include "light/shape_light.hpp"
#include "bvh/scene_bvh_builder.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene {

Scene::Scene() : surrounding_(nullptr) {}

Scene::~Scene() {
	for (auto l : lights_) {
		delete l;
	}

	for (auto p : props_) {
		delete p;
	}

	delete surrounding_;
}

bool Scene::intersect(math::Oray& ray, uint32_t worker_id, Intersection& intersection) const {
	return bvh_.intersect(ray, intersection);
}

bool Scene::intersect_p(const math::Oray& ray, uint32_t worker_id) const {
	return bvh_.intersect_p(ray);
}

void Scene::compile() {
	bvh::Builder builder;
	builder.build(bvh_, props_);
}

const surrounding::Surrounding* Scene::surrounding() const {
	return surrounding_;
}

void Scene::set_surrounding(const surrounding::Surrounding* surrounding) {
	surrounding_ = surrounding;
}

Prop* Scene::create_prop() {
	Prop* prop = new Prop;
	props_.push_back(prop);
	return prop;
}

const std::vector<light::Light*>& Scene::lights() const {
	return lights_;
}

light::Light* Scene::montecarlo_light(float random, float& pdf) const {
	float num = static_cast<float>(lights_.size());
	size_t l = static_cast<size_t>(num * random - 0.001f);

	pdf = 1.f / num;

	if (l >= lights_.size()) {
		return nullptr;
	} else {
		return lights_[l];
	}
}

light::Shape_light* Scene::create_shape_light() {
	light::Shape_light* light = new light::Shape_light;
	lights_.push_back(light);
	return light;
}

}
