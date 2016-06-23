#include "sampler_ems.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"
#include "base/math/sampling/sample_distribution.inl"

namespace sampler {

EMS::EMS(math::random::Generator& rng, uint32_t num_samples_per_iteration) :
	Sampler(rng, num_samples_per_iteration) {}

Sampler* EMS::clone() const {
	return new EMS(rng_, num_samples_per_iteration_);
}

math::uint2 EMS::seed() const {
	return math::uint2(rng_.random_uint(), rng_.random_uint());
}

void EMS::generate_camera_sample(int2 pixel, uint32_t index,
								 Camera_sample& sample) {
	float2 s2d = math::ems(index, seed_.x, seed_.y);

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = math::scrambled_radical_inverse_vdC(index, seed_.y);
}

float2 EMS::generate_sample_2D() {
	return math::ems(current_sample_++, seed_.x, seed_.y);
}

float EMS::generate_sample_1D() {
	return rng_.random_float();
//	return math::scrambled_radical_inverse_vdC(current_sample_++, seed_.y);
}

}
