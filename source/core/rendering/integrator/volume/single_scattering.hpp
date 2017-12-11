#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_SINGLE_SCATTERING_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_SINGLE_SCATTERING_HPP

#include "volume_integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace scene::entity { struct Composed_transformation; }

namespace rendering::integrator::volume {

class alignas(64) Single_scattering : public Integrator {

public:

	struct Settings {
		float step_size;

		uint32_t max_indirect_bounces;

		Light_sampling light_sampling;
	};

	Single_scattering(rnd::Generator& rng, const take::Settings& take_settings,
					  const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 transmittance(const Ray& ray, const Volume& volume,
								 Worker& worker) override final;

	virtual float3 li(const Ray& ray, bool primary_ray, const Volume& volume,
					  Worker& worker, float3& transmittance) override final;

	virtual size_t num_bytes() const override final;

private:

	float3 estimate_direct_light(const float3& w, const float3& p, float time,
								 const Material& material, Worker& worker);

	float3 evaluate_light(const Light& light, float light_weight, const float3& w, const float3& p,
						  float time, uint32_t sampler_dimension,
						  const Material& material, Worker& worker);

	float3 estimate_indirect_light(const float3& w, const float3& p, const Ray& history,
								   const Material& material, Worker& worker);

	const Settings settings_;

	sampler::Random sampler_;
};

class Single_scattering_factory : public Factory {

public:

	Single_scattering_factory(const take::Settings& take_settings, uint32_t num_integrators,
							  float step_size, uint32_t max_indirect_bounces,
							  Light_sampling light_sampling);

	~Single_scattering_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Single_scattering* integrators_;

	Single_scattering::Settings settings_;
};

}

#endif
