#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_HPP

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector4.hpp"

namespace rendering {

class Worker;

namespace integrator::volume {

class Integrator : public integrator::Integrator {

public:

	Integrator(rnd::Generator& rng, take::Settings const& settings);
	virtual ~Integrator();

	virtual float3 transmittance(Ray const& ray, Worker& worker) = 0;

	virtual bool integrate(Ray& ray, Intersection& intersection,
						   Sampler_filter filter, Worker& worker,
						   float3& li, float3& transmittance) = 0;
};

class Factory {

public:

	Factory(take::Settings const& settings,	uint32_t num_integrators);
	virtual ~Factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const = 0;

protected:

	take::Settings const& take_settings_;

	uint32_t const num_integrators_;
};

}}

#endif
