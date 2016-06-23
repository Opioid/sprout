#pragma once

#include "sampler.hpp"

namespace sampler {

class Uniform : public Sampler {
public:

	Uniform(math::random::Generator& rng);

	virtual Sampler* clone() const final override;

	virtual math::uint2 seed() const final override;

	virtual void generate_camera_sample(int2 pixel, uint32_t index, Camera_sample& sample) final override;

	virtual float2 generate_sample_2D() final override;

	virtual float generate_sample_1D() final override;
};

}
