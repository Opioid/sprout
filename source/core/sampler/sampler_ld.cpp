#include "sampler_ld.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"
#include "base/math/sampling/sample_distribution.inl"

namespace sampler {

inline float2 ld(uint32_t i, uint32_t r0, uint32_t r1) {
	return float2(math::scrambled_radical_inverse_vdC(i, r0), math::scrambled_radical_inverse_S(i, r1));
}

LD::LD(math::random::Generator& rng, uint32_t num_samples_per_iteration) :
	Sampler(rng, num_samples_per_iteration) {}

Sampler* LD::clone() const {
	return new LD(rng_, num_samples_per_iteration_);
}

math::uint2 LD::seed() const {
	return math::uint2(rng_.random_uint(), rng_.random_uint());
}

void LD::generate_camera_sample(int2 pixel, uint32_t index, Camera_sample& sample) {
	float2 s2d = ld(index, seed_.x, seed_.y);

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 LD::generate_sample_2D() {
	return ld(current_sample_++, seed_.x, seed_.y);
}

float LD::generate_sample_1D() {
	return rng_.random_float();
}

}

