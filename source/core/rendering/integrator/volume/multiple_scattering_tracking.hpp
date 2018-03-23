#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_MULTIPLE_SCATTERING_TRACKING_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_MULTIPLE_SCATTERING_TRACKING_HPP

#include "volume_integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace scene::entity { struct Composed_transformation; }

namespace rendering::integrator::volume {

class alignas(64) Multiple_scattering_tracking : public Integrator {

public:

	struct Settings {

	};

	Multiple_scattering_tracking(rnd::Generator& rng, const take::Settings& take_settings,
					  const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 transmittance(const Ray& ray, const Volume& volume,
								 const Worker& worker) override final;

	virtual float3 li(const Ray& ray, const Volume& volume,
					  Worker& worker, float3& transmittance) override final;

	virtual float3 transmittance(const Ray& ray, const Intersection& intersection,
								 const Worker& worker) override final;

	virtual bool integrate(Ray& ray, Intersection& intersection,
						   const Material_sample& material_sample, Worker& worker,
						   float3& li, float3& transmittance, float3& weight) override final;

	virtual size_t num_bytes() const override final;

private:

	float3 spectral_stuff(const Ray& ray, const Transformation& transformation,
						  const Material& material, uint32_t channel, float2 rr,
						  Worker& worker, float3& transmittance);

	float3 direct_light(const Ray& ray, const float3& position, Worker& worker);

	float3 direct_light(const Ray& ray, const float3& position, const Intersection& intersection,
						const Material_sample& material_sample, Worker& worker);

	const Settings settings_;

	sampler::Random sampler_;
};

class Multiple_scattering_tracking_factory : public Factory {

public:

	Multiple_scattering_tracking_factory(const take::Settings& take_settings, uint32_t num_integrators);

	~Multiple_scattering_tracking_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Multiple_scattering_tracking* integrators_;

	Multiple_scattering_tracking::Settings settings_;
};

}

#endif
