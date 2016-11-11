#include "sampler_hammersley.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/random/generator.inl"

namespace sampler {

Hammersley::Hammersley(rnd::Generator& rng, uint32_t num_samples) :
	Sampler(rng, num_samples) {}

void Hammersley::generate_camera_sample(int2 pixel, uint32_t index, Camera_sample& sample) {
	float2 s2d = math::hammersley(index, num_samples_, seed_.x);

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 Hammersley::generate_sample_2D() {
	return math::hammersley(current_sample_2D_++, num_samples_, seed_.x);
}

float Hammersley::generate_sample_1D() {
	return rng_.random_float();
}

Hammersley_factory::Hammersley_factory(uint32_t num_samples) :
	Factory(num_samples) {}

Sampler* Hammersley_factory::create(rnd::Generator& rng) const {
	return new Hammersley(rng, num_samples_);
}

}
