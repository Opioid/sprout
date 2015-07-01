#pragma once

#include "scene/animation/animation_stage.hpp"
#include "scene/bvh/scene_bvh_builder.hpp"
#include "scene/bvh/scene_bvh_tree.hpp"
#include "scene/shape/node_stack.hpp"
#include "base/math/ray.hpp"
#include "base/math/cdf.hpp"
#include <vector>
#include <memory>

namespace scene {

namespace entity {

class Entity;

}

namespace light {

class Light;
class Image_light;
class Prop_light;
class Uniform_light;

}

namespace animation {

class Animation;

}

class Prop;
struct Intersection;

class Scene {
public:

	Scene();
	~Scene();

	bool intersect(math::Oray& ray, Node_stack& node_stack, Intersection& intersection) const;
	bool intersect_p(const math::Oray& ray, Node_stack& node_stack) const;

	float opacity(const math::Oray& ray, Node_stack& node_stack, const image::sampler::Sampler_2D& sampler) const;

	float tick_duration() const;
	float simulation_time() const;

	void tick();

	Prop* create_prop();

	const std::vector<light::Light*>& lights() const;

	const light::Light* montecarlo_light(float random, float& pdf) const;

	light::Image_light* create_image_light();
	light::Prop_light* create_prop_light();
	light::Uniform_light* create_uniform_light();

    void add_animation(std::shared_ptr<animation::Animation> animation);

    void create_animation_stage(entity::Entity* entity, animation::Animation* animation);

private:

	void compile();

	float tick_duration_;
	float simulation_time_;

    bvh::Builder builder_;
	bvh::Tree bvh_;

	std::vector<Prop*> props_;

	std::vector<light::Light*> lights_;

	math::CDF light_cdf_;

    std::vector<std::shared_ptr<animation::Animation>> animations_;

    std::vector<animation::Stage> animation_stages_;
};

}
