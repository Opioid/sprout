#include "sampler.hpp"
#include "base/math/vector.inl"

namespace sampler {

Sampler::Sampler(rnd::Generator& rng) :
	rng_(rng),
	num_samples_(0),
	num_samples_per_iteration_(0),
	num_dimensions_2D_(0),
	num_dimensions_1D_(0),
	current_sample_2D_(nullptr),
	current_sample_1D_(nullptr) {}

Sampler::~Sampler() {
	delete [] current_sample_1D_;
	delete [] current_sample_2D_;
}

void Sampler::resize(uint32_t num_iterations, uint32_t num_samples_per_iteration,
					 uint32_t num_dimensions_2D, uint32_t num_dimensions_1D) {
	uint32_t num_samples = num_iterations * num_samples_per_iteration;

	if (num_samples != num_samples_
	||  num_samples_per_iteration != num_samples_per_iteration
	||  num_dimensions_1D != num_dimensions_1D_) {
		delete [] current_sample_1D_;
		delete [] current_sample_2D_;

		num_samples_ = num_samples;

		num_dimensions_2D_ = num_dimensions_2D;
		current_sample_2D_ = new uint32_t[num_dimensions_2D];

		num_dimensions_1D_ = num_dimensions_1D;
		current_sample_1D_ = new uint32_t[num_dimensions_1D];

		on_resize();
	}
}

rnd::Generator& Sampler::rng() {
	return rng_;
}

uint32_t Sampler::num_samples() const {
	return num_samples_;
}

void Sampler::resume_pixel(uint32_t iteration, rnd::Generator& scramble) {
	uint32_t sample = iteration * num_samples_per_iteration_;

	for (uint32_t i = 0; i < num_dimensions_2D_; ++i) {
		current_sample_2D_[i] = sample;
	}

	for (uint32_t i = 0; i < num_dimensions_1D_; ++i) {
		current_sample_1D_[i] = sample;
	}

	on_resume_pixel(scramble);
}

}
