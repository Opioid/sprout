#pragma once

#include "scene/scene_worker.hpp"
#include "base/math/vector.hpp"
#include "base/math/rectangle.hpp"
#include "base/random/generator.hpp"

namespace sampler {

class Sampler;
class Factory;

}

namespace rendering {

namespace integrator {

namespace surface {

class Integrator;
class Factory;

}

namespace volume {

class Integrator;
class Factory;

}}

class Worker : public scene::Worker {

public:

	Worker();
	~Worker();

	void init(uint32_t id, const scene::Scene& scene,
			  const rnd::Generator& rng,
			  integrator::surface::Factory& surface_integrator_factory,
			  integrator::volume::Factory& volume_integrator_factory,
			  sampler::Factory& sampler_factory);

	void prepare(uint32_t num_samples_per_pixel);

	float4 li(scene::Ray& ray);
	float3 surface_li(scene::Ray& ray);
	float4 volume_li(const scene::Ray& ray, float3& transmittance);

	float3 transmittance(const scene::Ray& ray);

	sampler::Sampler* sampler();

protected:

	integrator::surface::Integrator* surface_integrator_;
	integrator::volume::Integrator*  volume_integrator_;
	sampler::Sampler* sampler_;
	rnd::Generator rng_;
};

}
