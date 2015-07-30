#include "sampler.hpp"
#include "base/math/vector.inl"

namespace sampler {

Sampler::Sampler(math::random::Generator& rng, uint32_t num_samples_per_iteration) :
	rng_(rng), num_samples_per_iteration_(num_samples_per_iteration) {}

Sampler::~Sampler() {}

math::random::Generator& Sampler::rng() {
	return rng_;
}

uint32_t Sampler::num_samples_per_iteration() const {
	return num_samples_per_iteration_;
}

math::uint2 Sampler::restart(uint32_t num_iterations) {
	set_seed(seed());
	num_iterations_ = num_iterations;
	current_sample_ = 0;
	return seed_;
}

void Sampler::set_seed(math::uint2 seed) {
	seed_ = seed;
}

}
