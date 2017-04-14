#pragma once

#include "surface_integrator.hpp"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_golden_ratio.hpp"
// #include "sampler/sampler_halton.hpp"
// #include "sampler/sampler_ld.hpp"
// #include "sampler/sampler_scrambled_hammersley.hpp"
// #include "sampler/sampler_sobol.hpp"
#include "sampler/sampler_random.hpp"

namespace rendering { namespace integrator { namespace surface {

class AO : public Integrator {

public:

	struct Settings {
		uint32_t num_samples;
		float num_samples_reciprocal;
		float radius;
	};

	AO(rnd::Generator& rng, const take::Settings& take_settings, const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float4 li(Worker& worker, Ray& ray, Intersection& intersection) final override;

	virtual size_t num_bytes() const final override;

private:

	Settings settings_;

	sampler::Golden_ratio sampler_;
};

class AO_factory : public Factory {

public:

	AO_factory(const take::Settings& settings, uint32_t num_samples, float radius);

	virtual Integrator* create(rnd::Generator& rng) const;

private:

	AO::Settings settings_;
};

}}}
