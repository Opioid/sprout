#include "ems_sampler.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"
#include "base/math/distribution.inl"

namespace sampler {

EMS::EMS(uint32_t num_samples_per_iteration, math::random::Generator& rng) :
	Sampler(num_samples_per_iteration), rng_(rng) {}

Sampler* EMS::clone(math::random::Generator& rng) const {
	return new EMS(num_samples_per_iteration_, rng);
}

void EMS::restart(uint32_t num_iterations) {
	num_iterations_ = num_iterations;
	random_bits_ = rng_.random_uint();
	random_bits2_ = rng_.random_uint();
}

bool EMS::generate_camera_sample(const math::float2& offset, Camera_sample& sample) {
	if (current_sample_ >= num_samples_per_iteration_) {
		return false;
	}

	math::float2 s2d = math::ems(current_sample_, random_bits_);

	sample.coordinates = offset + s2d;
	sample.relative_offset = s2d - math::float2(0.5f, 0.5f);
	sample.lens_uv = s2d.yx();

	++current_sample_;

	return true;
}

math::float2 EMS::generate_sample2d(uint32_t index) {
	uint32_t offset = num_samples_per_iteration_ * current_iteration_;
	return math::ems(offset + index, random_bits_);
}

float EMS::generate_sample1d(uint32_t index) {
	return rng_.random_float();
}

}

