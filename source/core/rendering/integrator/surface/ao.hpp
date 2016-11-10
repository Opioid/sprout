#pragma once

#include "rendering/integrator/surface/surface_integrator.hpp"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_golden_ratio.hpp"
// #include "sampler/sampler_halton.hpp"
// #include "sampler/sampler_ld.hpp"
// #include "sampler/sampler_scrambled_hammersley.hpp"
// #include "sampler/sampler_sobol.hpp"
#include "sampler/sampler_random.hpp"
#include "image/texture/sampler/sampler_2d_linear.hpp"
#include "image/texture/sampler/address_mode.hpp"

namespace rendering { namespace integrator { namespace surface {

class Ao : public Integrator {

public:

	struct Settings {
		uint32_t num_samples;
		float num_samples_reciprocal;
		float radius;
	};

	Ao(uint32_t num_samples_per_pixel,
	   const take::Settings& take_settings,
	   rnd::Generator& rng,
	   const Settings& settings);

	virtual void resume_pixel(uint32_t sample, uint2 seed) final override;

	virtual float4 li(Worker& worker, scene::Ray& ray, bool volume,
					  scene::Intersection& intersection) final override;

private:

	Settings settings_;

	sampler::Golden_ratio sampler_;
};

class Ao_factory : public Factory {

public:

	Ao_factory(const take::Settings& settings, uint32_t num_samples, float radius);

	virtual Integrator* create(uint32_t num_samples_per_pixel,
							   rnd::Generator& rng) const;

private:

	Ao::Settings settings_;
};

}}}
