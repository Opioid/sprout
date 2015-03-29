#pragma once

#include "base/math/vector.hpp"
#include "base/math/ray.hpp"
#include "base/math/random/generator.hpp"

namespace scene {

struct Intersection;

}

namespace rendering {

class Worker;

class Integrator {
public:

	Integrator(uint32_t id, math::random::Generator& rng);

	virtual void start_new_pixel(uint32_t num_samples);

protected:

	uint32_t id_;

	math::random::Generator& rng_;
};

class Surface_integrator : public Integrator {
public:

	Surface_integrator(uint32_t id, math::random::Generator& rng);

	virtual math::float3 li(const Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) = 0;
};

class Surface_integrator_factory {
public:

	virtual Surface_integrator* create(uint32_t id, math::random::Generator& rng) const = 0;
};

}
