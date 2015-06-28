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

void EMS::generate_camera_sample(const math::float2& offset, uint32_t index, Camera_sample& sample) {
	math::float2 s2d = math::ems(index, seed_);

	sample.coordinates = offset + s2d;
	sample.relative_offset = s2d - math::float2(0.5f, 0.5f);
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

math::float2 EMS::generate_sample_2d() {
	return math::ems(current_sample_++, seed_);
}

float EMS::generate_sample_1d() {
	return rng_.random_float();
}

uint32_t EMS::seed() const {
	return rng_.random_uint();
}

}

