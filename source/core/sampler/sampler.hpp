#pragma once

#include "base/math/vector.hpp"

namespace math { namespace random { class Generator; }}

namespace sampler {

struct Camera_sample;

class Sampler {

public:

	Sampler(math::random::Generator& rng, uint32_t num_samples_per_iteration);
	virtual ~Sampler();

	math::random::Generator& rng();

	uint32_t num_samples_per_iteration() const;

	void restart(uint32_t num_iterations);
	void restart_and_seed(uint32_t num_iterations);

	virtual math::uint2 seed() const = 0;

	void set_seed(math::uint2 seed);

	virtual void generate_camera_sample(int2 pixel, uint32_t index, Camera_sample& sample) = 0;

	virtual float2 generate_sample_2D() = 0;

	virtual float generate_sample_1D() = 0;

protected:

	math::random::Generator& rng_;
	math::uint2 seed_;
	uint32_t num_iterations_;
	uint32_t num_samples_per_iteration_;
	uint32_t current_sample_;
};

class Factory {

public:

	Factory(uint32_t num_samples_per_iteration);

	virtual Sampler* create(math::random::Generator& rng) const = 0;

	uint32_t num_samples_per_iteration() const;

protected:

	uint32_t num_samples_per_iteration_;
};

}
