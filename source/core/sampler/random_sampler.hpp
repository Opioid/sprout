#pragma once

#include "sampler.hpp"

namespace sampler {

class Random : public Sampler {
public:

	Random(uint32_t num_samples_per_iteration, math::random::Generator& rng);

	virtual Sampler* clone(math::random::Generator& rng) const;

	virtual bool generate_camera_sample(const math::float2& offset, Camera_sample& sample);

	virtual math::float2 generate_sample2d(uint32_t index);

	virtual float generate_sample1d(uint32_t index);

protected:

	math::random::Generator& rng_;
};

}
