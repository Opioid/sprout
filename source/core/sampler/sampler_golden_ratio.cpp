#include "sampler_golden_ratio.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/random/generator.inl"
#include "base/random/shuffle.inl"

#include "base/debug/assert.hpp"

namespace sampler {

Golden_ratio::Golden_ratio(rnd::Generator& rng, uint32_t num_samples) :
	Sampler(rng, num_samples),
	samples_2D_(new float2[num_samples]),
	samples_1D_(new float[num_samples]),
	test_samples_(new float[num_samples])
{}

Golden_ratio::~Golden_ratio() {
	delete [] test_samples_;
	delete [] samples_1D_;
	delete [] samples_2D_;
}

void Golden_ratio::generate_camera_sample(int2 pixel, uint32_t index,
										  Camera_sample& sample) {
	sample.pixel = pixel;
	sample.pixel_uv = samples_2D_[index];
	sample.lens_uv = samples_2D_[num_samples_ - 1 - index].yx();
	sample.time = samples_1D_[index];
}

float2 Golden_ratio::generate_sample_2D() {
	SOFT_ASSERT(current_sample_2D_ < num_samples_);

	return samples_2D_[current_sample_2D_++];
}

float Golden_ratio::generate_sample_1D(uint32_t dimension) {
	SOFT_ASSERT(current_sample_1D_ < num_samples_);

	if (1 == dimension) {
		return test_samples_[current_test_sample_++];
	}

	return samples_1D_[current_sample_1D_++];
}

void Golden_ratio::on_resume_pixel(rnd::Generator& scramble) {
	float2 r(scramble.random_float(), scramble.random_float());
	math::golden_ratio(samples_2D_, num_samples_, r);
	rnd::shuffle(samples_2D_, num_samples_, scramble);

	math::golden_ratio(samples_1D_, num_samples_, scramble.random_float());
	rnd::shuffle(samples_1D_, num_samples_, scramble);

	current_test_sample_ = 0;
	math::golden_ratio(test_samples_, num_samples_, scramble.random_float());
	rnd::shuffle(test_samples_, num_samples_, scramble);
}

Golden_ratio_factory::Golden_ratio_factory(uint32_t num_samples) :
	Factory(num_samples) {}

Sampler* Golden_ratio_factory::create(rnd::Generator& rng) const {
	return new Golden_ratio(rng, num_samples_);
}

}
