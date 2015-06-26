#include "scene.hpp"
#include "scene/animation/animation.hpp"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/light/image_light.hpp"
#include "scene/light/prop_light.hpp"
#include "scene/light/uniform_light.hpp"
#include "base/color/color.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"
#include "base/math/cdf.inl"
#include "base/math/bounding/aabb.inl"

namespace scene {

Scene::Scene() {}

Scene::~Scene() {
	for (auto l : lights_) {
		delete l;
	}

	for (auto p : props_) {
		delete p;
	}
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

void Scene::tick(float time_slice) {
	for (auto a : animations_) {
		a->tick(time_slice);
	}

	for (auto& s : animation_stages_) {
		s.update();
	}
}

void Scene::compile() {
    builder_.build(bvh_, props_);

	std::vector<float> power;
	power.reserve(lights_.size());

	for (auto l : lights_) {
		l->prepare_sampling();
		power.push_back(color::luminance(l->power(bvh_.aabb())));
	}

	light_cdf_.init(power);
}

Prop* Scene::create_prop() {
	Prop* prop = new Prop;
	props_.push_back(prop);
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

light::Image_light* Scene::create_image_light() {
	light::Image_light* light = new light::Image_light;
	lights_.push_back(light);
	return light;
}

light::Prop_light* Scene::create_prop_light() {
	light::Prop_light* light = new light::Prop_light;
	lights_.push_back(light);
	return light;
}

light::Uniform_light* Scene::create_uniform_light() {
	light::Uniform_light* light = new light::Uniform_light;
	lights_.push_back(light);
	return light;
}

void Scene::add_animation(std::shared_ptr<animation::Animation> animation) {
    animations_.push_back(animation);
}

void Scene::create_animation_stage(entity::Entity* entity, animation::Animation* animation) {
    animation_stages_.push_back(animation::Stage(entity, animation));
}

}
