#ifndef SU_CORE_RENDERING_INTEGRATRO_SURFACE_AO_HPP
#define SU_CORE_RENDERING_INTEGRATRO_SURFACE_AO_HPP

#include "surface_integrator.hpp"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_golden_ratio.hpp"
// #include "sampler/sampler_halton.hpp"
// #include "sampler/sampler_ld.hpp"
// #include "sampler/sampler_scrambled_hammersley.hpp"
// #include "sampler/sampler_sobol.hpp"
#include "sampler/sampler_random.hpp"

namespace rendering::integrator::surface {

class alignas(64) AO final : public Integrator {

public:

	struct Settings {
		uint32_t num_samples;
		float num_samples_reciprocal;
		float radius;
	};

	AO(rnd::Generator& rng, const take::Settings& take_settings, const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float4 li(Ray& ray, Intersection& intersection, Worker& worker) override final;

	virtual size_t num_bytes() const override final;

private:

	const Settings settings_;

	sampler::Golden_ratio sampler_;
};

class AO_factory final : public Factory {

public:

	AO_factory(const take::Settings& settings, uint32_t num_integrators,
			   uint32_t num_samples, float radius);

	~AO_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const;

private:

	AO* integrators_;

	AO::Settings settings_;
};

}

#endif
