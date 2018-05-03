#include "sampler_golden_ratio.hpp"
#include "camera_sample.hpp"
#include "base/math/vector2.inl"
#include "base/math/sampling/sample_distribution.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/random/shuffle.hpp"

#include "base/debug/assert.hpp"

namespace sampler {

Golden_ratio::Golden_ratio(rnd::Generator& rng) :
	Sampler(rng),
	samples_2D_(nullptr),
	samples_1D_(nullptr) {}

Golden_ratio::~Golden_ratio() {
	memory::free_aligned(samples_1D_);
	memory::free_aligned(samples_2D_);
}

void Golden_ratio::generate_camera_sample(int2 pixel, uint32_t index, Camera_sample& sample) {
	sample.pixel = pixel;
	sample.pixel_uv = samples_2D_[index];
	sample.lens_uv  = samples_2D_[num_samples_ + index];
	sample.time = samples_1D_[index];
}

float2 Golden_ratio::generate_sample_2D(uint32_t dimension) {
	SOFT_ASSERT(current_sample_2D_[dimension] < num_samples_);

	uint32_t const current = current_sample_2D_[dimension]++;

	return samples_2D_[dimension * num_samples_ + current];
}

float Golden_ratio::generate_sample_1D(uint32_t dimension) {
	SOFT_ASSERT(current_sample_1D_[dimension] < num_samples_);

	uint32_t const current = current_sample_1D_[dimension]++;

	return samples_1D_[dimension * num_samples_ + current];
}

size_t Golden_ratio::num_bytes() const {
	return num_samples_ * num_dimensions_2D_ * sizeof(float2)
		 + num_samples_ * num_dimensions_1D_ * sizeof(float);
}

void Golden_ratio::on_resize() {
	memory::free_aligned(samples_1D_);
	memory::free_aligned(samples_2D_);

	samples_2D_ = memory::allocate_aligned<float2>(num_samples_ * num_dimensions_2D_);
	samples_1D_ = memory::allocate_aligned<float> (num_samples_ * num_dimensions_1D_);
}

void Golden_ratio::on_resume_pixel(rnd::Generator& scramble) {
	for (uint32_t i = 0, len = num_dimensions_2D_; i < len; ++i) {
		float2* begin = samples_2D_ + i * num_samples_;
		float2 const r(scramble.random_float(), scramble.random_float());
		math::golden_ratio(begin, num_samples_, r);
		rnd::shuffle(begin, num_samples_, scramble);
	}

	for (uint32_t i = 0, len = num_dimensions_1D_; i < len; ++i) {
		float* begin = samples_1D_ + i * num_samples_;
		math::golden_ratio(begin, num_samples_, scramble.random_float());
		rnd::shuffle(begin, num_samples_, scramble);
	}
}

Golden_ratio_factory::Golden_ratio_factory(uint32_t num_samplers) :
	Factory(num_samplers),
	samplers_(memory::allocate_aligned<Golden_ratio>(num_samplers)) {}

Golden_ratio_factory::~Golden_ratio_factory() {
	memory::free_aligned(samplers_);
}

Sampler* Golden_ratio_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&samplers_[id]) Golden_ratio(rng);
}

}
