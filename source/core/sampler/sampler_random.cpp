#include "sampler_random.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"

namespace sampler {

Random::Random(math::random::Generator& rng, uint32_t num_samples_per_iteration) :
	Sampler(rng, num_samples_per_iteration) {}

Sampler* Random::clone() const {
	return new Random(rng_, num_samples_per_iteration_);
}

math::uint2 Random::seed() const {
	return math::uint2::identity;
}

void Random::generate_camera_sample(int2 pixel, uint32_t /*index*/,
									Camera_sample& sample) {
	float2 s2d(rng_.random_float(), rng_.random_float());

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 Random::generate_sample_2D() {
	return float2(rng_.random_float(), rng_.random_float());
}

float Random::generate_sample_1D() {
	return rng_.random_float();
}

}
