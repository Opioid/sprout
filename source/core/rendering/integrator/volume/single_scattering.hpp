#pragma once

#include "volume_integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace rendering { namespace integrator { namespace volume {

class alignas(64) Single_scattering : public Integrator {

public:

	struct Settings {
		float step_size;

		Light_sampling light_sampling;
	};

	Single_scattering(rnd::Generator& rng, const take::Settings& take_settings,
					  const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 transmittance(const Ray& ray, const Volume& volume,
								 Worker& worker) override final;

	virtual float4 li(const Ray& ray, bool primary_ray, const Volume& volume,
					  Worker& worker, float3& transmittance) override final;

	virtual size_t num_bytes() const override final;

private:

	float3 estimate_direct_light(const float3& w, const float3& p, float time,
								 const Volume& volume, Worker& worker);

	float3 evaluate_light(const Light& light, float light_weight,
						  const float3& w, const float3& p,
						  float time, uint32_t sampler_dimension,
						  const Volume& volume, Worker& worker);

	const Settings settings_;

	sampler::Random sampler_;
};

class Single_scattering_factory : public Factory {

public:

	Single_scattering_factory(const take::Settings& take_settings, uint32_t num_integrators,
							  float step_size, Light_sampling light_sampling);

	~Single_scattering_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Single_scattering* integrators_;

	Single_scattering::Settings settings_;
};

}}}
