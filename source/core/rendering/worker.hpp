#pragma once

#include "rectangle.hpp"
#include "base/math/vector.hpp"
#include "base/math/ray.hpp"

namespace scene {

class Scene;
struct Intersection;

}

namespace camera {

class Camera;

}

namespace rendering {

class Surface_integrator;

class Worker {
public:

	Worker(Surface_integrator* surface_integrator);

	void render(const scene::Scene& scene, const camera::Camera& camera, const Rectui& tile);

	math::float3 li(uint32_t subsample, math::Oray& ray) const;

	bool intersect(math::Oray& ray, scene::Intersection& intersection) const;

	bool visibility(const math::Oray& ray) const;

private:

	Surface_integrator* surface_integrator_;
	const scene::Scene* scene_;
};

}
