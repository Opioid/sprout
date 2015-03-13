#include "scene.hpp"
#include "prop/prop.hpp"
#include "prop/prop_intersection.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene {

Scene::~Scene() {
	for (auto p : props_) {
		delete p;
	}
}

bool Scene::intersect(math::Oray& ray, Intersection& intersection) const {
	bool hit = false;

	for (auto p : props_) {
		if (p->intersect(ray, intersection.geo)) {
			hit = true;
			intersection.prop = p;
		}
	}

	return hit;
}

bool Scene::intersect_p(math::Oray& ray) const {
	for (auto p : props_) {
		if (p->intersect_p(ray)) {
			return true;
		}
	}

	return false;
}

Prop* Scene::create_prop() {
	Prop* prop = new Prop;
	props_.push_back(prop);
	return prop;
}

}
