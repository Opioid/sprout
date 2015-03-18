#pragma once

#include "sampler.hpp"

namespace rendering { namespace sampler {

class Scrambled_hammersley : public Sampler {
public:

	Scrambled_hammersley(uint32_t num_samples_per_iteration, math::random::Generator& rng);

	virtual Sampler* clone(math::random::Generator& rng) const;

	virtual void restart();

	virtual bool generate_camera_sample(const math::float2& offset, Camera_sample& sample);

protected:

	math::random::Generator& rng_;
	uint32_t random_bits_;
};

}}
