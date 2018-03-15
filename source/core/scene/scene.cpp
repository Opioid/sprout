#include "scene.hpp"
#include "scene_constants.hpp"
#include "scene_ray.hpp"
#include "scene_worker.hpp"
#include "bvh/scene_bvh_builder.inl"
#include "animation/animation.hpp"
#include "animation/animation_stage.hpp"
#include "entity/dummy.hpp"
#include "light/prop_light.hpp"
#include "light/prop_image_light.hpp"
#include "prop/prop.hpp"
#include "prop/prop_intersection.hpp"
#include "shape/shape.hpp"
#include "volume/volume.hpp"
#include "image/texture/texture.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/quaternion.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/spectrum/rgb.hpp"

#include "base/debug/assert.hpp"

namespace scene {

Scene::Scene(const take::Settings& settings) : take_settings_(settings) {
	dummies_.reserve(16);
	finite_props_.reserve(16);
	infinite_props_.reserve(2);
	lights_.reserve(16);
	volumes_.reserve(16);
	infinite_volumes_.reserve(1);
	extensions_.reserve(16);
	entities_.reserve(16);
	light_powers_.reserve(16);
	materials_.reserve(16);
	animations_.reserve(16);
	animation_stages_.reserve(16);
}

Scene::~Scene() {
	for (auto e : extensions_) {
		delete e;
	}

	for (auto v : volumes_) {
		delete v;
	}

	for (auto v : infinite_volumes_) {
		delete v;
	}

	// Normally lights_ should never be empty; containing null_light instead
	// But it can happen for partially constructed scenes,
	// and we still don't want the destructor to crash.
	if (!lights_.empty() && lights_[0] != &null_light_) {
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
	return prop_bvh_.aabb();
}

bool Scene::intersect(Ray& ray, Node_stack& node_stack, prop::Intersection& intersection) const {
	return prop_bvh_.intersect(ray, node_stack, intersection);
}

bool Scene::intersect_p(const Ray& ray, Node_stack& node_stack) const {
	return prop_bvh_.intersect_p(ray, node_stack);
}

float Scene::opacity(const Ray& ray, Sampler_filter filter, const Worker& worker) const {
	if (has_masked_material_) {
		return prop_bvh_.opacity(ray, filter, worker);
	}

	return prop_bvh_.intersect_p(ray, worker.node_stack()) ? 1.f : 0.f;
}

float3 Scene::thin_absorption(const Ray& ray, Sampler_filter filter, const Worker& worker) const {
	if (has_tinted_shadow_) {
		return prop_bvh_.thin_absorption(ray, filter, worker);
	}

	return float3(opacity(ray, filter, worker));
}

const volume::Volume* Scene::closest_volume_segment(Ray& ray, Node_stack& node_stack,
													float& epsilon) const {
	const float original_max_t = ray.max_t;

	ray.max_t = Ray_max_t;

	float local_epsilon;
	bool inside;
	const volume::Volume* volume = volume_bvh_.intersect(ray, node_stack, true, 
														 local_epsilon, inside);

	if (!volume) {
		return nullptr;
	}

	if (inside) {
		if (!volume->shape()->is_finite()) {
			volume_bvh_.intersect(ray, node_stack, false, local_epsilon, inside);
		}

		if (ray.max_t > original_max_t) {
			ray.max_t = original_max_t;
		}

		epsilon = std::max(local_epsilon, 0.0001f);
		return volume;
	}

	const float first = ray.max_t;
	const float next = ray.max_t + (local_epsilon * take_settings_.ray_offset_factor);

	if (next >= original_max_t) {
		return nullptr;
	}

	entity::Composed_transformation temp;
	const auto& transformation = volume->transformation_at(ray.time, temp);

	ray.min_t = next;
	ray.max_t = Ray_max_t;

	if (!volume->shape()->intersect(transformation, ray, node_stack, local_epsilon, inside)) {
		return nullptr;
	}

	ray.min_t = first;

	if (ray.max_t > original_max_t) {
		ray.max_t = original_max_t;
	}

	epsilon = std::max(local_epsilon, 0.0001f);
	return volume;
}

float Scene::tick_duration() const {
	return static_cast<float>(tick_duration_);
}

float Scene::simulation_time() const {
	return static_cast<float>(simulation_time_);
}

uint64_t Scene::current_tick() const {
	return current_tick_;
}

entity::Entity* Scene::entity(size_t index) const {
	if (index >= entities_.size()) {
		return nullptr;
	}

	return entities_[index];
}

entity::Entity* Scene::entity(std::string_view name) const {
	auto e = named_entities_.find(name);
	if (named_entities_.end() == e) {
		return nullptr;
	}

	return e->second;
}

const std::vector<light::Light*>& Scene::lights() const {
	return lights_;
}

Scene::Light Scene::light(uint32_t id) const {
	// If the assert doesn't hold it would pose a problem,
	// but I think it is more efficient to handle those cases outside or implicitely.
	SOFT_ASSERT(!lights_.empty() && light::Light::is_light(id));

	const float pdf = light_distribution_.pdf(id);
	return { *lights_[id], pdf };
}

Scene::Light Scene::random_light(float random) const {
	SOFT_ASSERT(!lights_.empty());

	const auto l = light_distribution_.sample_discrete(random);

	SOFT_ASSERT(l.offset < static_cast<uint32_t>(lights_.size()));

	return { *lights_[l.offset], l.pdf };
}

const volume::Volume* Scene::volume_region() const {
	if (!volumes_.empty()) {
		return volumes_[0];
	}

	return nullptr;
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

	++current_tick_;
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

	const double first_tick_d = time_d - tick_offset_d;

	const float first_tick = static_cast<float>(first_tick_d);

	for (auto a : animations_) {
		a->seek(first_tick);
	}

	for (auto& s : animation_stages_) {
		s.update();
	}

	simulation_time_ = first_tick_d;

	current_tick_ = static_cast<uint64_t>(first_tick_d / tick_duration_);

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
		has_tinted_shadow_   = has_tinted_shadow_   || p->has_tinted_shadow();
	}

	for (auto p : infinite_props_) {
		p->calculate_world_transformation();
		has_masked_material_ = has_masked_material_ || p->has_masked_material();
		has_tinted_shadow_   = has_tinted_shadow_   || p->has_tinted_shadow();
	}

	for (auto v : volumes_) {
		v->calculate_world_transformation();
	}

	for (auto v : infinite_volumes_) {
		v->calculate_world_transformation();
	}

	// rebuild prop BVH
	prop_builder_.build(prop_bvh_.tree(), finite_props_);
	prop_bvh_.set_infinite_props(infinite_props_);

	// rebuild volume BVH
	volume_builder_.build(volume_bvh_.tree(), volumes_);
	volume_bvh_.set_infinite_props(infinite_volumes_);

	for (auto v : volumes_) {
		v->set_scene_aabb(prop_bvh_.aabb());
	}

	// resort lights PDF
	light_powers_.clear();

	for (uint32_t i = 0, len = static_cast<uint32_t>(lights_.size()); i < len; ++i) {
		auto l = lights_[i];
		l->prepare_sampling(i, pool);
		light_powers_.push_back(std::sqrt(spectrum::luminance(l->power(prop_bvh_.aabb()))));
	}

	light_distribution_.init(light_powers_.data(), light_powers_.size());
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

Prop* Scene::create_prop(const Shape_ptr& shape, const Materials& materials) {
	prop::Prop* prop;

	if (1 == materials.size() && materials[0]->is_volumetric()) {
		volume::Volume* volume = new volume::Volume;

		if (shape->is_finite()) {
			volumes_.push_back(volume);
		} else {
			infinite_volumes_.push_back(volume);
		}

		prop = volume;
	} else {
		prop = new prop::Prop;

		if (shape->is_finite()) {
			finite_props_.push_back(prop);
		} else {
			infinite_props_.push_back(prop);
		}
	}

	prop->set_shape_and_materials(shape, materials);

	entities_.push_back(prop);

	return prop;
}

prop::Prop* Scene::create_prop(const Shape_ptr& shape, const Materials& materials,
							   const std::string& name) {
	prop::Prop* prop = create_prop(shape, materials);

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

void Scene::add_extension(Entity* extension) {
	extensions_.push_back(extension);

	entities_.push_back(extension);
}

void Scene::add_extension(Entity* extension, const std::string& name) {
	add_extension(extension);

	add_named_entity(extension, name);
}

void Scene::add_material(const Material_ptr& material) {
	materials_.push_back(material);
}

void Scene::add_animation(const std::shared_ptr<animation::Animation>& animation) {
    animations_.push_back(animation);
}

void Scene::create_animation_stage(Entity* entity, animation::Animation* animation) {
    animation_stages_.push_back(animation::Stage(entity, animation));
}

size_t Scene::num_bytes() const {
	size_t num_bytes = 0;

	for (auto p : finite_props_) {
		num_bytes += p->num_bytes();
	}

	for (auto p : infinite_props_) {
		num_bytes += p->num_bytes();
	}

	return num_bytes + sizeof(*this);
}

void Scene::add_named_entity(Entity* entity, const std::string& name) {
	if (!entity || name.empty()) {
		return;
	}

	if (named_entities_.find(name) != named_entities_.end()) {
		return;
	}

	named_entities_[name] = entity;
}

}
