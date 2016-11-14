
#pragma once

#include "surface_integrator.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "sampler/sampler_sobol.hpp"

namespace scene { namespace material { class Sample; } }

namespace rendering { namespace integrator { namespace surface {

class Whitted : public Integrator {

public:

	struct Settings {
		uint32_t num_light_samples;
		float    num_light_samples_reciprocal;
	};

	Whitted(uint32_t num_samples_per_pixel,
			const take::Settings& take_settings,
			rnd::Generator& rng, const
			Settings& settings);

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float4 li(Worker& worker, scene::Ray& ray, bool volume,
					  scene::Intersection& intersection) final override;

private:

	float3 shade(Worker& worker, const scene::Ray& ray,
				 const scene::Intersection& intersection);

	float3 estimate_direct_light(Worker& worker, const scene::Ray& ray,
								 const scene::Intersection& intersection,
								 const scene::material::Sample& material_sample);

	Settings settings_;

	sampler::Golden_ratio sampler_;
};

class Whitted_factory : public Factory {

public:

	Whitted_factory(const take::Settings& take_settings, uint32_t num_light_samples);

	virtual Integrator* create(uint32_t num_samples_per_pixel,
							   rnd::Generator& rng) const;

private:

	Whitted::Settings settings_;
};

}}}
