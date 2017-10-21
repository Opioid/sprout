#pragma once

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector4.hpp"

namespace rendering {

class Worker;

namespace integrator::surface {

class Integrator : public integrator::Integrator {

public:

	Integrator(rnd::Generator& rng, const take::Settings& settings);
	virtual ~Integrator();

	virtual float4 li(Ray& ray, Intersection& intersection, Worker& worker) = 0;

protected:

	bool resolve_mask(Ray& ray, Intersection& intersection,
					  Sampler_filter filter, Worker& worker);

	bool intersect_and_resolve_mask(Ray& ray, Intersection& intersection,
									Sampler_filter filter, Worker& worker);
};

class Factory {

public:

	Factory(const take::Settings& settings);
	virtual ~Factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const = 0;

protected:

	const take::Settings& take_settings_;
};

}}
