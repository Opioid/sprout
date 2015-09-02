#pragma once

#include "rendering/integrator/integrator.hpp"
#include "sampler/ems_sampler.hpp"
#include "sampler/ld_sampler.hpp"
#include "sampler/scrambled_hammersley_sampler.hpp"
#include "sampler/random_sampler.hpp"

namespace rendering {

class Ao : public Surface_integrator {
public:

	struct Settings {
		uint32_t num_samples;
		float num_samples_reciprocal;
		float radius;
	};

	Ao(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings);

	virtual void start_new_pixel(uint32_t num_samples) final override;

	virtual math::float4 li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) final override;

private:

	Settings settings_;

	sampler::EMS sampler_;
};

class Ao_factory : public Surface_integrator_factory {
public:

	Ao_factory(const take::Settings& settings, uint32_t num_samples, float radius);

	virtual Surface_integrator* create(math::random::Generator& rng) const;

private:

	Ao::Settings settings_;
};

}
