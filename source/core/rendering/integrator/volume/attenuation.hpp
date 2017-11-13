#pragma once

#include "rendering/integrator/volume/volume_integrator.hpp"

namespace rendering { namespace integrator { namespace volume {

class alignas(64) Attenuation : public Integrator {

public:

	Attenuation(rnd::Generator& rng, const take::Settings& take_settings);

	virtual void prepare(const scene::Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 transmittance(const Ray& ray, const Volume& volume,
								 const Worker& worker) override final;

	virtual float4 li(const Ray& ray, bool primary_ray, const Volume& volume,
					  const Worker& worker, float3& transmittance) override final;

	virtual size_t num_bytes() const override final;
};

class Attenuation_factory : public Factory {

public:

	Attenuation_factory(const take::Settings& settings, uint32_t num_integrators);

	~Attenuation_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Attenuation* integrators_;
};

}}}
