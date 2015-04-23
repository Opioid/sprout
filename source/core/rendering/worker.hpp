#pragma once

#include "rectangle.hpp"
#include "base/math/vector.hpp"
#include "base/math/ray.hpp"
#include "base/math/random/generator.hpp"

namespace sampler {

class Sampler;

}

namespace scene {

class Scene;
struct Intersection;

namespace camera {

class Camera;

}}

namespace rendering {

class Surface_integrator;
class Surface_integrator_factory;

class Worker {
public:

	Worker(uint32_t id, const math::random::Generator& rng,
		   Surface_integrator_factory& surface_integrator_factory, sampler::Sampler& sampler);
	~Worker();

	uint32_t id() const;

	void render(const scene::Scene& scene, const scene::camera::Camera& camera, const Rectui& tile);

	math::float3 li(uint32_t subsample, math::Oray& ray) const;

	bool intersect(math::Oray& ray, scene::Intersection& intersection) const;

	bool visibility(const math::Oray& ray) const;

	const scene::Scene& scene() const;

private:

	uint32_t id_;
	math::random::Generator rng_;
	Surface_integrator* surface_integrator_;
	sampler::Sampler* sampler_;
	const scene::Scene* scene_;
};

}
