#pragma once

#include "base/math/vector2.hpp"
#include <cstddef>

namespace rnd { class Generator; }

namespace sampler {

struct Camera_sample;

class Sampler {

public:

	Sampler(rnd::Generator& rng);
	virtual ~Sampler();

	void resize(uint32_t num_iterations, uint32_t num_samples_per_iteration,
				uint32_t num_dimensions_2D, uint32_t num_dimensions_1D);

	void resume_pixel(uint32_t iteration, rnd::Generator& scramble);

	uint32_t num_samples() const;

	virtual void generate_camera_sample(int2 pixel, uint32_t index, Camera_sample& sample) = 0;

	virtual float2 generate_sample_2D(uint32_t dimension = 0) = 0;

	virtual float generate_sample_1D(uint32_t dimension = 0) = 0;

	virtual size_t num_bytes() const = 0;

protected:

	virtual void on_resize() = 0;

	virtual void on_resume_pixel(rnd::Generator& scramble) = 0;

	rnd::Generator& rng_;
	uint32_t num_samples_;
	uint32_t num_samples_per_iteration_;

	uint32_t  num_dimensions_2D_;
	uint32_t  num_dimensions_1D_;

	uint32_t* current_sample_2D_;
	uint32_t* current_sample_1D_;
};

class Factory {

public:

	virtual Sampler* create(rnd::Generator& rng) const = 0;
};

}
