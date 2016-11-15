#include "sampler_random.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/random/generator.inl"

namespace sampler {

Random::Random(rnd::Generator& rng, uint32_t num_samples, uint32_t num_dimensions_1D) :
	Sampler(rng, num_samples, num_dimensions_1D) {}

void Random::generate_camera_sample(int2 pixel, uint32_t /*index*/,
									Camera_sample& sample) {
	sample.pixel = pixel;
	sample.pixel_uv = float2(rng_.random_float(), rng_.random_float());
	sample.lens_uv = float2(rng_.random_float(), rng_.random_float());
	sample.time = rng_.random_float();
}

float2 Random::generate_sample_2D() {
	return float2(rng_.random_float(), rng_.random_float());
}

float Random::generate_sample_1D(uint32_t /*dimension*/) {
	return rng_.random_float();
}

Random_factory::Random_factory(uint32_t num_samples) :
	Factory(num_samples) {}

Sampler* Random_factory::create(rnd::Generator& rng) const {
	return new Random(rng, num_samples_);
}

}
