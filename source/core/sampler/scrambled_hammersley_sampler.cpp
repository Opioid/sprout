#include "scrambled_hammersley_sampler.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"
#include "base/math/sampling/sample_distribution.inl"

namespace sampler {

Scrambled_hammersley::Scrambled_hammersley(math::random::Generator& rng, uint32_t num_samples_per_iteration) :
	Sampler(rng, num_samples_per_iteration) {}

Sampler* Scrambled_hammersley::clone() const {
	return new Scrambled_hammersley(rng_, num_samples_per_iteration_);
}

math::uint2 Scrambled_hammersley::seed() const {
	return math::uint2(rng_.random_uint(), 0);
}

void Scrambled_hammersley::generate_camera_sample(math::uint2 pixel, uint32_t index, Camera_sample& sample) {
	math::float2 s2d = math::scrambled_hammersley(index, num_samples_per_iteration_, seed_.x);

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

math::float2 Scrambled_hammersley::generate_sample_2D() {
	return math::scrambled_hammersley(current_sample_++, num_iterations_ * num_samples_per_iteration_, seed_.x);
}

float Scrambled_hammersley::generate_sample_1D() {
	return rng_.random_float();
}

}
