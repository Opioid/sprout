#include "sampler_sobol.hpp"
#include "camera_sample.hpp"
#include "sobol/sobol.h"
#include "base/math/vector.inl"
#include "base/random/generator.inl"

namespace sampler {

Sobol::Sobol(rnd::Generator& rng, uint32_t num_samples) :
	Sampler(rng, num_samples) {}

void Sobol::generate_camera_sample(int2 pixel, uint32_t index,
								   Camera_sample& sample) {
	float2 s2d(sobol::sample(index, 1), sobol::sample(index, 2));

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 Sobol::generate_sample_2D() {
	float x = sobol::sample(current_sample_2D_++, 1, seed_.x);
	float y = sobol::sample(current_sample_2D_++, 2, seed_.y);

//	++current_sample_2D_;

	return float2(x, y);
}

float Sobol::generate_sample_1D() {
//	return rng_.random_float();
	return sobol::sample(current_sample_1D_++, 3, seed_.x);
}

Sobol_factory::Sobol_factory(uint32_t num_samples) :
	Factory(num_samples) {}

Sampler* Sobol_factory::create(rnd::Generator& rng) const {
	return new Sobol(rng, num_samples_);
}

}
