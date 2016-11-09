#pragma once

#include "sampler.hpp"

namespace sampler {

class Scrambled_hammersley : public Sampler {

public:

	Scrambled_hammersley(math::random::Generator& rng,
						 uint32_t num_samples);

	virtual void generate_camera_sample(int2 pixel, uint32_t index,
										Camera_sample& sample) final override;

	virtual float2 generate_sample_2D() final override;

	virtual float generate_sample_1D() final override;
};

class Scrambled_hammersley_factory : public Factory {

public:

	Scrambled_hammersley_factory(uint32_t num_samples);

	virtual Sampler* create(math::random::Generator& rng) const final override;
};

}
