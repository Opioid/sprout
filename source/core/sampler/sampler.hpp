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

	uint32_t restart(uint32_t num_iterations);

	void set_seed(uint32_t seed);

	virtual void generate_camera_sample(const math::float2& offset, uint32_t index, Camera_sample& sample) = 0;

	virtual math::float2 generate_sample_2D() = 0;

	virtual float generate_sample_1D() = 0;

protected:

	virtual uint32_t seed() const = 0;

	math::random::Generator& rng_;
	uint32_t seed_;
	uint32_t num_iterations_;
	uint32_t num_samples_per_iteration_;
	uint32_t current_sample_;
};

}
