#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_AERIAL_PERSPECTIVE_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_AERIAL_PERSPECTIVE_HPP

#include "volume_integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace scene::entity { struct Composed_transformation; }

namespace rendering::integrator::volume {

class alignas(64) Aerial_perspective : public Integrator {

public:

	struct Settings {
		float step_size;

		bool disable_shadows;
	};

	Aerial_perspective(rnd::Generator& rng, const take::Settings& take_settings,
					   const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 transmittance(const Ray& ray, const Volume& volume,
								 const Worker& worker) override final;

	virtual float3 li(const Ray& ray, const Volume& volume,
					  Worker& worker, float3& transmittance) override final;

	virtual float3 transmittance(const Ray& ray, const Worker& worker) override final;

	virtual size_t num_bytes() const override final;

private:

	float3 integrate_with_shadows(const Ray& ray, const Volume& volume,
								  Worker& worker, float3& transmittance);

	float3 integrate_without_shadows(const Ray& ray, const Volume& volume,
									 Worker& worker, float3& transmittance);

	const Material_sample& sample(const float3& wo, float time, const Material& material,
								  Sampler_filter filter, Worker& worker);

	const Settings settings_;

	sampler::Random sampler_;
};

class Aerial_perspective_factory : public Factory {

public:

	Aerial_perspective_factory(const take::Settings& take_settings, uint32_t num_integrators,
							   float step_size, bool shadows);

	~Aerial_perspective_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Aerial_perspective* integrators_;

	Aerial_perspective::Settings settings_;
};

}

#endif

