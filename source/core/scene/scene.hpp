#pragma once

#include "scene_worker.hpp"
#include "bvh/scene_bvh_builder.hpp"
#include "bvh/scene_bvh_tree.hpp"
#include "base/math/distribution/distribution_1d.hpp"
#include <map>
#include <vector>
#include <memory>

namespace thread { class Pool; }

namespace scene {

namespace shape { class Shape; }

namespace entity {

class Entity;
class Dummy;

}

namespace material { class Material; }

namespace light {

class Light;
class Prop_light;
class Prop_image_light;

}

namespace animation {

class Animation;
class Stage;

}

namespace volume { class Volume; }

struct Intersection;
struct Ray;
class Prop;


class Scene {

public:

	Scene();
	~Scene();

	const math::aabb& aabb() const;

	bool intersect(scene::Ray& ray, shape::Node_stack& node_stack,
				   Intersection& intersection) const;

	bool intersect_p(const scene::Ray& ray, shape::Node_stack& node_stack) const;

	float opacity(const scene::Ray& ray, Worker& worker,
				  material::Sampler_settings::Filter filter) const;

	float tick_duration() const;
	float simulation_time() const;

	entity::Entity* entity(size_t index) const;
	entity::Entity* entity(const std::string& name) const;

	const std::vector<light::Light*>& lights() const;

	const light::Light* montecarlo_light(float random, float& pdf) const;

	const volume::Volume* volume_region() const;

	void tick(thread::Pool& thread_pool);
	float seek(float time, thread::Pool& thread_pool);

	void compile();

	entity::Dummy* create_dummy();
	entity::Dummy* create_dummy(const std::string& name);

	Prop* create_prop(std::shared_ptr<shape::Shape> shape);
	Prop* create_prop(std::shared_ptr<shape::Shape> shape, const std::string& name);

	light::Prop_light* create_prop_light(Prop* prop, uint32_t part);
	light::Prop_image_light* create_prop_image_light(Prop* prop, uint32_t part);

	volume::Volume* create_volume(const float3& absorption, const float3& scattering);

	void add_extension(entity::Entity* extension);
	void add_extension(entity::Entity* extension, const std::string& name);

	void add_material(std::shared_ptr<material::Material> material);
    void add_animation(std::shared_ptr<animation::Animation> animation);

    void create_animation_stage(entity::Entity* entity, animation::Animation* animation);

private:

	void add_named_entity(entity::Entity* entity, const std::string& name);

	float tick_duration_;
	float simulation_time_;

    bvh::Builder builder_;
	bvh::Tree bvh_;

	std::vector<entity::Dummy*> dummies_;

	std::vector<Prop*> finite_props_;
	std::vector<Prop*> infinite_props_;

	std::vector<light::Light*> lights_;

	std::vector<entity::Entity*> extensions_;

	std::vector<entity::Entity*> entities_;

	std::map<std::string, entity::Entity*> named_entities_;

	std::vector<float> light_powers_;

	math::Distribution_1D light_distribution_;

	volume::Volume* volume_region_;

	std::vector<std::shared_ptr<material::Material>> materials_;

    std::vector<std::shared_ptr<animation::Animation>> animations_;

    std::vector<animation::Stage> animation_stages_;
};

}
