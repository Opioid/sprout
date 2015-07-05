#include "scene.hpp"
#include "scene/animation/animation.hpp"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/light/prop_light.hpp"
#include "scene/light/prop_image_light.hpp"
#include "base/color/color.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"
#include "base/math/distribution_1d.inl"
#include "base/math/bounding/aabb.inl"

namespace scene {

Scene::Scene() : tick_duration_(1.f / 60.f), simulation_time_(0.f) {}

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

float Scene::tick_duration() const {
	return tick_duration_;
}

float Scene::simulation_time() const {
	return simulation_time_;
}

void Scene::tick() {
	for (auto a : animations_) {
		a->tick(tick_duration_);
	}

	for (auto& s : animation_stages_) {
		s.update();
	}

	compile();

	simulation_time_ += tick_duration_;
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

	uint32_t l = light_distribution_.sample_discrete(random, pdf);

	return lights_[l];
}

light::Prop_light* Scene::create_prop_light() {
	light::Prop_light* light = new light::Prop_light;
	lights_.push_back(light);
	return light;
}

light::Prop_image_light* Scene::create_prop_image_light() {
	light::Prop_image_light* light = new light::Prop_image_light;
	lights_.push_back(light);
	return light;
}

void Scene::add_animation(std::shared_ptr<animation::Animation> animation) {
    animations_.push_back(animation);
}

void Scene::create_animation_stage(entity::Entity* entity, animation::Animation* animation) {
    animation_stages_.push_back(animation::Stage(entity, animation));
}

void Scene::compile() {
	builder_.build(bvh_, props_);

	std::vector<float> power;
	power.reserve(lights_.size());

	for (auto l : lights_) {
		l->prepare_sampling();
		power.push_back(color::luminance(l->power(bvh_.aabb())));
	}

	light_distribution_.init(power.data(), power.size());
}

}
