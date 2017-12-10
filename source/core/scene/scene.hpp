#ifndef SU_CORE_SCENE_SCENE_HPP
#define SU_CORE_SCENE_SCENE_HPP

#include "scene_worker.hpp"
#include "material/material.hpp"
#include "light/null_light.hpp"
#include "bvh/scene_bvh_builder.hpp"
#include "prop/prop_bvh_wrapper.hpp"
#include "volume/volume_bvh_wrapper.hpp"
#include "take/take_settings.hpp"
#include "base/math/distribution/distribution_1d.hpp"
#include <map>
#include <vector>
#include <memory>

namespace thread { class Pool; }

namespace scene {

namespace shape { class Shape; }

namespace animation {

class Animation;
class Stage;

}

namespace entity {

class Entity;
class Dummy;

}

namespace light {

class Light;
class Prop_light;
class Prop_image_light;

}

namespace prop {

struct Intersection;
class Prop;

}

namespace volume { class Volume; }

struct Ray;

class Scene {

public:

	using Node_stack = shape::Node_stack;
	using Sampler_filter = material::Sampler_settings::Filter;
	using Shape_ptr = std::shared_ptr<shape::Shape>;

	Scene(const take::Settings& settings);
	~Scene();

	void finish();

	const math::AABB& aabb() const;

	bool intersect(Ray& ray, Node_stack& node_stack, prop::Intersection& intersection) const;

	bool intersect_p(const Ray& ray, Node_stack& node_stack) const;

	float opacity(const Ray& ray, Sampler_filter filter, const Worker& worker) const;

	float3 thin_absorption(const Ray& ray, Sampler_filter filter, const Worker& worker) const;

	const volume::Volume* closest_volume_segment(Ray& ray, Node_stack& node_stack,
												 float& epsilon) const;

	float tick_duration() const;
	float simulation_time() const;

	entity::Entity* entity(size_t index) const;
	entity::Entity* entity(std::string_view name) const;

	const std::vector<light::Light*>& lights() const;

	struct Light { const light::Light& ref; float pdf; };
	Light light(uint32_t id) const;
	Light random_light(float random) const;

	const volume::Volume* volume_region() const;

	void tick(thread::Pool& thread_pool);
	float seek(float time, thread::Pool& thread_pool);

	void compile(thread::Pool& pool);

	entity::Dummy* create_dummy();
	entity::Dummy* create_dummy(const std::string& name);

	prop::Prop* create_prop(const Shape_ptr& shape, const material::Materials& materials);

	prop::Prop* create_prop(const Shape_ptr& shape, const material::Materials& materials,
							const std::string& name);

	light::Prop_light* create_prop_light(prop::Prop* prop, uint32_t part);
	light::Prop_image_light* create_prop_image_light(prop::Prop* prop, uint32_t part);

	volume::Volume* create_height_volume(const Shape_ptr& shape);
	volume::Volume* create_homogenous_volume(const Shape_ptr& shape);
	volume::Volume* create_grid_volume(const Shape_ptr& shape, const Texture_ptr& grid);

	void add_extension(entity::Entity* extension);
	void add_extension(entity::Entity* extension, const std::string& name);

	void add_material(const material::Material_ptr& material);
	void add_animation(const std::shared_ptr<animation::Animation>& animation);

    void create_animation_stage(entity::Entity* entity, animation::Animation* animation);

	size_t num_bytes() const;

private:

	void add_named_entity(entity::Entity* entity, const std::string& name);

	const take::Settings take_settings_;

	double tick_duration_ = 1.0 / 60.0;
	double simulation_time_ = 0.0;

	bvh::Builder<prop::Prop> prop_builder_;
	prop::BVH_wrapper		 prop_bvh_;

	bvh::Builder<volume::Volume> volume_builder_;
	volume::BVH_wrapper          volume_bvh_;

	light::Null_light null_light_;

	bool has_masked_material_;
	bool has_tinted_shadow_;

	std::vector<entity::Dummy*> dummies_;

	std::vector<prop::Prop*> finite_props_;
	std::vector<prop::Prop*> infinite_props_;

	std::vector<light::Light*> lights_;

	std::vector<volume::Volume*> volumes_;

	std::vector<entity::Entity*> extensions_;

	std::vector<entity::Entity*> entities_;

	std::map<std::string, entity::Entity*, std::less<>> named_entities_;

	std::vector<float> light_powers_;

	math::Distribution_1D light_distribution_;

	std::vector<material::Material_ptr> materials_;

    std::vector<std::shared_ptr<animation::Animation>> animations_;

    std::vector<animation::Stage> animation_stages_;
};

}

#endif
