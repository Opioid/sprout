#pragma once

#include "sampler.hpp"
#include "halton/halton_sampler.hpp"

namespace sampler {

class Halton : public Sampler {

public:

	Halton(math::random::Generator& rng, uint32_t num_samples_per_iteration);

	virtual Sampler* clone() const final override;

	virtual math::uint2 seed() const final override;

	virtual void generate_camera_sample(int2 pixel, uint32_t index,
										Camera_sample& sample) final override;

	virtual float2 generate_sample_2D() final override;

	virtual float generate_sample_1D() final override;

private:

	Halton_sampler halton_sampler_;
};

}
