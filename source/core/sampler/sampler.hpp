#pragma once

#include "base/math/vector.hpp"

namespace math { namespace random {

class Generator;

}}

namespace sampler {

struct Camera_sample;

class Sampler {
public:

	Sampler(uint32_t num_samples_per_iteration);
	virtual ~Sampler();

	virtual Sampler* clone(math::random::Generator& rng) const = 0;

	virtual void restart();

	virtual bool generate_camera_sample(const math::float2& offset, Camera_sample& sample) = 0;

	virtual math::float2 generate_sample2d() = 0;

protected:

	uint32_t num_samples_per_iteration_;

	uint32_t current_sample_;
};

}
