#ifndef SU_CORE_RENDERING_INTEGRATOR_SURFACE_SUB_INTEGRATOR_HPP
#define SU_CORE_RENDERING_INTEGRATOR_SURFACE_SUB_INTEGRATOR_HPP

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector3.hpp"

namespace scene {

namespace material { class BSSRDF; }

namespace prop { class Prop; }

}

namespace sampler { class Sampler; }

namespace rendering {

class Worker;

namespace integrator::surface::sub {

class Integrator : public integrator::Integrator {

public:

	Integrator(rnd::Generator& rng, const take::Settings& settings);
	virtual ~Integrator();

	virtual float3 li(Worker& worker, const Ray& ray, Intersection& intersection,
					  const Material_sample& sample, Sampler_filter filter,
					  Bxdf_sample& sample_result) = 0;

protected:

	float3 estimate_direct_light(const float3& position, const Prop* prop,
								 const scene::material::BSSRDF& bssrdf,
								 float time, uint32_t depth,
								 sampler::Sampler& sampler, Worker& worker);

	float3 estimate_indirect_light(const float3& position, const Prop* prop,
								   const scene::material::BSSRDF& bssrdf,
								   float time, uint32_t depth,
								   sampler::Sampler& sampler, Worker& worker);
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
