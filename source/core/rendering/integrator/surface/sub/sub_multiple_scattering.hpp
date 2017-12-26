#ifndef SU_CORE_RENDERING_INTEGRATOR_SURFACE_SUB_MULTIPLE_SCATTERING_HPP
#define SU_CORE_RENDERING_INTEGRATOR_SURFACE_SUB_MULTIPLE_SCATTERING_HPP

#include "sub_integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace rendering::integrator::surface::sub {

class alignas(64) Multiple_scattering : public Integrator {

public:

	struct Settings {
		float step_size;
	};

	Multiple_scattering(rnd::Generator& rng, const take::Settings& take_settings,
						const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 li(const Ray& ray, Intersection& intersection,
					  const Material_sample& sample, Sampler_filter filter,
					  Worker& worker, Bxdf_sample& sample_result) override final;

	virtual size_t num_bytes() const override final;

private:

	const Settings settings_;

	sampler::Random sampler_;
};

class Multiple_scattering_factory : public Factory {

public:

	Multiple_scattering_factory(const take::Settings& take_settings, uint32_t num_integrators,
							    float step_size);

	~Multiple_scattering_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Multiple_scattering* integrators_;

	Multiple_scattering::Settings settings_;
};

}

#endif
