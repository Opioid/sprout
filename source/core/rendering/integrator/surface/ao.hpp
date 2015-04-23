#pragma once

#include "rendering/integrator/integrator.hpp"
#include "sampler/ems_sampler.hpp"
#include "sampler/random_sampler.hpp"

namespace rendering {

class Ao : public Surface_integrator {
public:

	struct Settings {
		uint32_t num_samples;
		float num_samples_reciprocal;
		float radius;
	};

	Ao(math::random::Generator& rng, const Settings& settings);

	virtual void start_new_pixel(uint32_t num_samples);

	virtual math::float3 li(Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection);

private:

	Settings settings_;

	sampler::EMS sampler_;
};

class Ao_factory : public Surface_integrator_factory {
public:

	Ao_factory(uint32_t num_samples, float radius);

	virtual Surface_integrator* create(math::random::Generator& rng) const;

private:

	Ao::Settings settings_;
};

}
