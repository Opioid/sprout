#pragma once

#include "sampler.hpp"

namespace sampler {

class EMS : public Sampler {
public:

	EMS(math::random::Generator& rng, uint32_t num_samples_per_iteration);

	virtual Sampler* clone() const final override;

	virtual void restart(uint32_t num_iterations) final override;

	virtual bool generate_camera_sample(const math::float2& offset, Camera_sample& sample) final override;

	virtual math::float2 generate_sample_2d(uint32_t index) final override;

	virtual float generate_sample_1d(uint32_t index) final override;

protected:

	uint32_t random_bits_;
};

}

