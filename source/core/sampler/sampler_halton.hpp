#pragma once

#include "sampler.hpp"
#include "halton/halton_sampler.hpp"

namespace sampler {

class Halton : public Sampler {

public:

	Halton(rnd::Generator& rng, uint32_t num_samples);

	virtual void generate_camera_sample(int2 pixel, uint32_t index,
										Camera_sample& sample) final override;

	virtual float2 generate_sample_2D() final override;

	virtual float generate_sample_1D() final override;

private:

	Halton_sampler halton_sampler_;
};

class Halton_factory : public Factory {

public:

	Halton_factory(uint32_t num_samples);

	virtual Sampler* create(rnd::Generator& rng) const final override;
};

}
