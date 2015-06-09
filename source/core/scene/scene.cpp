#include "scene.hpp"
#include "surrounding/surrounding.hpp"
#include "prop/prop.hpp"
#include "prop/prop_intersection.hpp"
#include "light/image_light.hpp"
#include "light/prop_light.hpp"
#include "light/uniform_light.hpp"
#include "bvh/scene_bvh_builder.hpp"
#include "base/color/color.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"
#include "base/math/cdf.inl"
#include "base/math/bounding/aabb.inl"

namespace scene {

Scene::Scene() : surrounding_(nullptr), image_light_(nullptr) {}

Scene::~Scene() {
	for (auto l : lights_) {
		delete l;
	}

	for (auto p : lose_props_) {
		delete p;
	}

	for (auto p : props_) {
		delete p;
	}

	delete surrounding_;
}

bool Scene::intersect(math::Oray& ray, Node_stack& node_stack, Intersection& intersection) const {
	return bvh_.intersect(ray, node_stack, intersection);
}

bool Scene::intersect_p(const math::Oray& ray, Node_stack& node_stack) const {
	return bvh_.intersect_p(ray, node_stack);
}

float Scene::opacity(const math::Oray& ray, Node_stack& node_stack, const image::sampler::Sampler_2D& sampler) const {
	return bvh_.opacity(ray, node_stack, sampler);
}

void Scene::compile() {
	bvh::Builder builder;
	builder.build(bvh_, props_);

	std::vector<float> power;
	power.reserve(lights_.size());

	for (auto l : lights_) {
		l->prepare_sampling();
		power.push_back(color::luminance(l->power(bvh_.aabb())));
	}

	light_cdf_.init(power);
}

const surrounding::Surrounding* Scene::surrounding() const {
	return surrounding_;
}

void Scene::set_surrounding(const surrounding::Surrounding* surrounding) {
	surrounding_ = surrounding;
}

Prop* Scene::create_prop(bool add_to_bvh) {
	Prop* prop = new Prop;

	if (add_to_bvh) {
		props_.push_back(prop);
	} else {
		lose_props_.push_back(prop);
	}

	return prop;
}

const std::vector<light::Light*>& Scene::lights() const {
	return lights_;
}

const light::Light* Scene::montecarlo_light(float random, float& pdf) const {
	if (lights_.empty()) {
		return nullptr;
	}

	uint32_t l = light_cdf_.sample(random, pdf);

	return lights_[l];
}

const light::Light* Scene::image_light() const {
	return image_light_;
}

light::Image_light* Scene::create_image_light() {
	light::Image_light* light = new light::Image_light;
	lights_.push_back(light);
	return light;
}

light::Prop_light* Scene::create_prop_light(bool image_light) {
	light::Prop_light* light = new light::Prop_light;

	if (image_light) {
		if (image_light_) {
			for (size_t i = 0, len = lights_.size(); i < len; ++i) {
				if (lights_[i] == image_light_) {
					delete image_light_;
					image_light_ = light;
					lights_[i] = light;
				}
			}
		}
	} else {
		lights_.push_back(light);
	}

	return light;
}

light::Uniform_light* Scene::create_uniform_light() {
	light::Uniform_light* light = new light::Uniform_light;
	lights_.push_back(light);
	return light;
}

}
