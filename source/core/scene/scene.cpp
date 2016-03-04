#include "scene.hpp"
#include "scene/animation/animation.hpp"
#include "scene/entity/dummy.hpp"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/shape/shape.hpp"
#include "scene/light/prop_light.hpp"
#include "scene/light/prop_image_light.hpp"
#include "scene/volume/homogeneous.hpp"
#include "base/color/color.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"
#include "base/math/bounding/aabb.inl"
#include "base/math/distribution/distribution_1d.inl"

namespace scene {

Scene::Scene() : tick_duration_(1.f / 60.f), simulation_time_(0.f), volume_region_(nullptr) {}

Scene::~Scene() {
	delete volume_region_;

	for (auto l : lights_) {
		delete l;
	}

	for (auto p : finite_props_) {
		delete p;
	}

	for (auto p : infinite_props_) {
		delete p;
	}

	for (auto d : dummies_) {
		delete d;
	}
}

const math::aabb& Scene::aabb() const {
	return bvh_.aabb();
}

bool Scene::intersect(scene::Ray& ray, shape::Node_stack& node_stack, Intersection& intersection) const {
	return bvh_.intersect(ray, node_stack, intersection);
}

bool Scene::intersect_p(const scene::Ray& ray, shape::Node_stack& node_stack) const {
	return bvh_.intersect_p(ray, node_stack);
}

float Scene::opacity(const scene::Ray& ray, Worker& worker, material::Texture_filter override_filter) const {
	return bvh_.opacity(ray, worker, override_filter);
}

float Scene::tick_duration() const {
	return tick_duration_;
}

float Scene::simulation_time() const {
	return simulation_time_;
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

const volume::Volume* Scene::volume_region() const {
	return volume_region_;
}

void Scene::tick(thread::Pool& thread_pool) {
	for (auto m : materials_) {
		m->tick(simulation_time_, tick_duration_);
	}

	for (auto a : animations_) {
		a->tick(tick_duration_);
	}

	for (auto& s : animation_stages_) {
		s.update();
	}

	for (auto p : finite_props_) {
		p->morph(thread_pool);
	}

	for (auto d : dummies_) {
		d->calculate_world_transformation();
	}

	for (auto p : finite_props_) {
		p->calculate_world_transformation();
	}

	for (auto p : infinite_props_) {
		p->calculate_world_transformation();
	}

	compile();

	simulation_time_ += tick_duration_;
}

float Scene::seek(float time, thread::Pool& thread_pool) {
//	float tick_offset = std::fmod(time, tick_duration_);

	// see http://stackoverflow.com/questions/4218961/why-fmod1-0-0-1-1
	// for explanation why std::floor() variant seems to give results more in line with my expectations
	float tick_offset = time - std::floor(time / tick_duration_) * tick_duration_;

	float first_tick = time - tick_offset;

	for (auto a : animations_) {
		a->seek(first_tick);
	}

	for (auto& s : animation_stages_) {
		s.update();
	}

	simulation_time_ = first_tick;

	tick(thread_pool);

	return tick_offset;
}

entity::Dummy* Scene::create_dummy() {
	entity::Dummy* dummy = new entity::Dummy;
	dummies_.push_back(dummy);
	return dummy;
}

Prop* Scene::create_prop(std::shared_ptr<shape::Shape> shape, const material::Materials& materials) {
	Prop* prop = new Prop;

	if (shape->is_finite()) {
		finite_props_.push_back(prop);
	} else {
		infinite_props_.push_back(prop);
	}

	prop->init(shape, materials);

	return prop;
}

light::Prop_light* Scene::create_prop_light(Prop* prop, uint32_t part) {
	light::Prop_light* light = new light::Prop_light;
	lights_.push_back(light);
	light->init(prop, part);
	return light;
}

light::Prop_image_light* Scene::create_prop_image_light(Prop* prop, uint32_t part) {
	light::Prop_image_light* light = new light::Prop_image_light;
	lights_.push_back(light);
	light->init(prop, part);
	return light;
}

volume::Volume* Scene::create_volume(const math::float3& absorption, const math::float3& scattering) {
	volume_region_ = new volume::Homogeneous(absorption, scattering);
	return volume_region_;
}

void Scene::add_material(std::shared_ptr<material::Material> material) {
	materials_.push_back(material);
}

void Scene::add_animation(std::shared_ptr<animation::Animation> animation) {
    animations_.push_back(animation);
}

void Scene::create_animation_stage(entity::Entity* entity, animation::Animation* animation) {
    animation_stages_.push_back(animation::Stage(entity, animation));
}

void Scene::compile() {
	builder_.build(bvh_, finite_props_, infinite_props_);

	light_powers_.clear();

	for (auto l : lights_) {
		l->prepare_sampling();
		light_powers_.push_back(std::sqrt(color::luminance(l->power(bvh_.aabb()))));
	}

	light_distribution_.init(light_powers_.data(), light_powers_.size());

	if (volume_region_) {
		volume_region_->set_scene_aabb(bvh_.aabb());
	}
}

}
