#pragma once

#include "sampler.hpp"

namespace sampler {

class alignas(64) EMS : public Sampler {

public:

	EMS(rnd::Generator& rng);

	virtual void generate_camera_sample(int2 pixel, uint32_t index,
										Camera_sample& sample) final override;

	virtual float2 generate_sample_2D(uint32_t dimension = 0) final override;

	virtual float generate_sample_1D(uint32_t dimension = 0) final override;

	virtual size_t num_bytes() const final override;

private:

	virtual void on_resize() final override;

	virtual void on_resume_pixel(rnd::Generator& scramble) final override;

	uint2 scramble_;
};

class EMS_factory : public Factory {

public:

	EMS_factory(uint32_t num_samplers);
	~EMS_factory();

	virtual Sampler* create(uint32_t id, rnd::Generator& rng) const final override;

private:

	EMS* samplers_;
};

}

