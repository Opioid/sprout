#include "scrambled_hammersley_sampler.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"
#include "base/math/distribution.inl"

namespace sampler {

Scrambled_hammersley::Scrambled_hammersley(uint32_t num_samples_per_iteration, math::random::Generator& rng) :
	Sampler(num_samples_per_iteration), rng_(rng) {}

Sampler* Scrambled_hammersley::clone(math::random::Generator& rng) const {
	return new Scrambled_hammersley(num_samples_per_iteration_, rng);
}

void Scrambled_hammersley::restart(uint32_t num_iterations) {
	num_iterations_ = num_iterations;
	random_bits_ = rng_.random_uint();
}

bool Scrambled_hammersley::generate_camera_sample(const math::float2& offset, Camera_sample& sample) {
	if (current_sample_ >= num_samples_per_iteration_) {
		return false;
	}

	math::float2 s2d = math::scrambled_hammersley(current_sample_, num_samples_per_iteration_, random_bits_);

	sample.coordinates = offset + s2d;
	sample.relative_offset = s2d - math::float2(0.5f, 0.5f);
	sample.lens_uv = s2d.yx();

	++current_sample_;

	return true;
}

math::float2 Scrambled_hammersley::generate_sample2d(uint32_t index) {
	return math::scrambled_hammersley(index, num_samples_per_iteration_, random_bits_);
}

float Scrambled_hammersley::generate_sample1d(uint32_t /*index*/) {
	return rng_.random_float();
}

}
