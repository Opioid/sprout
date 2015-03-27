#pragma once

#include "rendering/integrator/integrator.hpp"
#include "sampler/random_sampler.hpp"
#include <vector>

namespace scene { namespace light {

struct Sample;

}}

namespace rendering {

class Whitted : public Surface_integrator {
public:

	struct Settings {

	};

	Whitted(uint32_t id,  math::random::Generator& rng, const Settings& settings);

	virtual math::float3 li(const Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection);

private:

	Settings settings_;

	sampler::Random sampler_;

	std::vector<scene::light::Sample> light_samples_;
};

class Whitted_factory : public Surface_integrator_factory {
public:

	Whitted_factory();

	virtual Surface_integrator* create(uint32_t id,  math::random::Generator& rng) const;

private:

	Whitted::Settings settings_;
};

}

