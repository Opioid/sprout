#pragma once

#include "sampler.hpp"

namespace sampler {

class Random : public Sampler {
public:

	Random(math::random::Generator& rng, uint32_t num_samples_per_iteration);

	virtual Sampler* clone() const;

	virtual bool generate_camera_sample(const math::float2& offset, Camera_sample& sample);

	virtual math::float2 generate_sample2d(uint32_t index);

	virtual float generate_sample1d(uint32_t index);

};

}
