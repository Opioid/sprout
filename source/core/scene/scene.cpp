#include "scene.hpp"
#include "surrounding/surrounding.hpp"
#include "prop/prop.hpp"
#include "prop/prop_intersection.hpp"
#include "bvh/builder.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene {

Scene::Scene() : surrounding_(nullptr) {}

Scene::~Scene() {
	delete surrounding_;

	for (auto p : props_) {
		delete p;
	}
}

bool Scene::intersect(math::Oray& ray, Intersection& intersection) const {
//	return bvh_.intersect(ray, intersection);


	bool hit = false;

	for (auto p : props_) {
		if (p->intersect(ray, intersection.geo)) {
			intersection.prop = p;
			hit = true;
		}
	}

	return hit;

}

bool Scene::intersect_p(const math::Oray& ray) const {
//	return bvh_.intersect_p(ray);


	for (auto p : props_) {
		if (p->intersect_p(ray)) {
			return true;
		}
	}

	return false;

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

}
