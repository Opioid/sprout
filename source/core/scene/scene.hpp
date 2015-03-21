#pragma once

#include "bvh/scene_bvh_tree.hpp"
#include "base/math/ray.hpp"
#include <vector>

namespace scene {

namespace surrounding {

class Surrounding;

}

class Prop;
struct Intersection;

class Scene {
public:

	Scene();
	~Scene();

	bool intersect(math::Oray& ray, Intersection& intersection) const;
	bool intersect_p(const math::Oray& ray) const;

	void compile();

	const surrounding::Surrounding* surrounding() const;
	void set_surrounding(const surrounding::Surrounding* surrounding);

	Prop* create_prop();

private:

	const surrounding::Surrounding* surrounding_;

	bvh::Tree bvh_;

	std::vector<Prop*> props_;
};

}
