#pragma once

#include "bvh/scene_bvh_tree.hpp"
#include "shape/node_stack.hpp"
#include "base/math/ray.hpp"
#include "base/math/cdf.hpp"
#include <vector>

namespace scene {

namespace light {

class Light;
class Image_light;
class Prop_light;
class Uniform_light;

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

	void compile();

	Prop* create_prop();

	const std::vector<light::Light*>& lights() const;

	const light::Light* montecarlo_light(float random, float& pdf) const;

	light::Image_light* create_image_light();
	light::Prop_light* create_prop_light();
	light::Uniform_light* create_uniform_light();

private:

	bvh::Tree bvh_;

	std::vector<Prop*> props_;

	std::vector<light::Light*> lights_;

	math::CDF light_cdf_;
};

}
