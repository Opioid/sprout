#include "ems_sampler.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"
#include "base/math/distribution.inl"

namespace sampler {

EMS::EMS(math::random::Generator& rng, uint32_t num_samples_per_iteration) :
	Sampler(rng, num_samples_per_iteration) {}

Sampler* EMS::clone() const {
	return new EMS(rng_, num_samples_per_iteration_);
}

void EMS::restart(uint32_t num_iterations) {
	num_iterations_ = num_iterations;
	random_bits_  = rng_.random_uint();
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

math::float2 EMS::generate_sample_2d(uint32_t index) {
	uint32_t offset = current_iteration_ * num_samples_per_iteration_;
	return math::ems(offset + index, random_bits_);
}

float EMS::generate_sample_1d(uint32_t /*index*/) {
	return rng_.random_float();
}

}

