#include "sampler.hpp"

namespace sampler {

Sampler::Sampler(uint32_t num_samples_per_iteration) : num_samples_per_iteration_(num_samples_per_iteration) {}

Sampler::~Sampler() {}

uint32_t Sampler::num_samples_per_iteration() const {
	return num_samples_per_iteration_;
}

void Sampler::restart(uint32_t num_iterations) {
	num_iterations_ = num_iterations;
}

void Sampler::start_iteration(uint32_t iteration) {
	current_iteration_ = iteration;
	current_sample_    = 0;
}

}
