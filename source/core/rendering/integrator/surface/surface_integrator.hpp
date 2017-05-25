#pragma once

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector4.hpp"

namespace rendering {

class Worker;

namespace integrator { namespace surface {

class Integrator : public integrator::Integrator {

public:

	Integrator(rnd::Generator& rng, const take::Settings& settings);
	virtual ~Integrator();

	virtual float4 li(Worker& worker, Ray& ray, Intersection& intersection) = 0;

protected:

	bool resolve_mask(Worker& worker, Ray& ray, Intersection& intersection, Sampler_filter filter);

	bool intersect_and_resolve_mask(Worker& worker, Ray& ray, Intersection& intersection,
									Sampler_filter filter);
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

}}}
