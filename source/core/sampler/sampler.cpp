#include "sampler.hpp"
#include "base/math/vector.inl"

namespace sampler {

Sampler::Sampler(rnd::Generator& rng, uint32_t num_samples, uint32_t num_dimensions_1D) :
	rng_(rng),
	num_samples_(num_samples),
	current_sample_2D_(0),
	num_dimensions_1D_(num_dimensions_1D),
	current_sample_1D_(new uint32_t[num_dimensions_1D]) {}

Sampler::~Sampler() {}

rnd::Generator& Sampler::rng() {
	return rng_;
}

uint32_t Sampler::num_samples() const {
	return num_samples_;
}

void Sampler::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	current_sample_2D_ = sample;

	for (uint32_t i = 0; i < num_dimensions_1D_; ++i) {
		current_sample_1D_[i] = sample;
	}

	on_resume_pixel(scramble);
}

void Sampler::on_resume_pixel(rnd::Generator& /*scramble*/) {}

Factory::Factory(uint32_t num_samples) :
	num_samples_(num_samples) {}

uint32_t Factory::num_samples_per_iteration() const {
	return num_samples_;
}

}
