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
	float2 s2d(sobol::sample(index, 2, scramble_), sobol::sample(index, 3, scramble_));

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 Sobol::generate_sample_2D() {
	float x = sobol::sample(current_sample_2D_, 2, scramble_);
	float y = sobol::sample(current_sample_2D_, 3, scramble_);

	++current_sample_2D_;

	return float2(x, y);
}

float Sobol::generate_sample_1D(uint32_t /*dimension*/) {
//	return rng_.random_float();
	return sobol::sample(current_sample_1D_++, 5, scramble_);
}

void Sobol::on_resume_pixel(rnd::Generator& scramble) {
	scramble_ = scramble.random_uint();
}

Sobol_factory::Sobol_factory(uint32_t num_samples) :
	Factory(num_samples) {}

Sampler* Sobol_factory::create(rnd::Generator& rng) const {
	return new Sobol(rng, num_samples_);
}

}
