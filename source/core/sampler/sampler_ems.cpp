#include "sampler_ems.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/random/generator.inl"

namespace sampler {

EMS::EMS(random::Generator& rng, uint32_t num_samples) :
	Sampler(rng, num_samples) {}

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

EMS_factory::EMS_factory(uint32_t num_samples_per_iteration) :
	Factory(num_samples_per_iteration) {}

Sampler* EMS_factory::create(random::Generator& rng) const {
	return new EMS(rng, num_samples_);
}

}
