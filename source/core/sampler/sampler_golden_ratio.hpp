#pragma once

#include "sampler.hpp"

namespace sampler {

class alignas(64) Golden_ratio final : public Sampler {

public:

	Golden_ratio(rnd::Generator& rng);
	virtual ~Golden_ratio() override final;

	virtual void generate_camera_sample(int2 pixel, uint32_t index,
										Camera_sample& sample) override final;

	virtual float2 generate_sample_2D(uint32_t dimension = 0) override final;

	virtual float generate_sample_1D(uint32_t dimension = 0) override final;

	virtual size_t num_bytes() const override final;

private:

	virtual void on_resize() override final;

	virtual void on_resume_pixel(rnd::Generator& scramble) override final;

	float2* samples_2D_;
	float*  samples_1D_;
};

class Golden_ratio_factory final : public Factory {

public:

	Golden_ratio_factory(uint32_t num_samplers);
	virtual ~Golden_ratio_factory() override final;

	virtual Sampler* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Golden_ratio* samplers_;
};

}
