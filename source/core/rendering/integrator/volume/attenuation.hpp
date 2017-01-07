#pragma once

#include "rendering/integrator/volume/volume_integrator.hpp"

namespace rendering { namespace integrator { namespace volume {

class Attenuation : public Integrator {

public:

	Attenuation(const take::Settings& take_settings, rnd::Generator& rng);

	virtual void prepare(const scene::Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float3 transmittance(Worker& worker, const Ray& ray,
								 const Volume& volume) final override;

	virtual float4 li(Worker& worker, const Ray& ray, const Volume& volume,
					  float3& transmittance) final override;

	virtual size_t num_bytes() const final override;
};

class Attenuation_factory : public Factory {

public:

	Attenuation_factory(const take::Settings& settings);

	virtual Integrator* create(rnd::Generator& rng) const;
};

}}}
