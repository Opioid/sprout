#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_EMISSION_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_EMISSION_HPP

#include "rendering/integrator/volume/volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Emission final : public Integrator {

public:

	struct Settings {
		float step_size;
	};

	Emission(rnd::Generator& rng, const take::Settings& take_settings,
			 const Settings& settings);

	virtual void prepare(const scene::Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 transmittance(const Ray& ray, Worker& worker) override final;

	virtual bool integrate(Ray& ray, Intersection& intersection,
						   Sampler_filter filter, Worker& worker,
						   float3& li, float3& transmittance, float3& weight) override final;

	virtual size_t num_bytes() const override final;

	const Settings settings_;
};

class Emission_factory : public Factory {

public:

	Emission_factory(const take::Settings& settings, uint32_t num_integrators, float step_size);

	virtual ~Emission_factory() override;

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Emission* integrators_;

	const Emission::Settings settings_;
};

}

#endif
