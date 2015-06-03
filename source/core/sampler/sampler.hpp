#pragma once

#include "base/math/vector.hpp"

namespace math { namespace random {

class Generator;

}}

namespace sampler {

struct Camera_sample;

class Sampler {
public:

	Sampler(math::random::Generator& rng, uint32_t num_samples_per_iteration);
	virtual ~Sampler();

	virtual Sampler* clone() const = 0;

	math::random::Generator& rng();

	uint32_t num_samples_per_iteration() const;

	virtual void restart(uint32_t num_iterations);

	virtual bool generate_camera_sample(const math::float2& offset, Camera_sample& sample) = 0;

	virtual math::float2 generate_sample_2d() = 0;

	virtual float generate_sample_1d() = 0;

protected:

	math::random::Generator& rng_;
	uint32_t num_iterations_;
	uint32_t num_samples_per_iteration_;
	uint32_t current_sample_;
};

}
