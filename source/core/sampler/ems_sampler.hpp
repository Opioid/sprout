#pragma once

#include "sampler.hpp"

namespace sampler {

class EMS : public Sampler {
public:

	EMS(math::random::Generator& rng, uint32_t num_samples_per_iteration);

	virtual Sampler* clone() const final override;

	virtual void generate_camera_sample(const math::float2& offset, uint32_t index, Camera_sample& sample) final override;

	virtual math::float2 generate_sample_2d() final override;

	virtual float generate_sample_1d() final override;

private:

	virtual uint32_t seed() const final override;
};

}

