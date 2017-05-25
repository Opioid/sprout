#pragma once

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector3.hpp"

namespace rendering {

class Worker;

namespace integrator { namespace surface { namespace sub {

class Integrator : public integrator::Integrator {

public:

	Integrator(rnd::Generator& rng, const take::Settings& settings);
	virtual ~Integrator();

	virtual float3 li(Worker& worker, const Ray& ray, const Intersection& intersection) = 0;
};

class Factory {

public:

	Factory(const take::Settings& settings,	uint32_t num_integrators);

	virtual ~Factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const = 0;

protected:

	const take::Settings& take_settings_;

	const uint32_t num_integrators_;
};

}}}}

