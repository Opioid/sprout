#include "sampler_halton.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"

namespace sampler {

Halton::Halton(math::random::Generator& rng, uint32_t num_samples) :
	Sampler(rng, num_samples) {
	halton_sampler_.init_faure();
}

void Halton::generate_camera_sample(int2 pixel, uint32_t index,
									Camera_sample& sample) {
	float2 s2d(halton_sampler_.sample(0, index), halton_sampler_.sample(1, index));

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 Halton::generate_sample_2D() {
	float x = halton_sampler_.sample(1, current_sample_++);
	float y = halton_sampler_.sample(2, current_sample_++);
	return float2(x, y);
}

float Halton::generate_sample_1D() {
	return rng_.random_float();
}

Halton_factory::Halton_factory(uint32_t num_samples) :
	Factory(num_samples) {}

Sampler* Halton_factory::create(math::random::Generator& rng) const {
	return new Halton(rng, num_samples_);
}

}
