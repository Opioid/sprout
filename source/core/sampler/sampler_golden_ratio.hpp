#pragma once

#include "sampler.hpp"
#include <vector>

namespace sampler {

class alignas(64) Golden_ratio : public Sampler {

public:

	Golden_ratio(rnd::Generator& rng);
	~Golden_ratio();

	virtual void generate_camera_sample(int2 pixel, uint32_t index,
										Camera_sample& sample) final override;

	virtual float2 generate_sample_2D(uint32_t dimension = 0) final override;

	virtual float generate_sample_1D(uint32_t dimension = 0) final override;

	virtual size_t num_bytes() const final override;

private:

	virtual void on_resize() final override;

	virtual void on_resume_pixel(rnd::Generator& scramble) final override;

	float2* samples_2D_;
	float*  samples_1D_;
};

class Golden_ratio_factory : public Factory {

public:

	Golden_ratio_factory(uint32_t num_samplers);
	~Golden_ratio_factory();

	virtual Sampler* create(uint32_t id, rnd::Generator& rng) const final override;

private:

	Golden_ratio* samplers_;
};

}
