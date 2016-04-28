#pragma once

#include "scene/scene_worker.hpp"
#include "base/math/vector.hpp"
#include "base/math/rectangle.hpp"
#include "base/math/random/generator.hpp"

namespace sampler { class Sampler; }

namespace rendering {

namespace integrator {

namespace surface {

class Integrator;
class Integrator_factory;

}

namespace volume {

class Integrator;
class Integrator_factory;

}}

class Worker : public scene::Worker {
public:

	Worker();
	~Worker();

	void init(uint32_t id, const scene::Scene& scene, const math::random::Generator& rng,
			  integrator::surface::Integrator_factory& surface_integrator_factory,
			  integrator::volume::Integrator_factory& volume_integrator_factory,
			  sampler::Sampler& sampler);

	math::float4 li(scene::Ray& ray);
	math::float3 surface_li(scene::Ray& ray);
	math::float4 volume_li(const scene::Ray& ray, math::float3& transmittance);

	math::float3 transmittance(const scene::Ray& ray);

protected:

	integrator::surface::Integrator* surface_integrator_;
	integrator::volume::Integrator*  volume_integrator_;
	sampler::Sampler* sampler_;

private:

	math::random::Generator rng_;
};

}
