#ifndef SU_RENDERING_INTEGRATOR_SURFACE_INTEGRATOR_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_INTEGRATOR_HPP

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector.hpp"

namespace rendering {

class Worker;

namespace integrator::surface {

class Integrator : public integrator::Integrator {

public:

	Integrator(rnd::Generator& rng, const take::Settings& settings);
	virtual ~Integrator();

	virtual float3 li(Ray& ray, Intersection& intersection, Worker& worker) = 0;
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

#endif
