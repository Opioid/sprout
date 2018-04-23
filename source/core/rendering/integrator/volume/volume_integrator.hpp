#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_HPP

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector4.hpp"

namespace rendering {

class Worker;

namespace integrator::volume {

class Integrator : public integrator::Integrator {

public:

	Integrator(rnd::Generator& rng, const take::Settings& settings);
	virtual ~Integrator();

	virtual float3 transmittance(const Ray& ray, Worker& worker) = 0;

	virtual bool integrate(Ray& ray, Intersection& intersection,
						   Sampler_filter filter, Worker& worker,
						   float3& li, float3& transmittance) = 0;
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

}}

#endif
