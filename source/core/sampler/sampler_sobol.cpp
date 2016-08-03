#include "sampler_sobol.hpp"
#include "camera_sample.hpp"
#include "sobol/sobol.h"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"

namespace sampler {

Sobol::Sobol(math::random::Generator& rng, uint32_t num_samples_per_iteration) :
	Sampler(rng, num_samples_per_iteration) {}

Sampler* Sobol::clone() const {
	return new Sobol(rng_, num_samples_per_iteration_);
}

math::uint2 Sobol::seed() const {
	return math::uint2(rng_.random_uint(), rng_.random_uint());
}

void Sobol::generate_camera_sample(int2 pixel, uint32_t index,
									Camera_sample& sample) {
	float2 s2d(sobol::sample(index, 0), sobol::sample(index, 1));

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 Sobol::generate_sample_2D() {
//	return float2(halton_sampler_.sample(0, current_sample_++),
//				  halton_sampler_.sample(1, current_sample_++));

	return float2(sobol::sample(current_sample_++, 0, seed_.x),
				  sobol::sample(current_sample_++, 1, seed_.y));

}

float Sobol::generate_sample_1D() {
	return rng_.random_float();
}

}
