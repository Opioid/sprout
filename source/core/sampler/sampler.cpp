#include "sampler.hpp"
#include "base/math/vector.inl"

namespace sampler {

Sampler::Sampler(rnd::Generator& rng, uint32_t num_samples) :
	rng_(rng),
	num_samples_(num_samples),
	current_sample_2D_(0),
	current_sample_1D_(0) {}

Sampler::~Sampler() {}

rnd::Generator& Sampler::rng() {
	return rng_;
}

uint32_t Sampler::num_samples() const {
	return num_samples_;
}

void Sampler::resume_pixel(uint32_t sample, uint2 seed) {
	current_sample_2D_ = sample;
	current_sample_1D_ = sample;
	seed_ = seed;

	on_resume_pixel();
}

void Sampler::on_resume_pixel() {}

Factory::Factory(uint32_t num_samples) :
	num_samples_(num_samples) {}

uint32_t Factory::num_samples_per_iteration() const {
	return num_samples_;
}

}
