#include "sampler.hpp"

namespace sampler {

Sampler::Sampler(uint32_t num_samples_per_iteration) : num_samples_per_iteration_(num_samples_per_iteration) {}

Sampler::~Sampler() {}

void Sampler::restart() {
	current_sample_ = 0;
}

}
