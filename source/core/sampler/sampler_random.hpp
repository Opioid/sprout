#pragma once

#include "sampler.hpp"

namespace sampler {

class Random : public Sampler {

public:

	Random(rnd::Generator& rng, uint32_t num_samples, uint32_t num_dimensions_1D = 1);

	virtual void generate_camera_sample(int2 pixel, uint32_t index,
										Camera_sample& sample) final override;

	virtual float2 generate_sample_2D() final override;

	virtual float generate_sample_1D(uint32_t dimension = 0) final override;

private:

	virtual void on_resume_pixel(rnd::Generator& scramble) final override;
};

class Random_factory : public Factory {

public:

	Random_factory(uint32_t num_samples);

	virtual Sampler* create(rnd::Generator& rng) const final override;
};

}
