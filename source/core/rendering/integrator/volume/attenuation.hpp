#pragma once

#include "rendering/integrator/volume/volume_integrator.hpp"

namespace rendering { namespace integrator { namespace volume {

class Attenuation : public Integrator {

public:

	Attenuation(uint32_t num_samples_per_pixel,
				const take::Settings& take_settings,
				math::random::Generator& rng);

	virtual void resume_pixel(uint32_t sample, uint2 seed) final override;

	virtual float3 transmittance(Worker& worker, const scene::volume::Volume& volume,
								 const scene::Ray& ray) final override;

	virtual float4 li(Worker& worker, const scene::volume::Volume& volume,
					  const scene::Ray& ray, float3& transmittance) final override;
};

class Attenuation_factory : public Factory {

public:

	Attenuation_factory(const take::Settings& settings);

	virtual Integrator* create(uint32_t num_samples_per_pixel,
							   math::random::Generator& rng) const;
};

}}}
