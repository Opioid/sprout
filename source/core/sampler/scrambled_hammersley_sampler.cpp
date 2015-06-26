#include "scrambled_hammersley_sampler.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"
#include "base/math/distribution.inl"

namespace sampler {

Scrambled_hammersley::Scrambled_hammersley(math::random::Generator& rng, uint32_t num_samples_per_iteration) :
	Sampler(rng, num_samples_per_iteration) {}

Sampler* Scrambled_hammersley::clone() const {
	return new Scrambled_hammersley(rng_, num_samples_per_iteration_);
}

void Scrambled_hammersley::restart(uint32_t num_iterations) {
	num_iterations_ = num_iterations;
	random_bits_ = rng_.random_uint();
	current_sample_ = 0;
}

bool Scrambled_hammersley::generate_camera_sample(const math::float2& offset, Camera_sample& sample) {
	if (current_sample_ >= num_samples_per_iteration_) {
		return false;
	}

	math::float2 s2d = math::scrambled_hammersley(current_sample_, num_samples_per_iteration_, random_bits_);

	sample.coordinates = offset + s2d;
	sample.relative_offset = s2d - math::float2(0.5f, 0.5f);
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();

	++current_sample_;

	return true;
}

math::float2 Scrambled_hammersley::generate_sample_2d() {
	return math::scrambled_hammersley(current_sample_++, num_iterations_ * num_samples_per_iteration_, random_bits_);
}

float Scrambled_hammersley::generate_sample_1d() {
	return rng_.random_float();
}

}
