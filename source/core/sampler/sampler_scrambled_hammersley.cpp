#include "sampler_scrambled_hammersley.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/random/generator.inl"

namespace sampler {

Scrambled_hammersley::Scrambled_hammersley(rnd::Generator& rng,
										   uint32_t num_samples) :
	Sampler(rng, num_samples) {}

void Scrambled_hammersley::generate_camera_sample(int2 pixel, uint32_t index,
												  Camera_sample& sample) {
	float2 s2d = math::scrambled_hammersley(index, num_samples_, seed_.x);

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 Scrambled_hammersley::generate_sample_2D() {
	return math::scrambled_hammersley(current_sample_++,
									  num_samples_,
									  seed_.x);
}

float Scrambled_hammersley::generate_sample_1D() {
	return rng_.random_float();
}

Scrambled_hammersley_factory::Scrambled_hammersley_factory(uint32_t num_samples) :
	Factory(num_samples) {}

Sampler* Scrambled_hammersley_factory::create(rnd::Generator& rng) const {
	return new Scrambled_hammersley(rng, num_samples_);
}

}
