#pragma once

#include "sampler.hpp"

namespace sampler {

class Hammersley : public Sampler {

public:

	Hammersley(rnd::Generator& rng);

	virtual void generate_camera_sample(int2 pixel, uint32_t index,
										Camera_sample& sample) final override;

	virtual float2 generate_sample_2D() final override;

	virtual float generate_sample_1D(uint32_t dimension = 0) final override;

private:

	virtual void on_resize() final override;

	virtual void on_resume_pixel(rnd::Generator& scramble) final override;

	uint32_t scramble_;
};

class Hammersley_factory : public Factory {

public:

	virtual Sampler* create(rnd::Generator& rng) const final override;
};

}
