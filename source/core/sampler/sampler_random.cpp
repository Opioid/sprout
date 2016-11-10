#include "sampler_random.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/random/generator.inl"

namespace sampler {

Random::Random(rnd::Generator& rng, uint32_t num_samples) :
	Sampler(rng, num_samples) {}

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

Random_factory::Random_factory(uint32_t num_samples) :
	Factory(num_samples) {}

Sampler* Random_factory::create(rnd::Generator& rng) const {
	return new Random(rng, num_samples_);
}

}
