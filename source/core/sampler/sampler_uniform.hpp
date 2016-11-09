#pragma once

#include "sampler.hpp"

namespace sampler {

class Uniform : public Sampler {

public:

	Uniform(math::random::Generator& rng);

	virtual void generate_camera_sample(int2 pixel, uint32_t index,
										Camera_sample& sample) final override;

	virtual float2 generate_sample_2D() final override;

	virtual float generate_sample_1D() final override;
};

class Uniform_factory : public Factory {

public:

	Uniform_factory(uint32_t num_samples_per_iteration);

	virtual Sampler* create(math::random::Generator& rng) const final override;
};

}
