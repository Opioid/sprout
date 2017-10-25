#include "scene.hpp"
#include "scene/animation/animation.hpp"
#include "scene/animation/animation_stage.hpp"
#include "scene/entity/dummy.hpp"
#include "scene/prop.hpp"
#include "scene/scene_intersection.hpp"
#include "scene/shape/shape.hpp"
#include "scene/light/prop_light.hpp"
#include "scene/light/prop_image_light.hpp"
#include "scene/volume/grid.hpp"
#include "scene/volume/height.hpp"
#include "scene/volume/homogeneous.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/quaternion.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/spectrum/rgb.hpp"

#include "base/debug/assert.hpp"

namespace scene {

Scene::Scene() {
	dummies_.reserve(16);
	finite_props_.reserve(16);
	infinite_props_.reserve(16);
	lights_.reserve(16);
	extensions_.reserve(16);
	entities_.reserve(16);
	light_powers_.reserve(16);
	materials_.reserve(16);
	animations_.reserve(16);
	animation_stages_.reserve(16);
}

Scene::~Scene() {
	delete volume_region_;

	for (auto e : extensions_) {
		delete e;
	}

	if (lights_[0] != &null_light_) {
		for (auto l : lights_) {
			delete l;
		}
	}

	for (auto p : finite_props_) {
		delete p;
	}

	for (auto p : infinite_props_) {
		delete p;
	}

	for (const auto d : dummies_) {
		delete d;
	}
}

void Scene::finish() {
	if (lights_.empty()) {
		lights_.push_back(&null_light_);
	}
}

const math::AABB& Scene::aabb() const {
	return bvh_.aabb();
}

bool Scene::intersect(scene::Ray& ray, shape::Node_stack& node_stack,
					  Intersection& intersection) const {
	return bvh_.intersect(ray, node_stack, intersection);
}

bool Scene::intersect_p(const scene::Ray& ray, shape::Node_stack& node_stack) const {
	return bvh_.intersect_p(ray, node_stack);
}

float Scene::opacity(const scene::Ray& ray, material::Sampler_settings::Filter filter,
					 const Worker& worker) const {
	if (has_masked_material_) {
		return bvh_.opacity(ray, filter, worker);
	}

	return bvh_.intersect_p(ray, worker.node_stack()) ? 1.f : 0.f;
}

float3 Scene::thin_absorption(const scene::Ray& ray, material::Sampler_settings::Filter filter,
							  const Worker& worker) const {
	if (has_tinted_shadow_) {
		return bvh_.thin_absorption(ray, filter, worker);
	}

	return float3(opacity(ray, filter, worker));
}

float Scene::tick_duration() const {
	return static_cast<float>(tick_duration_);
}

float Scene::simulation_time() const {
	return static_cast<float>(simulation_time_);
}

entity::Entity* Scene::entity(size_t index) const {
	if (index >= entities_.size()) {
		return nullptr;
	}

	return entities_[index];
}

entity::Entity* Scene::entity(const std::string& name) const {
	auto e = named_entities_.find(name);
	if (named_entities_.end() == e) {
		return nullptr;
	}

	return e->second;
}

const std::vector<light::Light*>& Scene::lights() const {
	return lights_;
}

const light::Light* Scene::light(uint32_t id, float& pdf) const {
	// If the assert doesn't hold it would pose a problem,
	// but I think it is more efficient to handle those cases outside or implicitely.
	SOFT_ASSERT(!lights_.empty() && light::Light::is_light(id));

	pdf = light_distribution_.pdf(id);
	return lights_[id];
}

const light::Light* Scene::random_light(float random, float& pdf) const {
	SOFT_ASSERT(!lights_.empty());

	const uint32_t l = light_distribution_.sample_discrete(random, pdf);

	SOFT_ASSERT(l < static_cast<uint32_t>(lights_.size()));

	return lights_[l];
}

const volume::Volume* Scene::volume_region() const {
	return volume_region_;
}

void Scene::tick(thread::Pool& thread_pool) {
	const float simulation_time = static_cast<float>(simulation_time_);
	const float tick_duration	= static_cast<float>(tick_duration_);

	for (auto m : materials_) {
		m->tick(simulation_time, tick_duration);
	}

	for (auto a : animations_) {
		a->tick(tick_duration);
	}

	for (auto& s : animation_stages_) {
		s.update();
	}

	for (auto p : finite_props_) {
		p->morph(thread_pool);
	}

	compile(thread_pool);

	simulation_time_ += tick_duration_;
}

float Scene::seek(float time, thread::Pool& thread_pool) {
	// TODO: think about time precision
	// Using double for tick_duration_ specifically solved a particular bug.
	// But it did not particulary boost confidence in the entire timing thing.

//	float tick_offset = std::fmod(time, tick_duration_);

	// see http://stackoverflow.com/questions/4218961/why-fmod1-0-0-1-1
	// for explanation why std::floor() variant
	// seems to give results more in line with my expectations

	const double time_d = static_cast<double>(time);

	const double tick_offset_d = time_d - std::floor(time_d / tick_duration_) * tick_duration_;

//	float tick_offset =  static_cast<float>(tick_offset_d);// time - std::floor(time / tick_duration_) * tick_duration_;

	const double first_tick_d = time_d - tick_offset_d;

	const float first_tick = static_cast<float>(first_tick_d);

	for (auto a : animations_) {
		a->seek(first_tick);
	}

	for (auto& s : animation_stages_) {
		s.update();
	}

	simulation_time_ = first_tick_d;

	tick(thread_pool);

	return static_cast<float>(tick_offset_d);
}

void Scene::compile(thread::Pool& pool) {
	has_masked_material_ = false;
	has_tinted_shadow_	 = false;

	// handle changed transformations
	for (const auto d : dummies_) {
		d->calculate_world_transformation();
	}

	for (auto e : extensions_) {
		e->calculate_world_transformation();
	}

	for (auto p : finite_props_) {
		p->calculate_world_transformation();
		has_masked_material_ = has_masked_material_ || p->has_masked_material();
		has_tinted_shadow_ = has_tinted_shadow_ || p->has_tinted_shadow();
	}

	for (auto p : infinite_props_) {
		p->calculate_world_transformation();
		has_masked_material_ = has_masked_material_ || p->has_masked_material();
		has_tinted_shadow_ = has_tinted_shadow_ || p->has_tinted_shadow();
	}

	// rebuild the BVH
	builder_.build(bvh_, finite_props_, infinite_props_);

	// resort lights PDF
	light_powers_.clear();

	for (uint32_t i = 0, len = static_cast<uint32_t>(lights_.size()); i < len; ++i) {
		auto l = lights_[i];
		l->prepare_sampling(i, pool);
		light_powers_.push_back(std::sqrt(spectrum::luminance(l->power(bvh_.aabb()))));
	}

	light_distribution_.init(light_powers_.data(), light_powers_.size());

	if (volume_region_) {
		volume_region_->set_scene_aabb(bvh_.aabb());
	}
}

entity::Dummy* Scene::create_dummy() {
	entity::Dummy* dummy = new entity::Dummy;
	dummies_.push_back(dummy);

	entities_.push_back(dummy);

	return dummy;
}

entity::Dummy* Scene::create_dummy(const std::string& name) {
	entity::Dummy* dummy = create_dummy();

	add_named_entity(dummy, name);

	return dummy;
}

Prop* Scene::create_prop(std::shared_ptr<shape::Shape> shape,
						 const material::Materials& materials) {
	Prop* prop = new Prop;

	if (shape->is_finite()) {
		finite_props_.push_back(prop);
	} else {
		infinite_props_.push_back(prop);
	}

	prop->set_shape_and_materials(shape, materials);

	entities_.push_back(prop);

	return prop;
}

Prop* Scene::create_prop(std::shared_ptr<shape::Shape> shape,
						 const material::Materials& materials,
						 const std::string& name) {
	Prop* prop = create_prop(shape, materials);

	add_named_entity(prop, name);

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

volume::Volume* Scene::create_height_volume() {
	volume_region_ = new volume::Height;

	entities_.push_back(volume_region_);

	return volume_region_;
}

volume::Volume* Scene::create_homogenous_volume() {
	volume_region_ = new volume::Homogeneous;

	entities_.push_back(volume_region_);

	return volume_region_;
}

volume::Volume* Scene::create_grid_volume(Texture_ptr grid) {
	volume_region_ = new volume::Grid(grid);

	entities_.push_back(volume_region_);

	return volume_region_;
}

void Scene::add_extension(entity::Entity* extension) {
	extensions_.push_back(extension);

	entities_.push_back(extension);
}

void Scene::add_extension(entity::Entity* extension, const std::string& name) {
	add_extension(extension);

	add_named_entity(extension, name);
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

void Scene::add_named_entity(entity::Entity* entity, const std::string& name) {
	if (!entity || name.empty()) {
		return;
	}

	if (named_entities_.find(name) != named_entities_.end()) {
		return;
	}

	named_entities_[name] = entity;
}

}
