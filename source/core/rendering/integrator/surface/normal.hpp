#pragma once

#include "rendering/integrator/integrator.hpp"
#include "sampler/ems_sampler.hpp"
#include "sampler/random_sampler.hpp"

namespace rendering {

class Normal : public Surface_integrator {
public:

	struct Settings {

	};

	Normal(uint32_t id, math::random::Generator& rng, const Settings& settings);

	virtual void start_new_pixel(uint32_t num_samples);

	virtual math::float3 li(const Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection);

private:

	Settings settings_;
};

class Normal_factory : public Surface_integrator_factory {
public:

	Normal_factory();

	virtual Surface_integrator* create(uint32_t id,  math::random::Generator& rng) const;

private:

	Normal::Settings settings_;
};

}
