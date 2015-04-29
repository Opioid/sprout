#pragma once

#include "base/math/vector.hpp"
#include "base/math/ray.hpp"
#include "base/math/random/generator.hpp"

namespace take {

struct Settings;

}

namespace scene {

struct Intersection;

}

namespace rendering {

class Worker;

class Integrator {
public:

	Integrator(const take::Settings& settings, math::random::Generator& rng);

	virtual void start_new_pixel(uint32_t num_samples);

protected:

	const take::Settings& take_settings_;
	math::random::Generator& rng_;
};

class Surface_integrator : public Integrator {
public:

	Surface_integrator(const take::Settings& settings, math::random::Generator& rng);

	virtual math::float3 li(Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) = 0;
};

class Surface_integrator_factory {
public:

	Surface_integrator_factory(const take::Settings& settings);

	virtual Surface_integrator* create(math::random::Generator& rng) const = 0;

protected:

	const take::Settings& take_settings_;
};

}
