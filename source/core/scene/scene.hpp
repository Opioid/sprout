#pragma once

#include "base/math/ray.hpp"
#include <vector>

namespace scene {

class Prop;
struct Intersection;

class Scene {
public:

	~Scene();

	bool intersect(math::Oray& ray, Intersection& intersection) const;
	bool intersect_p(math::Oray& ray) const;

	Prop* create_prop();

private:

	std::vector<Prop*> props_;
};

}
