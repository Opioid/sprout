#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_RAY_MARCHING_SINGLE_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_RAY_MARCHING_SINGLE_HPP

#include "volume_integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace scene::entity { struct Composed_transformation; }

namespace rendering::integrator::volume {

class alignas(64) Ray_marching_single : public Integrator {

public:

	struct Settings {
		float step_size;
		float step_probability;
	};

	Ray_marching_single(rnd::Generator& rng, const take::Settings& take_settings,
						const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 transmittance(const Ray& ray, Worker& worker) override final;

	virtual bool integrate(Ray& ray, Intersection& intersection,
						   Sampler_filter filter, Worker& worker,
						   float3& li, float3& transmittance) override final;

	virtual size_t num_bytes() const override final;

private:

	float3 estimate_direct_light(const Ray& ray, const float3& position, Worker& worker);

	const Settings settings_;

	sampler::Random sampler_;
};

class Ray_marching_single_factory : public Factory {

public:

	Ray_marching_single_factory(const take::Settings& take_settings, uint32_t num_integrators,
								float step_size, float step_probability);

	virtual ~Ray_marching_single_factory() override;

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Ray_marching_single* integrators_;

	Ray_marching_single::Settings settings_;
};

}

#endif
