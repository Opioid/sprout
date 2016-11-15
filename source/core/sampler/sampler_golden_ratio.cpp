#include "sampler_golden_ratio.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/random/generator.inl"
#include "base/random/shuffle.inl"

#include "base/debug/assert.hpp"

namespace sampler {

Golden_ratio::Golden_ratio(rnd::Generator& rng, uint32_t num_samples, uint32_t num_dimensions_1D) :
	Sampler(rng, num_samples, num_dimensions_1D),
	samples_2D_(new float2[num_samples]),
	samples_1D_(new float[num_samples * num_dimensions_1D]) {}

Golden_ratio::~Golden_ratio() {
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
	SOFT_ASSERT(current_sample_1D_[dimension] < num_samples_);

	uint32_t current = current_sample_1D_[dimension]++;

	return samples_1D_[dimension * num_samples_ + current];
}

void Golden_ratio::on_resume_pixel(rnd::Generator& scramble) {
	float2 r(scramble.random_float(), scramble.random_float());
	math::golden_ratio(samples_2D_, num_samples_, r);
	rnd::shuffle(samples_2D_, num_samples_, scramble);

	for (uint32_t i = 0; i < num_dimensions_1D_; ++i) {
		float* begin = samples_1D_ + i * num_samples_;
		math::golden_ratio(begin, num_samples_, scramble.random_float());
		rnd::shuffle(begin, num_samples_, scramble);
	}
}

Golden_ratio_factory::Golden_ratio_factory(uint32_t num_samples) :
	Factory(num_samples) {}

Sampler* Golden_ratio_factory::create(rnd::Generator& rng) const {
	return new Golden_ratio(rng, num_samples_, 1);
}

}
