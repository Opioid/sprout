#pragma once

#include "sampler.hpp"

namespace sampler {

class alignas(64) Uniform : public Sampler {

public:

	Uniform(rnd::Generator& rng);

	virtual void generate_camera_sample(int2 pixel, uint32_t index,
										Camera_sample& sample) final override;

	virtual float2 generate_sample_2D(uint32_t dimension = 0) final override;

	virtual float generate_sample_1D(uint32_t dimension = 0) final override;

	virtual size_t num_bytes() const final override;

private:

	virtual void on_resize() final override;

	virtual void on_resume_pixel(rnd::Generator& scramble) final override;
};

class Uniform_factory : public Factory {

public:

	Uniform_factory(uint32_t num_samplers);
	~Uniform_factory();

	virtual Sampler* create(uint32_t id, rnd::Generator& rng) const final override;

private:

	Uniform* samplers_;
};

}
