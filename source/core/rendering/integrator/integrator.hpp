#pragma once

#include "base/math/vector.hpp"
#include "base/math/ray.hpp"

namespace scene {

struct Intersection;

}

namespace rendering {

class Worker;

class Surface_integrator {
public:

	virtual math::float3 li(const Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) const = 0;
};

class Surface_integrator_factory {
public:

	virtual Surface_integrator* create(uint32_t id) const = 0;
};

}
