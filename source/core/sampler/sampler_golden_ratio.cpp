#include "sampler_golden_ratio.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/random/generator.inl"
#include "base/random/shuffle.inl"

#include "base/debug/assert.hpp"

namespace sampler {

Golden_ratio::Golden_ratio(rnd::Generator& rng) :
	Sampler(rng),
	samples_2D_(nullptr),
	samples_1D_(nullptr) {}

Golden_ratio::~Golden_ratio() {
	delete [] samples_1D_;
	delete [] samples_2D_;
}

void Golden_ratio::generate_camera_sample(int2 pixel, uint32_t index,
										  Camera_sample& sample) {
	sample.pixel = pixel;
	sample.pixel_uv = samples_2D_[index];
	sample.lens_uv  = samples_2D_[num_samples_ + index];
	sample.time = samples_1D_[index];
}

float2 Golden_ratio::generate_sample_2D(uint32_t dimension) {
	SOFT_ASSERT(current_sample_2D_[dimension] < num_samples_);

	uint32_t current = current_sample_2D_[dimension]++;

	return samples_2D_[dimension * num_samples_ + current];
}

float Golden_ratio::generate_sample_1D(uint32_t dimension) {
	SOFT_ASSERT(current_sample_1D_[dimension] < num_samples_);

	uint32_t current = current_sample_1D_[dimension]++;

	return samples_1D_[dimension * num_samples_ + current];
}

void Golden_ratio::on_resize() {
	delete [] samples_1D_;
	delete [] samples_2D_;

	samples_2D_ = new float2[num_samples_ * num_dimensions_2D_];
	samples_1D_ = new float [num_samples_ * num_dimensions_1D_];
}

void Golden_ratio::on_resume_pixel(rnd::Generator& scramble) {
	for (uint32_t i = 0; i < num_dimensions_2D_; ++i) {
		float2* begin = samples_2D_ + i * num_samples_;
		float2 r(scramble.random_float(), scramble.random_float());
		math::golden_ratio(begin, num_samples_, r);
		rnd::shuffle(begin, num_samples_, scramble);
	}

	for (uint32_t i = 0; i < num_dimensions_1D_; ++i) {
		float* begin = samples_1D_ + i * num_samples_;
		math::golden_ratio(begin, num_samples_, scramble.random_float());
		rnd::shuffle(begin, num_samples_, scramble);
	}
}

Sampler* Golden_ratio_factory::create(rnd::Generator& rng) const {
	return new Golden_ratio(rng);
}

}
