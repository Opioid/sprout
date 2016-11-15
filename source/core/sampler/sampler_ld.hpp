#pragma once

#include "sampler.hpp"

namespace sampler {

class LD : public Sampler {

public:

	LD(rnd::Generator& rng, uint32_t num_samples);

	virtual void generate_camera_sample(int2 pixel, uint32_t index,
										Camera_sample& sample) final override;

	virtual float2 generate_sample_2D() final override;

	virtual float generate_sample_1D(uint32_t dimension = 0) final override;

private:

	virtual void on_resume_pixel(rnd::Generator& scramble) final override;

	uint2 scramble_;
};

class LD_factory : public Factory {

public:

	LD_factory(uint32_t num_samples);

	virtual Sampler* create(rnd::Generator& rng) const final override;
};

}

