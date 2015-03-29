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

	uint32_t num_samples_per_iteration() const;

	virtual void restart(uint32_t num_iterations);

	void start_iteration(uint32_t iteration);

	virtual bool generate_camera_sample(const math::float2& offset, Camera_sample& sample) = 0;

	virtual math::float2 generate_sample2d(uint32_t index) = 0;

	virtual float generate_sample1d(uint32_t index) = 0;

protected:

	uint32_t num_iterations_;
	uint32_t num_samples_per_iteration_;
	uint32_t current_iteration_;
	uint32_t current_sample_;
};

}
