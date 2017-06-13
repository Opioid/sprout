#pragma once

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector3.hpp"

namespace scene {

namespace material { class BSSRDF; }

class Prop;

}


namespace sampler { class Sampler; }

namespace rendering {

class Worker;

namespace integrator { namespace surface { namespace sub {

class Integrator : public integrator::Integrator {

public:

	Integrator(rnd::Generator& rng, const take::Settings& settings);
	virtual ~Integrator();

	virtual float3 li(Worker& worker, const Ray& ray, const Intersection& intersection) = 0;

	virtual float3 li(Worker& worker, Ray& ray, Intersection& intersection,
					  sampler::Sampler& sampler, Sampler_filter filter,
					  Bxdf_result& sample_result) = 0;

protected:

	float3 estimate_direct_light(const float3& position, const scene::Prop* prop,
								 const scene::material::BSSRDF& bssrdf, float time,
								 sampler::Sampler& sampler, Worker& worker);
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

