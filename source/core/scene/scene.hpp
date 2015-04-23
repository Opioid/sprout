#pragma once

#include "bvh/scene_bvh_tree.hpp"
#include "shape/node_stack.hpp"
#include "base/math/ray.hpp"
#include <vector>

namespace scene {

namespace surrounding {

class Surrounding;

}

namespace light {

class Light;
class Shape_light;

}

class Prop;
struct Intersection;

class Scene {
public:

	Scene();
	~Scene();

	bool intersect(math::Oray& ray, Node_stack& node_stack, Intersection& intersection) const;
	bool intersect_p(const math::Oray& ray, Node_stack& node_stack) const;

	void compile();

	const surrounding::Surrounding* surrounding() const;
	void set_surrounding(const surrounding::Surrounding* surrounding);

	Prop* create_prop();

	const std::vector<light::Light*>& lights() const;

	light::Light* montecarlo_light(float random, float& pdf) const;

	light::Shape_light* create_shape_light();

private:

	const surrounding::Surrounding* surrounding_;

	bvh::Tree bvh_;

	std::vector<Prop*> props_;

	std::vector<light::Light*> lights_;

};

}
